/**********************************************************************
 *
 *  Module de calcul de transformation géométrique
 *  Pour l'instant :
 *  1 TO DO (Va chercher les coeffs en RAM et les mets dans un buffer.)
 *  2 Va chercher des tuiles de (T_IN_SIZE)² en RAM et les mets dans un buffer.
 *  3 Effectue le calcul sur chaque tuile de (T_OUT_SIZE)²
 *  4 Stocke les tuiles transformées de (T_OUT_SIZE)² en RAM
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
               fifo(F_SIZE * T_OUT_SIZE * T_OUT_SIZE),
               wb_tab(tab),
               master0(p_clk,p_resetn,p_wb)
    {

      // Module de remplissage du buffer
      // * Bufferise les tuiles d'entrée
      // * TO DO (Bufferise les coeffs)
      SC_THREAD(get_buffer);
      sensitive << clk.pos();

      // Module de calcul incrémental
      // Pose les tuiles calculées dans la fifo
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
   *   - Remplit un buffer avec une zone de taille (T_IN_SIZE)² de
   *     l'image située à l'adresse RAM wb_tab[4].
   *   - TO DO (Remplit un buffer avec les coeffs correspondant à
   *     la tuile en cours de traitement.)
   */

    tmpl(void)::get_buffer()
    {
      /*DEB*/ std::cout <<"VCALC get_buffer in progress" << std::endl;

      // Adresse de l'image en RAM
      uint32_t img_adr;
      wb_tab[4] = RAM_BASE;
      wb_tab[5] = 0;
      img_addr = wb_tab[4];

      // Numéro de la tuile en train d'être traitée.
      //   - Si vaut 0, c'est que l'on est entre deux images.
      //   - Si vaut n, on traite la nième tuile de l'image
      uint32_t nb_tile = 0;

      for (;;)
      {
        buffer_rdy = false;
        img_rdy = false;

        if (reset_n == false)
        {
          img_adr = wb_tab[4];
          wb_tab[5] = 0;
          tile_nb = 0;
          buffer_rdy = false;
          img_rdy = false;
          /*DEB*/ std::cout <<"VCALC get_cache reset ok" << std::endl;
        }

        // Si nouvelle image, on met à jour l'adresse
        // de l'image en RAM
        if (tile_nb == 0)
        {
          /*DEB*/ std::cout <<"VCALC get_buffer wait for a new adress" << std::endl;
          while (wb_tab[5] == 0)
            wait();
          img_adr = wb_tab[4];
          wb_tab[5] = 0;
          tile_nb = 1;
        }
        // Si on possède une adresse valide
        else
        {
          /*DEB*/ std::cout << "VCALC get_buffer got a new adress" << std::endl;

          // On attend que process_tile fasse une demande
          // de remplissage du buffer.
          while (!ask_buffer)
            wait();

          // Remplissage du buffer
          /*DEB*/ std::cout << "VCALC get_buffer is filling the buffer" << std::endl;
          buffer_fill(img_adr, tile_nb);

          // Le buffer est maintenant rempli
          /*DEB*/ std::cout << "VCALC get_buffer the buffer is now full" << std::endl;
          buffer_rdy = true;

          //On se prépare à traiter la tuile suivante
          tile_nb = tile_nb % TILE_IN_NUMBER;

          //Fin d'une image
          if (tile_nb == 0)
          {
            img_rdy = true;
            /*DEB*/ std::cout << "VCALC get_buffer new image ready " << tile_nb<< std::endl;
          }

          /*DEB*/ std::cout << "VCALC get_buffer eob " << tile_nb<< std::endl;
        }
      }
    }

  ////////////////////////////
  // Process_tile
  ////////////////////////////

  /* Calcule l'antécédent de la tuile courante,une fois que l'antécédent du centre
   * de la tuile a été calculé, demande le remplissage du cache.
   * Une fois que le cache a été obtenu (pixel_ready à 1),
   * remplit la fifo avec les pixels de la tuile résultat.
   */
  // TO DO : Pour l'instant on se contente de ressortir l'image sans modification.

    tmpl(void)::process_tile() {

      uint32_t tile_nb = 0;

      /*DEB*/ std::cout <<"VCALC process_tile in progress" << std::endl;

      // Tableau des antécédents des pixels
      // de la tuile traitée
      int invimg_c[T_H][T_W];
      int invimg_l[T_H][T_W];

      tile_nb++;

      //Calcul du centre
      /*DEB*/ std::cout <<"VCALC process_tile center calculus" << std::endl;
      process_center(tile_nb);

      /*DEB*/ std::cout <<"VCALC process_tile calcul asking to fill buffer" << std::endl;
      //Demande de remplissage du cache
      ask_buffer = true;

      //Calcul des antécédents
      process_invimg(tile_nb, invimg_c, invimg_l);
      /*DEB*/ std::cout <<"VCALC process_tile process_invimg finished : wait for buffer to be full" << std::endl;

      //On attend que le buffer soit rempli
      while (buffer_rdy == false)
        wait();
      ask_buffer = false;
      /*DEB*/ std::cout <<"VCALC process_tile buffer ready" << std::endl;

      //On parcourt le tableau des coordonnées
      //des antécédents. On met dans la fifo
      //les pixels du cache correspondants
      for (int l = 0; l<T_H; l++)
        for (int c = 0; c<T_W; c++)
          //Si le pixel n'est pas dans
          //le cache, on met un pixel noir
          if (invimg_c[l][c] < (cache_center_c - C_W/2) ||
              invimg_c[l][c] > (cache_center_c + C_W/2) ||
              invimg_l[l][c] < (cache_center_l - C_H/2) ||
              invimg_l[l][c] > (cache_center_l + C_H/2)
            )
            fifo.write(255);
      //Sinon on écrit le pixel du cache correspondant
          else fifo.write(cache[invimg_l[l][c]%C_H][invimg_c[l][c]%C_W]); //XXX TODO CHANGE THIS, C EST PAS CA

      /*DEB*/ std::cout <<"VCALC process_tile buffer in FIFO" << std::endl;
    }
    wait();
  }

    /////////////////////////////
    // Store_tile
    /////////////////////////////

    /* Stocke les tuiles de la fifo à l'adresse de la RAM stockée dans wb_tab[6]
     * indique au processeur quand l'image a fini d'être stockée
     */
    tmpl(void)::store_tile() {
      //Tuile en cours de stockage
      //0 signifie que l'on est entre 2 images
      int tile_nb = 0;
      //Ligne de tuile en cours de stockage, on stocke
      //par une écriture bloc, ligne de tuile par ligne
      //de tuile
      int tile_line = 0;
      //adresse de l'image en RAM
      int img_addr = 0;
      //Interruption pour indique la fin de l'écriture d'une image
      img_w = false;

      //Une ligne de pixels groupés par paquets de 4
      uint32_t pixel_pack[T_W/4];
      uint8_t mask[T_W/4];
      for (int i = 0; i<T_W/4; i++) {
        mask[i] = 0xff;
      }

      /*DEB*/ std::cout <<"VCALC store_tile ICI" << std::endl;
      for (;;) {
        img_w = false;
        if (reset_n == false)  {
          tile_nb = 0;
          tile_line = 0;
          img_addr = 0;
          /*DEB*/ std::cout <<"VCALC store_tile reset OK" << std::endl;
        }
        else {
          //Si on est entre 2 images, on attend
          //de disposer d'une adresse fournie par le
          //proces:wseur
          if (tile_nb == 0) {
            /*DEB*/ std::cout <<"VCALC store_tile attend une adresse" << std::endl;
            while (wb_tab[7] != 1)
              wait();
            img_addr = wb_tab[6];
            wb_tab[7] = 0;
            tile_nb = 1;
            /*DEB*/ std::cout <<"VCALC store_tile a une adresse" << std::endl;
          }
          //Dès que l'on a une ligne de tuiles en pixels on la stocke
          //en RAM. On garde le compte de la position dans l'image
          else {
            //Parce qu'on aime les boucles for...

            //Pour chaque ligne de tuile
            for (int l = 0; l < T_H; l++)
            {
              //On récupère la ligne de tuile
              for (int c = 0; c < T_W/4; c++)
              {
                //On récupère les pixels 4 par 4
                for (int p = 0; p < 3; p++) {
                  pixel_pack[c] = pixel_pack[c] << 8;
                  pixel_pack[c] = fifo.read();
                }
              }
              //Et on la stocke en RAM
              //p_WIDTH/T_W = nombre de tuiles par ligne
              master0.wb_write_blk(	img_addr + (tile_nb/(p_WIDTH/T_W))*T_W*T_H + l*p_WIDTH + (tile_nb%(p_WIDTH/T_W) - 1)*T_W,
                                        mask,
                                        pixel_pack,
                                        T_W/4);
            }
            //tuile suivante
            tile_nb = (tile_nb +1)%((p_WIDTH*p_HEIGHT)/(T_W*T_H) + 1);
            if (tile_nb == 0) {
              img_w = true;
              wait();
            }
          }
        }
      }
    }

    //TODO CHANGE THIS : pour l'instant on ne fait
    //aucun calcul sur l'image. Le centre du cache est
    //le centre de la tuile
    tmpl(void)::process_invimg(int tile_nb, int invimg_c[T_H][T_W], int invimg_l[T_H][T_W]) {
      /*DEB*/ std::cout <<"VCALC process_invimg ICI" << std::endl;
      int tile_line = p_WIDTH / C_W;
      //Coin en haut à gauche
      int pixel_c = (tile_nb-1)%tile_line*C_W;
      int pixel_l = (tile_nb-1)/tile_line*C_H;

      /*DEB*/ std::cout <<"VCALC process_invimg c'est parti pour la boucle" << std::endl;
      for (int i = 0; i < T_H;  i++) {
        for (int j = 0; j<T_W; j++) {
          /*DEB*/ std::cout <<"VCALC process_invimg i " << i <<" j  "<< j << std::endl;
          invimg_c[i][j] = pixel_c + j;
          invimg_c[i][j] = pixel_l + i;
        }
      }
      /*DEB*/ std::cout <<"VCALC process_invimg boucle fini avec succes" << std::endl;
    }

    // TO DO :
    // Pour l'instant on ne fait aucun calcul sur l'image.
    // Le centre du buffer est le centre de la tuile
    tmpl(void)::process_center(int tile_nb)
    {
      buffer_center_c = ((tile_nb - 1) % T_IN_L_NB) * B_W + B_W/2;
      buffer_center_l = ((tile_nb - 1) / T_IN_L_NB) * B_H + B_H/2;
    }

    tmpl(void)::buffer_fill(uint32_t img_addr, int tile_nb)
    {
      // Coordonnées du point en haut à gauche
      // de la zone de buffer à remplir
      // Peut se trouver en dehors de l'image réelle
      uint32_t buffer_l = 0;
      uint32_t buffer_c = 0;

      // Coordonnées du point en haut à gauche du buffer
      // qui est vraiment dans l'image
      uint32_t buffer_im_l = 0;
      uint32_t buffer_im_c = 0;

      // Taille de la zone de buffer qui est dans l'image
      uint32_t buffer_im_h;
      uint32_t buffer_im_w;

      // Ligne de buffer en cours de traitement
      uint32_t buffer_line = 0;

      // Calcul des coordonnées du pixels en haut à gauche de la zone
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
      if (buffer_c > (uint32_t) (p_WIDTH - 1)  ||
          buffer_l > (uint32_t) (p_HEIGHT - 1) ||
        )
        return;

      // Sinon on calcule l'intersection entre la zone de buffer
      // et l'image

      // Par défaut la taille du buffer est la taille d'une
      // tuile d'entrée
      buffer_im_w = B_W;
      buffer_im_h = B_H;

      // Dépassement à gauche
      if (buffer_c < 0)
      {
        buffer_im_c = 0;
        buffer_im_w += buffer_c;
      }

      // Dépassement à droite
      if ((buffer_c + B_W) > (uint32_t) (p_WIDTH - 1))
        buffer_im_w -= (buffer_c +B_W - p_WIDTH);

      // Dépassement en haut
      if (buffer_l < 0)
      {
        buffer_im_l = 0;
        buffer_im_l += buffer_l;
      }

      // Dépassement en bas
      if (buffer_l + B_H > p_HEIGHT - 1)
        buffer_im_h -= (buffer_l +B_H - p_HEIGHT);

      //Lectures blocs de chaque ligne de buffer
      for (buffer_line = 0; buffer_line < B_H; buffer_line ++)
        master0.wb_read_blk(img_addr + (tile_nb/(p_WIDTH/T_W))*T_W*T_H + (tile_nb%(p_WIDTH*T_W))*T_W + buffer_im_c ,
                            buffer_im_w, (uint32_t *)&buffer[buffer_im_l+buffer_line][buffer_im_c%C_W]);

      //On s'assure de faire des accès alignés
      buffer_l += buffer_l%4;
      buffer_c += buffer_c%4;

      for (buffer_line = 0; buffer_line < buffer_im_h; buffer_line++)
        master0.wb_read_blk(img_addr + ((buffer_im_l+buffer_line)*p_WIDTH) + buffer_im_c, buffer_im_w/4, (uint32_t *)&buffer[(buffer_im_l+buffer_line)%C_H][buffer_im_c%C_W]);
    }
  }
}

