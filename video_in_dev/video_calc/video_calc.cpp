/*!
 * \file video_calc.cpp
 * \brief This file contains the module which processes an image
 * \author Samuel Mokrani & Florian Thorey
 * \date 20/06/2011
 *
 * First, this module loads all coefficients and information to fill his cache. 
 * Then it fills the cache and can begin the incremental computation for the tile processed.
 * To finish, it computes the bilinear interpolation and put each pixel in a fifo to be write in RAM.
 */
#include "video_calc.h"

namespace soclib { namespace caba {

    /////////////////////////////////////////////////////////////////////
    //	Constructor
    ////////////////////////////////////////////////////////////////////

    tmpl(/**/)::VideoCalc(sc_core::sc_module_name insname,
                          uint32_t * tab,
                          int w,
                          int h):
               sc_core::sc_module(insname),
               p_clk("p_clk"),
               p_resetn("p_resetn"),
               p_WIDTH(w),
               p_HEIGHT(h),
               fifo(F_SIZE * T_W * T_H),
               wb_tab(tab),
               master0(p_clk,p_resetn,p_wb_read),
               master1(p_clk,p_resetn,p_wb_write)
    {

      SC_THREAD(get_tile);
      sensitive << p_clk.pos();
      dont_initialize();

      SC_THREAD(process_tile);
      sensitive << p_clk.pos();
      dont_initialize();

      SC_THREAD(store_tile);
      sensitive << p_clk.pos();
      dont_initialize();

      std::cout <<  name()
		<< " was created successfully " << std::endl;
    }

    ///////////////////////////
    // Get_tile
    //////////////////////////

    tmpl(void)::get_tile()
    {
      bool get_ok = false;
      int tile_nb = 0;
      int nb_frame = 0;

      std::cout << " VCALC GET_TILE: START "  << std::endl;

      cache_rdy = false;

      for(;;)
      {
        /****************
         * RESET
         * *************/
        if (!p_resetn)
        {
          tile_nb = 0;
          cache_rdy = false;
          std::cout << " VCALC GET_TILE: RESET " << std::endl;
        }

        /**********
			* We are waiting a new address where will be read a new image
         **********/
        while(!wb_tab[5] && !get_ok)
          wait();

        if (!get_ok)
        {
          deb_im_in = wb_tab[4];
          wb_tab[5] = 0;
			 deb_coeff = wb_tab[8];
          get_ok = true;
          std::cout << " VCALC GET_TILE: NOUVELLE ADRESSE: " << deb_im_in << std::endl;
        }

        if (get_ok)
        {
          /**********
			  * We wait to be able to fill the cache
           **********/
          std::cout << " VCALC GET_TILE: ATTENTE ASK CACHE" << std::endl;
          while(!ask_cache)
            wait();

			 //We load the coefficients
			 fill_coeff(tile_nb+1);

          //We fill the cache
          fill_cache();
          std::cout << " VCALC GET_TILE: CACHE REMPLI " << std::endl;

          cache_rdy = true;
          ask_cache = false;

          tile_nb++;
          if (tile_nb == T_NB)
          {
            tile_nb = 0;
            get_ok = false;
          }
          else
            wait();
        }
      }
    }

    /////////////////////////////
    // Process_tile
    /////////////////////////////

    tmpl(void)::process_tile()
    {
      int16_t pixel_x;
      int16_t pixel_y;

      uint16_t coord_x;
      uint16_t coord_y;

      uint16_t dx;
      uint16_t dy;

      uint8_t I[2][2];
      uint8_t intensity_tab[4];
      uint32_t intensity;

      int count_pix = 0;
      int tile_nb = 0;

      std::cout << " VCALC PROCESS_TILE: START "  << std::endl;

      ask_cache = false;

      for(;;)
      {

        /****************
         * RESET
         * *************/
        if (!p_resetn)
        {
          std::cout << " VCALC PROCESS_TILE: RESET " << std::endl;
          tile_nb = 0;
          ask_cache = false;
          count_pix = 0;
        }


        ask_cache = true;

        /**********
         * We are waiting for the cache
         **********/
        std::cout << " VCALC PROCESS_TILE: ATTENTE CACHE RDY " << std::endl;
        while(!cache_rdy)
          wait();

        /**********
			* We update the beginning of the cache
         **********/
		  if ( ((uint16_t)(coeff.reg.cache_x << 16) >> 16) & 0x8000)
			 cache_x = (coeff.reg.cache_x >> 16) + 1;
		  else
			 cache_x = (coeff.reg.cache_x >> 16);

		  if ( (((uint16_t)coeff.reg.cache_y << 16) >> 16) & 0x8000)
			 cache_y = (coeff.reg.cache_y >> 16) + 1;
		  else
			 cache_y = (coeff.reg.cache_y >> 16);
		  std::cout << "ezfaaaaaaaaaa : " << cache_x << std::endl;
		  std::cout << "ezfaaaaaaaaaa : " << cache_y << std::endl;

        cache_rdy = false;

        for (int i = 0; i < T_H; i++)
        {
          for(int j = 0; j < T_W; j++)
          {
				//we choose the nearest pixel according to the fractionnal part
				if ( (((uint16_t)coeff.reg.Px[3] << 16) >> 16) & 0x8000)
				  pixel_x = (coeff.reg.Px[3] >> 16) + 1;
				else
				  pixel_x = (coeff.reg.Px[3] >> 16);

				if ( (((uint16_t)coeff.reg.Py[3] << 16) >> 16) & 0x8000)
				  pixel_y = (coeff.reg.Py[3] >> 16) + 1;
				else
				  pixel_y = (coeff.reg.Py[3] >> 16);


				//The pixel is in the cache or not ?
            if ((pixel_x <  cache_x) || (pixel_x >=  (cache_x + (int16_t)C_W)) ||
                (pixel_y <  cache_y) || (pixel_y >=  (cache_y + (int16_t)C_H)))
				{
              intensity_tab[count_pix] = (uint8_t) WHITE_PIXEL;
				}
            else
            {
              coord_x = (uint16_t) (pixel_x - cache_x);
              coord_y = (uint16_t) (pixel_y - cache_y);

              /*std::cout << " VCALC PROCESS_TILE: TILE NUMBER "
                        << tile_nb
                        << " antecedent_y : "
                        << coord_y
                        << " antecedent_x : "
                        << coord_x
                        << std::endl;*/

              dx = (coeff.reg.Px[3] << 16) >> 16;
              dy = (coeff.reg.Py[3] << 16) >> 16;

				  //We get the four pixels to do the bilinear interpolation
				  //TODO: do the same in one cycle with 4 memories.
              I[0][0] = cache[coord_y][coord_x];
              if ((coord_x + 1) < C_W)
                I[1][0] = cache[coord_y][coord_x + 1];
              else
                I[1][0] = I[0][0];
              if ((coord_y + 1) < C_H)
                I[0][1] = cache[coord_y + 1][coord_x];
              else
                I[0][1] = I[0][0];
              if (((coord_x + 1) < C_W) && ((coord_y + 1) < C_H))
                I[1][1] = cache[coord_y + 1][coord_x + 1];
              else
                I[1][1] = I[0][0];

				  //Computation of the final intensity
				  uint32_t dx_1 = (1 << 16) - dx;
				  uint32_t dy_1 = (1 << 16) - dy;
				  int32_t a1 = fx_mul(dx_1, fx_mul(dy_1, (I[0][0] << 16)));
				  int32_t a2 = fx_mul(dx_1, fx_mul(dy, (I[0][1] << 16)));
				  int32_t a3 = fx_mul(dx, fx_mul(dy_1, (I[1][0] << 16)));
				  int32_t a4 = fx_mul(dx, fx_mul(dy, (I[1][1] << 16)));
				  intensity = a1 + a2 + a3 + a4;
				  wait();
				  wait();
				  wait();

              if ( (uint8_t) (intensity >> 16) >= WHITE_PIXEL)

                intensity_tab[count_pix] = (uint8_t) WHITE_PIXEL;
              else
                intensity_tab[count_pix] = (uint8_t) (intensity>>16);
            }

				//We write the intensity in a fifo
            if (count_pix == 3)
            {
              count_pix = 0;
              for (int k = 0; k < 4; k++)
                fifo.write(intensity_tab[k]);
            }
            else
              count_pix++;

            for (int k = 3; k > 0; k--)
            {
              coeff.reg.Px[k] += coeff.reg.Px[k-1];
              coeff.reg.Py[k] += coeff.reg.Py[k-1];
            }

          }

			 //We update coefficients to the incremental computation
          for(int k = 3; k > 0; k--)
          {
            coeff.reg.Qx[k] += coeff.reg.Qx[k-1];
            coeff.reg.Qy[k] += coeff.reg.Qy[k-1];
          }

          for(int k = 2; k > 0; k--)
          {
            coeff.reg.Rx[k] += coeff.reg.Rx[k-1];
            coeff.reg.Ry[k] += coeff.reg.Ry[k-1];
          }

          coeff.reg.Sx[1] += coeff.reg.Sx[0];
          coeff.reg.Sy[1] += coeff.reg.Sy[0];

          coeff.reg.Px[3] = coeff.reg.Qx[3];
          coeff.reg.Py[3] = coeff.reg.Qy[3];
          coeff.reg.Px[2] = coeff.reg.Rx[2];
          coeff.reg.Py[2] = coeff.reg.Ry[2];
          coeff.reg.Px[1] = coeff.reg.Sx[1];
          coeff.reg.Py[1] = coeff.reg.Sy[1];
        }
      }
    }

	 tmpl(int32_t)::fx_mul(int32_t A, int32_t B)
	 {
		uint32_t tmp_l = ((uint32_t)(A & 0x0000ffff) * (uint32_t)(B & 0x0000ffff)) >> 16;
		int32_t tmp_lh = ((int32_t)(A & 0xffff0000) >> 16) * (uint32_t)(B & 0x0000ffff);
		int32_t tmp_hl = ( (int32_t)(B & 0xffff0000) >> 16) * (uint32_t)(A & 0x0000ffff);
		int32_t tmp_h = (((int32_t)(B & 0xffff0000) >> 16) * ((int32_t)(A & 0xffff0000)>>16)) << 16;
		return (int32_t) (tmp_l + tmp_lh + tmp_hl + tmp_h);
	 }


    /////////////////////////////
    // Store_tile
    /////////////////////////////

    tmpl(void)::store_tile()
    {
      uint32_t to_store[T_W/4];
      uint8_t mask[T_W/4];
      bool store_ok = false;
      int nb_line_stored = 0;
      int nb_tile = 0;
      uint32_t addr;
      uint8_t pixel_temp;

      for (int i = 0; i < T_W/4; i++)
        mask[i] = 0xf;

      std::cout << " VCALC STORE_TILE: START "  << std::endl;

      for(;;)
      {
        /**********
         * RESET
         *********/
        if (!p_resetn)
        {
          std::cout << " VCALC STORE_TILE: RESET " << std::endl;
          nb_line_stored = 0;
          p_interrupt = 0;
        }

        /**********
			* We are waiting a new address where will be read a new image
         **********/
        while(!wb_tab[7] && !store_ok)
          wait();

        if(!store_ok)
        {
          store_ok = true;
          deb_im_out = wb_tab[6];
          wb_tab[7] = 0;
          std::cout << " VCALC STORE_TILE: NOUVELLE ADRESSE: " << deb_im_out << std::endl;
        }

		  //If there are anough elements in the fifo (a line), we can write
		  //them in RAM
        if ((uint32_t) fifo.num_available() >= T_W)
        {
          for (int i = 0; i< T_W/4; i++)
          {
            to_store[i] = 0;
            for (int j = 0; j < 4; j++)
            {
              to_store[i] = to_store[i] << 8;
              if (!fifo.nb_read(pixel_temp))
                std::cout << " VCALC STORE_TILE: bloque sur FIFO " << std::endl;
              else
                to_store[i] += pixel_temp;
            }
          }

          addr = deb_im_out + (nb_line_stored % T_H) * p_WIDTH + (nb_tile % (p_WIDTH/T_W)) * T_W + (nb_tile/(p_WIDTH/T_W)) * T_H * p_WIDTH;
          master1.wb_write_blk(addr, mask, to_store, T_W/4);

          nb_line_stored++;
          if (nb_line_stored == T_H)
          {
            nb_line_stored = 0;
            nb_tile++;
            std::cout << " VCALC STORE_TILE: TILE NUMBER " << nb_tile << std::endl;
          }

          /**********************
           * INTERRUPT GENERATION
           *********************/

          if((uint32_t) nb_tile == T_NB)
          {
            store_ok = false;
            std::cout << " VCALC STORE_TILE: INTERRUPTION SENT " << std::endl;
            nb_line_stored = 0;
            nb_tile = 0;
            p_interrupt = 1;
            wait();
            wait();
            wait();
            p_interrupt = 0;
          }

        }
        else
          wait();
      }
    }



    /////////////////////////////
    // fill_cache
    /////////////////////////////

    tmpl(void)::fill_cache()
    {
		// Width of the cache which is is the image
      uint32_t cache_w;
		// Height of the cache which is is the image
      uint32_t cache_h;

		int32_t cache_x_img;
		int32_t cache_y_img;

      uint32_t decalage_w = 0;
      uint32_t decalage_h = 0;

      uint32_t buffer_line[C_W/4];
      uint32_t adr;

      // We fiil the cache with white pixels
      for (int i = 0; i < C_H; i++)
        for (int j = 0; j < C_W; j++)
          cache[i][j] = WHITE_PIXEL;

      // If the cache is out of the image => return
      if (cache_x > ((int16_t) p_WIDTH - 1)  ||
          cache_y > ((int16_t) p_HEIGHT - 1)
        )
		  return;

		//else we compute the intersection between the cache
		//and the image

      // By default, the cache is in the image
      cache_w = C_W;
      cache_h = C_H;

		cache_x_img = cache_x;
		cache_y_img = cache_y;

		//Overflow on the left
      if (cache_x < 0)
      {
        if (cache_x + (int16_t) C_W - 1 < 0)
          return;
        else
        {
          cache_w += cache_x;
          decalage_w = C_W - cache_w;
          cache_x_img = 0;
        }
      }

      // Overflow on the right
      if (cache_y < 0)
      {
        if (cache_y + (int16_t)C_H - 1 < 0)
          return;
        else
        {
          cache_h += cache_y;
          decalage_h = C_H - cache_h;
          cache_y_img = 0;
        }
      }

      // Overflow on the top
      if ((cache_x + C_W) > (int16_t) p_WIDTH)
        cache_w -= (cache_x + C_W - p_WIDTH);

      // Overflow on the bottom
      if ((cache_y + C_H) > (int16_t) p_HEIGHT)
        cache_h -= (cache_y + C_H - p_HEIGHT);

      // We fill the cache
      for (uint32_t line = 0; line < cache_h; line++)
      {
        adr = deb_im_in + (cache_y_img + line) * p_WIDTH + cache_x_img;
        master0.wb_read_blk(adr, cache_w / 4, buffer_line);

		  int k = 0;
        for (uint32_t i = 0; i < (cache_w / 4); i++)
        {
          for (int j = 3; j >= 0; j--)
          {
            cache[decalage_h + line][decalage_w + k + (3-j)] = buffer_line[i] >> 8 * j;
            buffer_line[i] = buffer_line[i] - ((buffer_line[i] >> 8 * j) << 8 * j);
          }
           k += 4;
        }

      }

    }

	 tmpl(void)::fill_coeff(int tile_nb)
	 {
		  master0.wb_read_blk(	deb_coeff + tile_nb * NB_COEFF * 4 / 2,
										NB_COEFF/2,
										&coeff.raw[0]);

		  master0.wb_read_blk(	deb_coeff + T_NB * NB_COEFF * 4 / 2 +
										tile_nb * NB_COEFF * 4 / 2,
										NB_COEFF/2,
										&coeff.raw[NB_COEFF / 2]);
		  std::cout << "cache_x : " << (coeff.reg.cache_x >> 16) << std::endl;
		  std::cout << "cache_y : " << (coeff.reg.cache_y >> 16) << std::endl;

	 }

    /*tmpl(void)::init_coeff()
    {
		uint32_t addr = wb_tab[8];
		for (int tile_nb = 0; tile_nb < T_NB; tile_nb++)
		{
		  master0.wb_read_blk(	addr + tile_nb * NB_COEFF * 4 / 2,
										NB_COEFF/2,
										&coeff_image[tile_nb].raw[0]);

		  master0.wb_read_blk(	addr + T_NB * NB_COEFF * 4 / 2 +
										tile_nb * NB_COEFF * 4 / 2,
										NB_COEFF/2,
										&coeff_image[tile_nb].raw[NB_COEFF / 2]);

		}
    }*/


  }}
