/**********************************************************************
 *
 *  Module de calcul de transformation géométrique
 *  Pour l'instant :
 *  1 TO DO (Va chercher les coeffs en RAM et les mets dans un buffer.)
 *  2 Va chercher des tuiles de B_W * T_H en RAM et les mets dans un buffer.
 *  3 Effectue le calcul sur chaque tuile de T_W * T_H
 *  4 Stocke les tuiles transformées de T_W * T_H en RAM
 *
 *********************************************************************/
#include "video_calc.h"

namespace soclib { namespace caba {

    /////////////////////////////////////////////////////////////////////
    //	Constructeur
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

      // Module de remplissage du buffer
      // * Bufferise les tuiles d'entrée
      // * TO DO (Bufferise les coeffs)
      SC_THREAD(get_tile);
      sensitive << p_clk.pos();
      dont_initialize();

      // Module de calcul incrémental
      // * Effectue le calcul incrémental
      SC_THREAD(process_tile);
      sensitive << p_clk.pos();
      dont_initialize();

      // Module d'écriture en RAM
      // * Pose les tuiles calculées dans la fifo
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
         * On attend une nouvelle addresse ou l'on va stocker l'image
         **********/
        while(!wb_tab[5] && !get_ok)
          wait();

        /**********
         * On change les coeffs toutes les X images
         **********/
        if (!(nb_frame % 3))
        {
          nb_frame = 1;
          init_coeff();
          for (int i = 0; i < T_NB; i++)
            for (int j = 0; j < NB_COEFF; j++)
            {
              coeff[i].raw[j] = coeff_image[i].raw[j];
               /*std::cout << " VCALC GET_TILE: COEFF RAM : "
                         << " tile nb : "
                         << i
                         << " coeff : "
                         << (int) (coeff[i].raw[j] >> 16)
                         << std::endl;*/
            }
        }

        if (!get_ok)
        {
          get_ok = true;
          deb_im_in = wb_tab[4];
          wb_tab[5] = 0;
          std::cout << " VCALC GET_TILE: NOUVELLE ADRESSE: " << deb_im_in << std::endl;
        }

        if (get_ok)
        {
          /**********
           * On attend de pouvoir remplir le cache
           **********/
          std::cout << " VCALC GET_TILE: ATTENTE ASK CACHE" << std::endl;
          while(!ask_cache)
            wait();

          /**********
           * On remplit le cache
           **********/
          fill_cache(deb_im_in, tile_nb);
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
      int tile_nb = T_NB;

      std::cout << " VCALC PROCESS_TILE: START "  << std::endl;

      ask_cache = false;

      for(;;)
      {

        if (tile_nb == T_NB)
        {
          tile_nb = 0;
          for (int i = 0; i < T_NB; i++)
            for (int j = 0; j < NB_COEFF; j++)
              coeff[i].raw[j] = coeff_image[i].raw[j];
        }


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

        /**********
         * On met à jour le coin gauche de la tuile
         **********/
        cache_x =  coeff[tile_nb].reg.Px[3] >> 16;
        cache_y =  coeff[tile_nb].reg.Py[3] >> 16;
		  /*std::cout << "cache_x : "
			 			<< cache_x
						<< " cache_y : "
						<< cache_y
						<< std::endl;*/

        ask_cache = true;

        /**********
         * On attend que le cache soit remplit
         **********/
        std::cout << " VCALC PROCESS_TILE: ATTENTE CACHE RDY " << std::endl;
        while(!cache_rdy)
          wait();

        cache_rdy = false;

        for (int i = 0; i < T_H; i++)
        {
          for(int j = 0; j < T_W; j++)
          {
            pixel_x = coeff[tile_nb].reg.Px[3] >> 16;
            pixel_y = coeff[tile_nb].reg.Py[3] >> 16;

              /*std::cout << " VCALC PROCESS_TILE: TILE NUMBER "
                        << tile_nb
                        << " antecedent_y : "
                        << pixel_y
                        << " antecedent_x : "
                        << pixel_x
                        << std::endl;*/

            if ((pixel_x < (uint16_t) cache_x) || (pixel_x > (uint16_t) (cache_x + C_W)) ||
                (pixel_y < (uint16_t) cache_y) || (pixel_y > (uint16_t) (cache_y + C_H)))
              intensity_tab[count_pix] = (uint8_t) PIXEL_BLANC;
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

              dx = coeff[tile_nb].reg.Px[3];
              dy = coeff[tile_nb].reg.Px[3];

              /*std::cout << " VCALC PROCESS_TILE: TILE NUMBER "
                        << tile_nb
                        << " dy : "
                        << dy
                        << " dx : "
                        << dx
                        << std::endl;*/

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
                I[1][1] = cache[coord_y + 1][coord_y + 1];
              else
                I[1][1] = I[0][0];

				  uint32_t dx_1 = (1 << 16) - dx;
				  uint32_t dy_1 = (1 << 16) - dy;
				  int32_t a1 = fx_mul(dx_1, fx_mul(dy_1, (int32_t)(I[0][0] << 16)));
				  int32_t a2 = fx_mul(dx_1, fx_mul(dy, (int32_t)(I[0][1] << 16)));
				  int32_t a3 = fx_mul(dx, fx_mul(dy_1, (int32_t)(I[1][0] << 16)));
				  int32_t a4 = fx_mul(dx, fx_mul(dy, (int32_t)(I[1][1] << 16)));
				  /*std::cout << "entire part : " << 
					 				(a1 >> 16) << 
									" fractionnale part : " << ((a1 << 16) >> 16) << std::endl;*/
				  intensity = a1 + a2 + a3 + a4;
              /*intensity = 	(1 - dx) * 
					 				(1 - dy) * 
									I[0][0] +

                				(1 - dx) * 
									dy * 
									I[0][1] +

                				dx * 
									(1 - dy) * 
									I[1][0] +

                				dx * 
									dy 
									* I[1][1];*/


              /*std::cout << " VCALC PROCESS_TILE: TILE NUMBER "
                        << tile_nb
                        << " intensity : "
                        << intensity
                        << std::endl;*/

              if ( (uint8_t) (intensity >> 16) > PIXEL_BLANC)
                intensity_tab[count_pix] = (uint8_t) PIXEL_BLANC;
              else
                intensity_tab[count_pix] = (uint8_t) (intensity>>16);
            }

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
              coeff[tile_nb].reg.Px[k] += coeff[tile_nb].reg.Px[k-1];
              coeff[tile_nb].reg.Py[k] += coeff[tile_nb].reg.Py[k-1];
            }

          }

          for(int k = 3; k > 0; k--)
          {
            coeff[tile_nb].reg.Qx[k] += coeff[tile_nb].reg.Qx[k-1];
            coeff[tile_nb].reg.Qy[k] += coeff[tile_nb].reg.Qy[k-1];
          }

          for(int k = 2; k > 0; k--)
          {
            coeff[tile_nb].reg.Rx[k] += coeff[tile_nb].reg.Rx[k-1];
            coeff[tile_nb].reg.Ry[k] += coeff[tile_nb].reg.Ry[k-1];
          }

          coeff[tile_nb].reg.Sx[1] += coeff[tile_nb].reg.Sx[0];
          coeff[tile_nb].reg.Sy[1] += coeff[tile_nb].reg.Sy[0];

          coeff[tile_nb].reg.Px[3] = coeff[tile_nb].reg.Qx[3];
          coeff[tile_nb].reg.Py[3] = coeff[tile_nb].reg.Qy[3];
          coeff[tile_nb].reg.Px[2] = coeff[tile_nb].reg.Rx[2];
          coeff[tile_nb].reg.Py[2] = coeff[tile_nb].reg.Ry[2];
          coeff[tile_nb].reg.Px[1] = coeff[tile_nb].reg.Sx[1];
          coeff[tile_nb].reg.Py[1] = coeff[tile_nb].reg.Sy[1];
        }

      tile_nb++;
      std::cout << " VCALC PROCESS_TILE: TILE NUMBER " << tile_nb  <<std::endl;

      }
    }

	 tmpl(int32_t)::fx_mul(int32_t A, int32_t B)
	 {
		/*std::cout << " A decale : " << (A & 0x0000ffff) << std::endl;
		std::cout << " B decale : " << (B & 0x0000ffff) << std::endl;
		std::cout << " A decale : " << ((A & 0xffff0000) >> 16)<< std::endl;
		std::cout << " B decale : " << ((B & 0xffff0000) >> 16) << std::endl;*/
		uint16_t tmp_l = (A & 0x0000ffff) * (B & 0x0000ffff);
		int32_t tmp_lh = ( (A & 0xffff0000) >> 16) * (B & 0x0000ffff);
		int32_t tmp_hl = ( (B & 0xffff0000) >> 16) * (A & 0x0000ffff);
		int32_t tmp_h = ((B & 0xffff0000)*(A & 0xffff0000) >> 16) << 16;
		int32_t result = (int32_t) tmp_l + tmp_lh + tmp_hl + tmp_h;
		/*std::cout 	<< "A : " << A
		  				<< " B : " << B
						<< " tmp_l : " << tmp_l
						<< " tmp_lh : " << tmp_lh
						<< " tmp_hl : " << tmp_hl
						<< " tmp_h : " << tmp_h 
						<< " result : " << result << std::endl;*/

		return result;
	 }


    /////////////////////////////
    // Store_tile
    /////////////////////////////

    /* Stocke les tuiles de la fifo à l'adresse de la RAM stockée dans wb_tab[6]
     * Indique au processeur quand l'image a fini d'être stockée
     */

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
         * On attend une nouvelle addresse ou l'on va stocker l'image
         **********/
        while(!wb_tab[7] && !store_ok)
        {
          //std::cout << "j'attends une addresse ou l'on va stocker une image" << std::endl;
          wait();
        }

        if(!store_ok)
        {
          store_ok = true;
          deb_im_out = wb_tab[6];
          wb_tab[7] = 0;
          std::cout << " VCALC STORE_TILE: NOUVELLE ADRESSE: " << deb_im_out << std::endl;
        }

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

    tmpl(void)::fill_cache(uint32_t deb_im_in, int tile_nb)
    {
      // Taille de la zone du cache qui est dans l'image
      uint32_t cache_w;
      uint32_t cache_h;

      // Décalage dans le cache de la zone dans l'image
      uint32_t decalage_w = 0;
      uint32_t decalage_h = 0;

      uint32_t buffer_line[C_W/4];
      uint32_t adr;

      // On remplit le cache de pixels
      for (int i = 0; i < C_H; i++)
        for (int j = 0; j < C_W; j++)
          cache[i][j] = PIXEL_BLANC;

      // Cas où le cache est complétement en dehors de l'image
      if (cache_x > (int32_t) p_WIDTH  ||
          cache_y > (int32_t) p_HEIGHT
        )
        return;

      // Sinon on calcule l'intersection entre le cache
      // et l'image

      // Par défaut le cache est dans l'image
      cache_w = C_W;
      cache_h = C_H;

      // std::cout << " VCALC PROCESS_TILE: BEFORE TILE NUMBER "
      //           << tile_nb
      //           << " cache_y : "
      //           << cache_y
      //           << " cache_x : "
      //           << cache_x
      //           << " cache_h : "
      //           << cache_h
      //           << " cache_w : "
      //           << cache_w
      //           << std::endl;

      // Dépassement à gauche
      if (cache_x < 0)
      {
        if (cache_x < (int32_t) -C_W)
          return;
        else
        {
          cache_w += cache_x;
          decalage_w = C_W - cache_w;
          cache_x = 0;
        }
      }

      // Dépassement en haut
      if (cache_y < 0)
      {
        if (cache_y < (int32_t) -C_H)
          return;
        else
        {
          cache_h += cache_y;
          decalage_h = C_H - cache_h;
          cache_y = 0;
        }
      }

      // std::cout << " VCALC PROCESS_TILE: AFTER TILE NUMBER "
      //           << tile_nb
      //           << " cache_y : "
      //           << cache_y
      //           << " cache_x : "
      //           << cache_x
      //           << " cache_h : "
      //           << cache_h
      //           << " cache_w : "
      //           << cache_w
      //           << std::endl;

      // Dépassement à droite
      if ((cache_x + C_W) > (int32_t) p_WIDTH)
        cache_w -= (cache_x + C_W - p_WIDTH);

      // Dépassement en bas
      if (cache_y + C_H > (int32_t) p_HEIGHT)
        cache_h -= (cache_y + C_H - p_HEIGHT);

      // Remplissage du cache
      for (uint32_t line = 0; line < cache_h; line++)
      {
        adr = deb_im_in + (cache_y + line) * p_WIDTH + cache_x;
        master0.wb_read_blk(adr, cache_w / 4, buffer_line);

        /*std::cout << " VCALC PROCESS_TILE: adr TILE NUMBER "
                  << tile_nb
                  << " adr : "
                  << adr
                  << std::endl;*/


         int k = 0;
        for (uint32_t i = 0; i < (cache_w / 4); i++)
        {
          for (int j = 3; j >= 0; j--)
          {

            /*std::cout << " VCALC PROCESS_TILE: position cache TILE NUMBER "
                      << tile_nb
                      << " line "
                      << decalage_h + line
                      << " col "
                      << decalage_w + k + j
                      << std::endl;*/

            cache[decalage_h + line][decalage_w + k + (3-j)] = buffer_line[i] >> 8 * j;
            buffer_line[i] = buffer_line[i] - ((buffer_line[i] >> 8 * j) << 8 * j);
          }
           k +=4;
        }

      }

    }

    tmpl(void)::init_coeff()
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
		/*for (int tile_nb = 0; tile_nb < T_NB; tile_nb++)
		{
		  int16_t temp_Px3 = coeff_image[tile_nb].reg.Px[3] >> 16;
		  int16_t temp_Py3 = coeff_image[tile_nb].reg.Py[3] >> 16;
		  int16_t temp_Px2 = coeff_image[tile_nb].reg.Px[2] >> 16;
		  int16_t temp_Py2 = coeff_image[tile_nb].reg.Py[2] >> 16;
		  int16_t temp_Px1 = coeff_image[tile_nb].reg.Px[1] >> 16;
		  int16_t temp_Py1 = coeff_image[tile_nb].reg.Py[1] >> 16;
		  int16_t temp_Px0 = coeff_image[tile_nb].reg.Px[0] >> 16;
		  int16_t temp_Py0 = coeff_image[tile_nb].reg.Py[0] >> 16;
		  int16_t temp_Rx2 = coeff_image[tile_nb].reg.Rx[2] >> 16;
		  int16_t temp_Ry2 = coeff_image[tile_nb].reg.Ry[2] >> 16;
		  int16_t temp_Qx2 = coeff_image[tile_nb].reg.Qx[2] >> 16;
		  int16_t temp_Qy2 = coeff_image[tile_nb].reg.Qy[2] >> 16;
		  std::cout << " VCALC GET_TILE: COEFF RAM : "
			 << " tile nb : "
			 << tile_nb
			 << " Px3 : "
			 << temp_Px3
			 << " Py3 : "
			 << temp_Py3
			 << " Px2 : "
			 << temp_Px2
			 << " Py2 : "
			 << temp_Py2
			 << " Px1 : "
			 << temp_Px1
			 << " Py1 : "
			 << temp_Py1
			 << " Px0 : "
			 << temp_Px0
			 << " Py0 : "
			 << temp_Py0
			 << " Rx2 : "
			 << temp_Rx2
			 << " Ry2 : "
			 << temp_Ry2
			 << " Qx2 : "
			 << temp_Qx2
			 << " Qy2 : "
			 << temp_Qy2
			 << std::endl;
		}*/
    }


  }}
