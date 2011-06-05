/**********************************************************************
 *
 *  Module de calcul de transformation géométrique
 *  Pour l'instant :
 *  1 TO DO (Va chercher les coeffs en RAM et les mets dans un buffer.)
 *  2 Va chercher des tuiles de B_W * B_H en RAM et les mets dans un buffer.
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
      SC_THREAD(get_buffer);
      sensitive << clk.pos();

      // Module de calcul incrémental
      // * Effectue le calcul incrémental
      // * Pose les tuiles calculées dans la fifo
      SC_THREAD(process_tile);
      sensitive << clk.pos();

      // Prend les tuiles calculées dans la fifo
      // et les met en RAM
      SC_THREAD(store_tile);
      sensitive << clk.pos();

      std::cout << "Video_calc "  << name()
                << " was created successfully " << std::endl;
    }

    ///////////////////////////
    // Get_buffer
    //////////////////////////

    /* Lorsque process_tile le demande (ask_buffer à 1), get_buffer :
     *   - Remplit un buffer avec une zone de taille B_W * B_H de
     *     l'image située à l'adresse RAM wb_tab[4].
     *   - TO DO (Remplit un buffer avec les coeffs correspondant à
     *     la tuile en cours de traitement.)
     */

    tmpl(void)::get_buffer()
    {

      tile_nb = 0;

      for (;;)
      {
        // Le buffer n'est pas rempli
        buffer_rdy = false;

        // Si un reset a lieu on réinitialise
        // toutes les variables
        if (reset_n == false)
        {
          buffer_rdy = false;
          /*DEB*/ std::cout <<"VCALC get_buffer reset ok" << std::endl;
        }

        /*DEB*/ std::cout <<"VCALC get_buffer START" << std::endl;

        // Si nouvelle image, on met à jour l'adresse
        // de l'image en RAM
        if (tile_nb == 0)
        {
          /*DEB*/ std::cout <<"VCALC get_buffer wait for an address in" << std::endl;
          while (wb_tab[5] == 0)
            wait();
          /*DEB*/ std::cout << "VCALC get_buffer got an address in" << std::endl;
          img_adr_in = wb_tab[4];
          wb_tab[5] = 0;
          /*DEB*/ std::cout <<"VCALC get_buffer wait for an address out" << std::endl;
          while (wb_tab[7] == 0)
            wait();
          /*DEB*/ std::cout << "VCALC get_buffer got an address out" << std::endl;
          img_adr_out = wb_tab[6];
          wb_tab[6] = 0;
          // On mets à jour le nombre de tuile traitées
          tile_nb = 1;
        }
        // Si on possède une adresse valide
        else
        {
          // On attend que process_tile fasse une demande
          // de remplissage du buffer.
          while (!ask_buffer)
            wait();

          // Remplissage du buffer
          /*DEB*/ std::cout << "VCALC get_buffer is filling the buffer" << std::endl;
          buffer_fill(img_adr_in, tile_nb);

          // Le buffer est maintenant rempli
          /*DEB*/ std::cout << "VCALC get_buffer the buffer is now full" << std::endl;
          buffer_rdy = true;

          /*DEB*/ std::cout << "VCALC get_buffer END " << tile_nb << std::endl;
          wait();
        }
      }
    }

    ////////////////////////////
    // Process_tile
    ////////////////////////////

    /* Calcule l'antécédent de la tuile courante,une fois que l'antécédent du centre
     * de la tuile a été calculé, demande le remplissage du cache.
     * Une fois que le cache a été obtenu (buffer_rdy à 1),
     * remplit la fifo avec les pixels de la tuile résultat.
     */

    // TO DO : Pour l'instant on se contente de ressortir l'image sans modification.

    tmpl(void)::process_tile()
    {
      // Tableau des antécédents des pixels
      // de la tuile traitée
      int invimg_c[T_H][T_W];
      int invimg_l[T_H][T_W];

      for (;;)
      {
        // On ne demande pas encore de remplir le buffer
        ask_buffer = false;
        // On a pas terminé encore les calculs
        process_rdy = false;

        /*DEB*/ std::cout << "VCALC process_tile START" << std::endl;

        // Si un reset a lieu on réinitialise
        // toutes les variables
        if (reset_n == false)
        {
          tile_nb = 0;
          process_rdy = false;
          ask_buffer = false;
          img_adr_in = wb_tab[4];
          img_adr_out = wb_tab[6];
          wb_tab[5] = 0;
          wb_tab[7] = 0;
          /*DEB*/ std::cout <<"VCALC process_tile reset ok" << std::endl;
        }

        //Calcul du centre
        /*DEB*/ std::cout <<"VCALC process_tile center coord calculus" << std::endl;
        process_center(tile_nb);

        //Demande de remplissage du cache
        /*DEB*/ std::cout <<"VCALC process_tile calcul asking to fill buffer" << std::endl;
        ask_buffer = true;

        //Calcul des antécédents
        process_invimg(tile_nb, invimg_c, invimg_l);
        /*DEB*/ std::cout <<"VCALC  process_invimg finished : wait for buffer to be fill in" << std::endl;

        //On attend que le buffer soit rempli
        while (!buffer_rdy)
          wait();

        /*DEB*/ std::cout <<"VCALC process_tile buffer ready" << std::endl;
        ask_buffer = false;

        //On parcourt le tableau des coordonnées
        //des antécédents. On met dans la fifo
        //les pixels du buffer correspondants
        for (int l = 0; l < T_H; l++)
          for (int c = 0; c < T_W; c++)
            //Si le pixel n'est pas dans
            //le buffer, on met un pixel noir
            if (invimg_c[l][c] < (buffer_center_c - B_W/2) ||
                invimg_c[l][c] > (buffer_center_c + B_W/2) ||
                invimg_l[l][c] < (buffer_center_l - B_H/2) ||
                invimg_l[l][c] > (buffer_center_l + B_H/2)
              )
              fifo.write(255);
        //Sinon on écrit le pixel du buffer correspondant
            else
              fifo.write(buffer[invimg_l[l][c] % B_H][invimg_c[l][c] % B_W]);
        process_rdy = true;

        /*DEB*/ std::cout <<"VCALC process_tile END" << std::endl;
        wait();
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
      //Une ligne de pixels groupés par paquets de 4
      uint32_t pixel_pack[T_W/4];
      uint8_t mask[T_W/4];

      for (int i = 0; i < T_W/4; i++)
        mask[i] = 0xff;

      img_rdy = false;

      for (;;)
      {
        /*DEB*/ std::cout << "VCALC store_tile START" << std::endl;

        // Si un reset a lieu on réinitialise
        // toutes les variables
        if (reset_n == false)
        {
          img_rdy = false;
          /*DEB*/ std::cout <<"VCALC store_tile reset ok" << std::endl;
        }

        while (!process_rdy)
          wait();

        //Dès que l'on a une ligne de tuiles en pixels on la stocke
        //en RAM. On garde le compte de la position dans l'image

        //Pour chaque ligne de tuile
        for (int i = 0; i < T_H; i++)
        {
          //On récupère la ligne de tuile
          for (int j = 0; j < T_W/4; j++)
          {
            //On récupère les pixels 4 par 4
            for (int p = 0; p < 3; p++)
            {
              pixel_pack[j] = pixel_pack[j] << 8;
              pixel_pack[j] = fifo.read();
            }
          }
            //Et on la stocke en RAM
          master1.wb_write_blk(img_adr_out + (i * p_WIDTH) + (tile_nb % T_OUT_L_NB - 1) * T_W + (tile_nb / T_OUT_L_NB) * T_W * T_H,
                               mask,
                               pixel_pack,
                               T_W/4);
        }

        // tuile suivante
        tile_nb++;
        tile_nb = (tile_nb + 1) % (T_OUT_NB + 1);
        // Si on a terminé une image on met l'interruption
        // img_rdy à 1 pour dire au processeur que le
        // traitement est terminé
        if (tile_nb == 0)
          img_rdy = true;
        // Sinon on attend
        else
          img_rdy = false;

        /*DEB*/ std::cout << "VCALC store_tile END" << std::endl;

      }
      wait();
    }

    /////////////////////////////
    // Process_invimg
    /////////////////////////////

    /* Calcul les coordonnées des antécédents des pixels de la tuile
     * en cours de traitement.
     * Met ces antécédents dans deux tableaux :
     *   - invimg_c pour les coordonnées en c
     *   - invimg_l pour les coordonnées en l
     */

    //TODO CHANGE THIS : pour l'instant on ne fait
    //aucun calcul sur l'image. Le centre du buffer est
    //le centre de la tuile
    tmpl(void)::process_invimg(int tile_nb, int invimg_c[T_H][T_W], int invimg_l[T_H][T_W])
    {
      /*DEB*/ std::cout <<"VCALC process_invimg" << std::endl;

      //Coin en haut à gauche
      int pixel_c = ((tile_nb - 1) % T_IN_L_NB) * B_W;
      int pixel_l = ((tile_nb - 1) / T_IN_L_NB) * B_H;

      /*DEB*/ std::cout <<"VCALC process_invimg is filling invimg_c" << std::endl;
      for (int l = 0; l < T_H;  l++)
        for (int c = 0; c < T_W; c++)
        {
          /*DEB*/ std::cout <<"VCALC process_invimg l " << l << " c  "<< c << std::endl;
          invimg_c[l][c] = pixel_c + c;
          invimg_l[l][c] = pixel_l + l;
        }

      /*DEB*/ std::cout <<"VCALC process_invimg finished" << std::endl;
    }

    /////////////////////////////
    // Process_center
    /////////////////////////////

    /* Calcul les coordonnées du centre de la zone de buffer à récuperer.
     */

    // TO DO :
    // Pour l'instant on ne fait aucun calcul sur l'image.
    // Le centre du buffer est le centre de la tuile
    tmpl(void)::process_center(int tile_nb)
    {
      buffer_center_c = ((tile_nb - 1) % T_IN_L_NB) * B_W + B_W/2;
      buffer_center_l = ((tile_nb - 1) / T_IN_L_NB) * B_H + B_H/2;
    }

    /////////////////////////////
    // Buffer_fill
    /////////////////////////////

    /* Calcul l'intersection de la zone à récupérer avec les images
     * et remplit le buffer avec cette zone.
     */

    tmpl(void)::buffer_fill(uint32_t img_adr_in, int tile_nb)
    {
      // Coordonnées du point en haut à gauche
      // de la zone de buffer à remplir
      // Peut se trouver en dehors de l'image réelle
      int32_t buffer_l = 0;
      int32_t buffer_c = 0;

      // Coordonnées du point en haut à gauche du buffer
      // qui est vraiment dans l'image
      int32_t buffer_im_l = 0;
      int32_t buffer_im_c = 0;

      // Taille de la zone de buffer qui est dans l'image
      int32_t buffer_im_h;
      int32_t buffer_im_w;

      // Ligne de buffer en cours de traitement
      int32_t buffer_line = 0;
      uint32_t buffer_line_temp[B_W/4];

      // Calcul des coordonnées du pixel en haut à gauche de la zone
      // à traiter
      buffer_c = buffer_center_c - B_W/2;
      buffer_l = buffer_center_l - B_H/2;

      //On assure les accès mémoires alignés:
      buffer_c -= buffer_c % 4;
      buffer_l -= buffer_l % 4;

      // ATTENTION : il est possible que la zone de buffer ne soit
      // pas forcément dans l'image

      // On remplit la zone buffer de 0 (pixels noirs)
      for (int i = 0; i < B_H; i++)
        for (int j = 0; j < B_W; j++)
          buffer[i][j] = 0;

      // Calcul de la zone de buffer qu'il faut remplir avec
      // une recherche en mémoire

      // Cas où la zone de buffer est complétement en dehors de l'image
      // La zone de buffer doit être laissée noire
      if (buffer_c > (int32_t) (p_WIDTH - 1)  ||
          buffer_l > (int32_t) (p_HEIGHT - 1)
        )
        return;

      // Sinon on calcule l'intersection entre la zone de buffer
      // et l'image

      // Par défaut la taille du buffer est la taille d'une
      // tuile d'entrée
      buffer_im_w = B_W;
      buffer_im_h = B_H;

      // Par défaut les coordonnées du pixel en haut à gauche
      // du buffer qui est véritablement dans l'image
      // sont les même
      buffer_im_c = buffer_c;
      buffer_im_l = buffer_l;

      // Dépassement à gauche
      if (buffer_c < 0)
      {
        buffer_im_c = 0;
        if ((-buffer_c) > B_W)
          return;
        else
          buffer_im_w += buffer_c;
      }

      // Dépassement à droite
      if ((buffer_c + B_W) > (int32_t) (p_WIDTH - 1))
        buffer_im_w -= (buffer_c + B_W - p_WIDTH);

      // Dépassement en haut
      if (buffer_l < 0)
      {
        buffer_im_l = 0;
        if ((-buffer_l) > B_H)
          return;
        buffer_im_l += buffer_l;
      }

      // Dépassement en bas
      if (buffer_l + B_H > (int32_t) p_HEIGHT - 1)
        buffer_im_h -= (buffer_l + B_H - p_HEIGHT);

      // On fait une lecture bloc ligne par ligne
      // On récupere chaque ligne composées d'éléments
      // de 4 pixels que l'on met ensuite dans le buffer.

      for (buffer_line = 0; buffer_line < buffer_im_h; buffer_line++)
      {
        master0.wb_read_blk(img_adr_in + ((buffer_im_l + buffer_line) * p_WIDTH) + buffer_im_c,
                            buffer_im_w / 4,
                            buffer_line_temp);

        for (int i = 0; i < (buffer_im_w / 4); i++)
          for (int j = 0; j < 4; j++)
          {
            buffer[(buffer_im_l + buffer_line) % B_H][(buffer_im_c + i + j) % B_W] = buffer_line_temp[i];
            buffer_line_temp[j] >>= 8;
          }
      }
    }

  }
}




