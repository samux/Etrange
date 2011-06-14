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

      std::cout << " VCALC GET_TILE: START: "  << std::endl;

      cache_rdy = false;

      for(;;)
      {
        /****************
         * RESET
         * *************/
        if (!p_resetn)
        {
          std::cout << " VCALC GET_TILE: RESET " << std::endl;
          cache_rdy = false;
        }

        /**********
         * On attend une nouvelle addresse ou l'on va stocker l'image
         **********/
        while(!wb_tab[5] && !get_ok)
          wait();

        if (!get_ok)
        {
          get_ok = true;
          deb_im_in = wb_tab[4];
          wb_tab[5] = 0;
          std::cout << " VCALC GET_TILE: NOUVELLE ADRESSE: " << deb_im_in << std::endl;
        }

        /**********
         * On attend de pouvoir remplir le cache
         **********/
        while(!ask_cache)
          wait();

        /**********
         * On remplit le cache
         **********/
        fill_cache(deb_im_in);

        cache_rdy = true;
        ask_cache = false;
      }
    }

    /////////////////////////////
    // Process_tile
    /////////////////////////////

    tmpl(void)::process_tile()
    {
      float pixel_x;
      float pixel_y;

      uint32_t coord_x_i;
      uint32_t coord_y_i;

      float coord_x_f;
      float coord_y_f;

      float dx;
      float dy;

      uint32_t I[2][2];

      float intensity;

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
          ask_cache = false;
        }

        /**********
         * On met à jour le coin gauche de la tuile
         **********/
        cache_x = (int32_t) coeff.reg.Px[3];
        cache_y = (int32_t) coeff.reg.Py[3];

        ask_cache = true;

        /**********
         * On attend que le cache soit remplit
         **********/
        while(!cache_rdy)
          wait();

        cache_rdy = false;

        for (int i = 0; i < T_H; i++)
        {
          for(int j = 0; j < T_W; j++)
          {
            pixel_x = coeff.reg.Px[3];
            pixel_y = coeff.reg.Py[3];

            if (((int32_t) pixel_x < cache_x) || ((int32_t) pixel_x > (cache_x + C_W)) ||
                ((int32_t) pixel_y < cache_y) || ((int32_t) pixel_y > (cache_y + C_H)))
              fifo.write(PIXEL_BLANC);
            else
            {
              coord_x_i = ((uint32_t) pixel_x) - cache_x;
              coord_y_i = ((uint32_t) pixel_y) - cache_y;

              coord_x_f = pixel_x - ((float) cache_x);
              coord_y_f = pixel_y - ((float) cache_y);

              dx = coord_x_f - coord_x_i;
              dy = coord_x_f - coord_x_i;

              I[0][0] = cache[coord_y_i][coord_x_i];
              if ((coord_x_i + 1) < C_W)
                I[1][0] = cache[coord_y_i][coord_x_i + 1];
              else
                I[1][0] = I[0][0];
              if ((coord_y_i + 1) < C_H)
                I[0][1] = cache[coord_y_i + 1][coord_x_i];
              else
                I[0][1] = I[0][0];
              if (((coord_x_i + 1) < C_W) && ((coord_y_i + 1) < C_H))
                I[1][1] = cache[coord_y_i + 1][coord_y_i + 1];
              else
                I[1][1] = I[0][0];

              intensity = (1 - dx) * (1 - dy) * I[0][0] +
                (1 - dx) * dy * I[0][1] +
                dx * (1 - dy) * I[1][0] +
                dx * dy * I[1][1];

              if ((uint8_t) intensity > PIXEL_BLANC)
                fifo.write(PIXEL_BLANC);
              else
                fifo.write((uint8_t) intensity);
            }

            for (int k = 3; k > 0; k--)
            {
              coeff.reg.Px[k] += coeff.reg.Px[k-1];
              coeff.reg.Py[k] += coeff.reg.Py[k-1];
            }
          }

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
            //std::cout << " VCALC STORE_TILE: TILE NUMBER " << nb_tile << std::endl;
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

    tmpl(void)::fill_cache(uint32_t deb_im_in)
    {
      // Taille de la zone du cache qui est dans l'image
      uint32_t cache_w;
      uint32_t cache_h;

      // Décalage dans le cache de la zone dans l'image
      uint32_t decalage_w = 0;
      uint32_t decalage_h = 0;

      int line = 0;
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

      // Dépassement à droite
      if ((cache_x + C_W) > (uint32_t) p_WIDTH)
        cache_w -= (cache_x + C_W - p_WIDTH);

      // Dépassement en bas
      if (cache_y + C_H > (uint32_t) p_HEIGHT)
        cache_h -= (cache_y + C_H - p_HEIGHT);

      // Remplissage du cache
      for (line = 0; line < cache_h; line++)
      {
        adr = deb_im_in + (cache_y + line) * p_WIDTH + cache_x;
        master0.wb_read_blk(adr, cache_w / 4, buffer_line);

        int k = 0;
        for (int i = 0; i < (cache_w / 4); i++)
        {
          for (int j = 3; j >= 0; j--)
          {
            cache[decalage_h + line][decalage_w + k + j] = buffer_line[i] >> 8 * j;
            buffer_line[i] = buffer_line[i] - ((buffer_line[i] >> 8 * j) << 8 * j);
          }
          k +=4;
        }

      }

    }

  }}
