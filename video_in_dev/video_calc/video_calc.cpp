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
      sensitive << clk.pos();

      // Module de calcul incrémental
      // * Effectue le calcul incrémental
      // * Pose les tuiles calculées dans la fifo
      SC_THREAD(store_tile);
      sensitive << clk.pos();

      std::cout <<  name()
                << " was created successfully " << std::endl;
    }

    ///////////////////////////
    // Get_tile
    //////////////////////////

    tmpl(void)::get_tile()
    {
      // Nb de tuiles traitées dans get_tile
      uint32_t tile_nb = 0;

      uint32_t buffer_c;
      uint32_t buffer_l;

      bool stockage_ok = false;

      int tile_line = p_WIDTH / T_W;
      int buffer_line;
      uint32_t buffer_line_temp[T_W/4];

      get_tile_end = false;

      /*DEB*/ std::cout << " VCALC GET_TILE: START " << std::endl;

      for (;;)
      {


        if (reset_n == false)
        {
          tile_nb = 0;
          /*DEB*/ std::cout << " VCALC GET_TILE: RESET " << std::endl;
          wait();
        }

        while (wb_tab[5] == 0)
          wait();

        img_adr_in = wb_tab[4];
        wb_tab[5] = 0;
        wait();
        /*DEB*/ std::cout << " VCALC GET_TILE: address IN: " << img_adr_in << std::endl;

        for (tile_nb = 0; tile_nb < T_NB; tile_nb++)
        {

          buffer_c = tile_nb % tile_line * T_W;
          buffer_l = tile_nb / tile_line * T_H;

          for (buffer_line = 0; buffer_line < T_H; buffer_line++)
          {

            master0.wb_read_blk(img_adr_in + (buffer_l + buffer_line) * p_WIDTH + buffer_c,
                                T_W / 4,
                                buffer_line_temp);

            int k = 0;
            for (int i = 0; i < (T_W / 4); i++)
            {
              for (int j = 0; j < 4; j++)
              {
                fifo.write(buffer_line_temp[i] >> 8 * (3-j));
                buffer_line_temp[i] = buffer_line_temp[i] - ((buffer_line_temp[i] >> 8 * (3-j)) << 8 * (3-j));
              }
              k +=4;
            }
          }
        }
        get_tile_end = true;
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
      uint32_t tile_nb = 0;
      int tile_line = p_WIDTH / T_W;
      int line_nb = 0;

      //Une ligne de pixels groupés par paquets de 4
      uint32_t pixel_pack[T_W/4];
      uint8_t mask[T_W/4];

      bool stockage_ok = false;

      for (int i = 0; i < T_W/4; i++)
        mask[i] = 0xf;

      /*DEB*/ std::cout << " VCALC STORE_TILE: START" << std::endl;

      for (;;)
      {

        // Si un reset a lieu on réinitialise toutes les variables
        if (reset_n == false)
        {
          tile_nb = 0;
          p_interrupt = 0;
          /*DEB*/ std::cout <<" VCALC STORE_TILE: RESET" << std::endl;
          wait();
        }

        if (wb_tab[7] != 0 && !stockage_ok)
        {
          img_adr_out = wb_tab[6];
          wb_tab[7] = 0;
          stockage_ok = true;
          // On affiche la valeur l'adresse recupérée
          /*DEB*/ std::cout << " VCALC GET_TILE: address OUT: " << img_adr_out << std::endl;
          wait();
        }

        if (stockage_ok)
        {

          if ( (int) fifo.num_available() > (T_W/4))
          {

            for (int k = 0; k < T_W / 4; k++)
            {
              pixel_pack[k] = 0;
              for (int j = 0; j < 4; j++)
              {
                pixel_pack[k] = pixel_pack[k] << 8;
                pixel_pack[k] += fifo.read();
              }

              // Et on la stocke en RAM
              master1.wb_write_blk(img_adr_out + ((tile_nb / tile_line) * T_H + line_nb ) * p_WIDTH + (tile_nb % tile_line) * T_W,
                                   mask,
                                   pixel_pack,
                                   T_W/4);

              // /*DEB*/ std::cout << " VCALC STORE_TILE: INDICES: tile_nb:"
              //                   << tile_nb
              //                   << " ADRESSE:"
              //                   << img_adr_out + ((tile_nb / tile_line) * T_H + line_nb ) * p_WIDTH + (tile_nb % tile_line) * T_W
              //                   << std::endl;

              // /*DEB*/ std::cout << " VCALC STORE_TILE: tile_processed: "
              //                   << tile_nb + 1
              //                   << std::endl;

              line_nb++;

              if (line_nb == T_H)
              {
                line_nb = 0;
                tile_nb++;
              }

            }
          }

          // Si on a terminé une image on met l'interruption
          // p_interrupt à 1 pour dire au processeur que le
          // traitement est terminé
          if (tile_nb == T_NB && get_tile_end)
          {
            tile_nb = 0;
            stockage_ok = false;
            get_tile_end = false;
            p_interrupt = 1;
            wait();
            wait();
            wait();
            p_interrupt = 0;
            /*DEB*/ std::cout << " VCALC STORE_TILE: INTERRUPTION SENT" << std::endl;
          }

        }
        wait();
      }
    }
  }}

