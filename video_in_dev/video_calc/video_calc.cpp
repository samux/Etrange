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

      std::cout <<  name()
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
      // Nb de tuiles traitées dans get_buffer
      uint32_t tile_nb = 0;

      wb_tab[5] = 0;
      wb_tab[7] = 0;

      for (;;)
      {
        /*DEB*/ std::cout << " VCALC GET_BUFFER: START " << std::endl;

        // Le buffer n'est pas rempli
        buffer_rdy = false;

        // Si un reset a lieu on réinitialise
        // toutes les variables
        if (reset_n == false)
        {
          tile_nb = 0;
          buffer_rdy = false;
          /*DEB*/ std::cout << " VCALC GET_BUFFER: RESET " << std::endl;
          wait();
        }
        // Si nouvelle image, on met à jour l'adresse
        // de l'image en RAM
        if (!img_proc)
        {
          /*DEB*/ std::cout << " VCALC GET_BUFFER: wait for address IN " << std::endl;
          while (wb_tab[5] == 0)
            wait();
          /*DEB*/ std::cout << " VCALC GET_BUFFER: address IN OK " << std::endl;
          img_adr_in = wb_tab[4];
          wb_tab[5] = 0;

          /*DEB*/ std::cout << " VCALC GET_BUFFER: wait for address OUT " << std::endl;
          while (wb_tab[7] == 0)
            wait();
          /*DEB*/ std::cout << " VCALC GET_BUFFER: address OUT OK " << std::endl;
          img_adr_out = wb_tab[6];
          wb_tab[7] = 0;

          // On affiche la valeur des adresses récupérées
          /*DEB*/ std::cout << " VCALC GET_BUFFER: address IN: " << img_adr_in << std::endl;
          /*DEB*/ std::cout << " VCALC GET_BUFFER: address OUT: " << img_adr_out << std::endl;

          // On vient d'obtenir une nouvelle image
          // on commence le traitement
          img_proc = true;
        }
        // Si on possède une adresse valide
        else
        {
          // On attend que process_tile fasse une demande
          // de remplissage du buffer.
          ///*DEB*/ std::cout << " VCALC GET_BUFFER: ask_buffer ?? " << std::endl;
          while (!ask_buffer)
            wait();
          ///*DEB*/ std::cout << " VCALC GET_BUFFER: ask_buffer OK " << std::endl;

          // Remplissage du buffer
          /*DEB*/ std::cout << " VCALC GET_BUFFER: FILLING buffer " << std::endl;
          buffer_fill(img_adr_in, tile_nb);

          // Le buffer est maintenant rempli
          /*DEB*/ std::cout << " VCALC GET_BUFFER: buffer FULL" << std::endl;
          buffer_rdy = true;
          wait();

          /*DEB*/ std::cout << " VCALC GET_BUFFER: tile processed: " << tile_nb + 1 << std::endl;
          // On incrémente le nombre de tuile traitées dans get_buffer
          tile_nb++;
          // On remets à 0 si on a traité toutes les tuiles de l'image
          // et on annonce qu'on a terminé une image dans ce thread
          if (tile_nb == T_OUT_NB)
          {
            tile_nb = 0;
            wb_tab[5] = 0;
            wb_tab[7] = 0;
            while (img_proc)
              wait();
          }
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
      // Nombre de tuiles traitées dans process_tile
      uint32_t tile_nb = 0;

      // Tableau des antécédents des pixels
      // de la tuile traitée
      int32_t invimg_c[T_H][T_W];
      int32_t invimg_l[T_H][T_W];

      // Coordonnées du point en haut à gauche
      // du buffer
      // Peut se trouver en dehors de l'image réelle
      int32_t buffer_c = 0;
      int32_t buffer_l = 0;

      // Distance entre le point en haut à gauche
      // du buffer et l'antécédent en cours de
      // traitement
      int32_t distance_c = 0;
      int32_t distance_l = 0;

      // On ne demande pas encore de remplir le buffer
      ask_buffer = false;

      for (;;)
      {
        /*DEB*/ std::cout <<  " VCALC PROCESS_TILE: START " << std::endl;

        // Si un reset a lieu on réinitialise
        // toutes les variables
        if (reset_n == false)
        {
          tile_nb = 0;
          img_proc = false;
          ask_buffer = false;
          img_adr_in = wb_tab[4];
          img_adr_out = wb_tab[6];
          wb_tab[5] = 0;
          wb_tab[7] = 0;
          /*DEB*/ std::cout << " VCALC PROCESS_TILE: RESET " << std::endl;
          wait();
        }

        //Calcul du centre
        process_center(tile_nb);
        /*DEB*/ std::cout << " VCALC PROCESS_TILE: tile center OK" << std::endl;

        //Demande de remplissage du cache
        ///*DEB*/ std::cout << " VCALC PROCESS_TILE: asking to fill buffer " << std::endl;
        ask_buffer = true;
        wait();

        //Calcul des antécédents
        ///*DEB*/ std::cout << " VCALC PROCESS_TILE: starting back wrapping " << std::endl;
        process_invimg(tile_nb, invimg_c, invimg_l);
        /*DEB*/ std::cout << " VCALC PROCESS_TILE: back wrapping OK " << std::endl;

        //On attend que le buffer soit rempli
        ///*DEB*/ std::cout << " VCALC PROCESS_TILE: buffer full ?? " << std::endl;
        while (!buffer_rdy)
          wait();
        /*DEB*/ std::cout << " VCALC  PROCESS_TILE: buffer full OK " << std::endl;

        // On annule la demande de remplissage du cache
        ask_buffer = false;
        wait();

        buffer_c = buffer_center_c - B_W / 2;
        buffer_l = buffer_center_l - B_H / 2;

        // On parcourt le tableau des coordonnées
        // des antécédents. On met dans la fifo
        // les pixels du buffer correspondants
        for (int l = 0; l < T_H; l++)
          for (int c = 0; c < T_W; c++)
            // Si le pixel n'est pas dans
            // le buffer, on met un pixel noir
            if (invimg_c[l][c] < (buffer_center_c - B_W / 2) ||
                invimg_c[l][c] > (buffer_center_c + B_W / 2) ||
                invimg_l[l][c] < (buffer_center_l - B_H / 2) ||
                invimg_l[l][c] > (buffer_center_l + B_H / 2)
              )
              fifo.write(255);
        // Sinon on écrit le pixel du buffer correspondant
            else
            {
              distance_c = invimg_c[l][c] - buffer_c;
              distance_l = invimg_l[l][c] - buffer_l;

              // /*DEB*/ std::cout << "     VCALC PROCESS_TILE: DISTANCE: tile_nb:"
              //                   << tile_nb
              //                   << " distance_l:"
              //                   << distance_l
              //                   << " distance c:"
              //                   << distance_c
              //                   << std::endl;

              fifo.write(buffer[distance_l][distance_c]);
            }
        wait();

        /*DEB*/ std::cout << " VCALC PROCESS_TILE: tile_processed: " << tile_nb + 1 << std::endl;
        // On incrémente le nombre de tuile traitées dans process_tile
        tile_nb++;
        // On remets à 0 si on a traité toutes les tuiles de l'image
        if (tile_nb == T_OUT_NB)
        {
          tile_nb = 0;
          while (img_proc)
            wait();
        }

        /*DEB*/ std::cout << " VCALC PROCESS_TILE END " << std::endl;
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
      uint32_t tile_nb = 0;
      int tile_line = p_WIDTH / T_W;

      //Une ligne de pixels groupés par paquets de 4
      uint32_t pixel_pack[T_W/4];
      uint8_t mask[T_W/4];

      for (int i = 0; i < T_W/4; i++)
        mask[i] = 0xf;

      for (;;)
      {
        /*DEB*/ std::cout << " VCALC STORE_TILE: START" << std::endl;

        // Si un reset a lieu on réinitialise
        // toutes les variables
        if (reset_n == false)
        {
          tile_nb = 0;
          p_interrupt = 0;
          /*DEB*/ std::cout <<" VCALC STORE_TILE: RESET" << std::endl;
          wait();
        }

        while ((uint32_t) fifo.num_available() < (uint32_t) (T_W * T_H))
          wait();

        // Pour chaque ligne de tuile
        for (int i = 0; i < T_H; i++)
        {
          // On récupère la ligne de tuile
          for (int j = 0; j < T_W/4; j++)
            // On récupère les pixels 4 par 4
            for (int p = 0; p < 4; p++)
            {
              pixel_pack[j] = pixel_pack[j] << 8;
              pixel_pack[j] += fifo.read();
            }

          // Et on la stocke en RAM
          master1.wb_write_blk(img_adr_out + ((tile_nb / tile_line) * T_H + i ) * p_WIDTH + (tile_nb % tile_line) * T_W,
                               mask,
                               pixel_pack,
                               T_W/4);

          //img_addr_out + ( tile_nb / tile_line) * T_W * T_H + i * p_WIDTH + (tile_nb % (tile_line - 1) * T_W

          // /*DEB*/ std::cout << " VCALC STORE_TILE: INDICES: tile_nb:"
          //                   << tile_nb
          //                   << " ADRESSE:"
          //                   << img_adr_out + ((tile_nb / tile_line) * T_H + i ) * p_WIDTH + (tile_nb % tile_line) * T_W
          //                   << std::endl;
        }

        /*DEB*/ std::cout << " VCALC STORE_TILE: tile_processed: "
                          << tile_nb + 1
                          << std::endl;

        tile_nb++;
        // Si on a terminé une image on met l'interruption
        // p_interrupt à 1 pour dire au processeur que le
        // traitement est terminé
        if (tile_nb == T_OUT_NB)
        {
          img_proc = false;
          p_interrupt = 1;
          wait();
          wait();
          wait();
          p_interrupt = 0;
          tile_nb = 0;
          /*DEB*/ std::cout << " VCALC STORE_TILE: INTERRUPTION SENT" << std::endl;
        }

        /*DEB*/ std::cout << " VCALC STORE_TILE: END " << std::endl;
      }
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
      int tile_line = p_WIDTH / T_W;
      //Coin en haut à gauche
      int pixel_c = tile_nb % tile_line * T_W;
      int pixel_l = tile_nb / tile_line * T_H;

      // /*DEB*/ std::cout << "     VCALC PROCESS_TILE: INVIMG: tile_nb:"
      //                   << tile_nb
      //                   << " col:"
      //                   << pixel_c
      //                   << " line:"
      //                   << pixel_l
      //                   << std::endl;

      for (int l = 0; l < T_H;  l++)
        for (int c = 0; c < T_W; c++)
        {
          invimg_c[l][c] = pixel_c + c;
          invimg_l[l][c] = pixel_l + l;
          // /*DEB*/ std::cout << "     VCALC PROCESS_TILE: INVIMG: tile_nb:"
          //                   << tile_nb
          //                   << " col:"
          //                   << invimg_c[l][c]
          //                   << " line:"
          //                   << invimg_l[l][c]
          //                   << std::endl;
        }
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
      int tile_line = p_WIDTH / T_W;
      buffer_center_c = tile_nb % tile_line * T_W + T_W/2;
      buffer_center_l = tile_nb / tile_line * T_H + T_H/2;

      // /*DEB*/ std::cout << "     VCALC PROCESS_TILE: CENTER: tile_nb:"
      //                   << tile_nb
      //                   << " col:"
      //                   << buffer_center_c
      //                   << " line:"
      //                   << buffer_center_l
      //                   << std::endl;
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

      // Décalage dans le buffer
      int32_t decalage_h = 0;
      int32_t decalage_w = 0;

      // Ligne de buffer en cours de traitement
      int32_t buffer_line = 0;
      uint32_t buffer_line_temp[B_W/4];

      int k;

      // Calcul des coordonnées du pixel en haut à gauche de la zone
      // à traiter

      buffer_c = buffer_center_c - B_W / 2;
      buffer_l = buffer_center_l - B_H / 2;

      // /*DEB*/ std::cout << "     VCALC GET_BUFFER: COIN GAUCHE (OP): tile_nb:"
      //                   << tile_nb
      //                   << " col:"
      //                   << buffer_c
      //                   << " line:"
      //                   << buffer_l
      //                   << std::endl;

      // //On assure les accès mémoires alignés:
      // buffer_c -= buffer_c % 4;
      // buffer_l -= buffer_l % 4;
      // /*DEB*/ std::cout << "     VCALC GET_BUFFER: COIN GAUCHE (OP aligné): tile_nb:"
      //                   << tile_nb
      //                   << " col:"
      //                   << buffer_c
      //                   << " line:"
      //                   << buffer_l
      //                   << std::endl;

      // ATTENTION : il est possible que la zone de buffer ne soit
      // pas forcément dans l'image

      // On remplit la zone buffer de 0 (pixels noirs)
      for (int i = 0; i < B_H; i++)
        for (int j = 0; j < B_W; j++)
          buffer[i][j] = 255;

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

        buffer_im_w += buffer_c;
        decalage_w = B_W - buffer_im_w;

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

        buffer_im_h += buffer_l;
        decalage_h = B_H - buffer_im_h;
      }

      // Dépassement en bas
      if (buffer_l + B_H > (int32_t) p_HEIGHT - 1)
        buffer_im_h -= (buffer_l + B_H - p_HEIGHT);

    //   /*DEB*/ std::cout << "     VCALC GET_BUFFER: COIN GAUCHE (IP): tile_nb:"
    //                     << tile_nb
    //                     << " col:"
    //                     << buffer_im_c
    //                     << " line:"
    //                     << buffer_im_l
    //                     << std::endl;

    //   /*DEB*/ std::cout << "     VCALC GET_BUFFER: DIMENSIONS (IP): tile_nb:"
    //                     << tile_nb
    //                     << " width:"
    //                     << buffer_im_w
    //                     << " height:"
    //                     << buffer_im_h
    //                     << std::endl;

    // /*DEB*/ std::cout << "     VCALC GET_BUFFER: DECALAGE (IP): tile_nb:"
    //                     << tile_nb
    //                     << " decalage_w:"
    //                     << decalage_w
    //                     << " decalage_h:"
    //                     << decalage_h
    //                     << std::endl;


      // On fait une lecture bloc ligne par ligne
      // On récupere chaque ligne composées d'éléments
      // de 4 pixels que l'on met ensuite dans le buffer
      // au bon endroit

      for (buffer_line = 0; buffer_line < buffer_im_h; buffer_line++)
      {
        master0.wb_read_blk(img_adr_in + ((buffer_im_l + buffer_line) * p_WIDTH) + buffer_im_c,
                            buffer_im_w / 4,
                            buffer_line_temp);

    // /*DEB*/ std::cout << "     VCALC GET_BUFFER: ADRESSES: tile_nb:"
    //                   << tile_nb
    //                   << " ADRESSE:"
    //                   << img_adr_in + ((buffer_im_l + buffer_line) * p_WIDTH) + buffer_im_c
    //                   << std::endl;

        k = 0;
        for (int i = 0; i < (buffer_im_w / 4); i++)
        {
          for (int j = 3; j>= 0; j--)
          {
            buffer[buffer_line + decalage_h][k + j + decalage_w] = buffer_line_temp[i] >> 8*j;
            buffer_line_temp[i] = buffer_line_temp[i] - ((buffer_line_temp[i] >> 8*j) << 8*j);
          }
          k +=4;
        }

      }
    }

  }
}


