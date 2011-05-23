/**********************************************************************
 *
 *    Futur module de calcul de transformation géométrique
 *    Pour l'instant : 
 *    -Va chercher une zone a mettre en cache dans la ram,
 *    -Met une partie de cette zone en cache dans la tuile à stocker
 *    -Stocke la tuile en RAM
 *
 *********************************************************************/    
#include "video_calc.h"
#define DEBUG_CALC 1



namespace soclib { namespace caba {

	/////////////////////////////////////////////////////////////////////
	//	Constructeur
	////////////////////////////////////////////////////////////////////

	tmpl(/**/)::VideoCalc(sc_core::sc_module_name insname,
			uint32_t * tab, int w, int h):
		sc_core::sc_module(insname),
		p_clk("p_clk"),p_resetn("p_resetn"),
		p_WIDTH(w), p_HEIGHT(h),
		fifo(F_SIZE*T_W*T_H),
		wb_tab(tab),
		master0(p_clk,p_resetn,p_wb) {
			std::cout << "VCALC constructeur " << std::endl;
			//Module de remplissage du cache
			SC_THREAD(get_cache);
			sensitive << clk.pos();

			//Module de calcul
			//Pose les tuiles calculées dans la fifo
			SC_THREAD(process_tile);
			sensitive << clk.pos();

			//Prend les tuiles calculées dans la fifo
			//et les met en RAM
			SC_THREAD(store_tile);
			sensitive << clk.pos();
			std::cout << "VCALC constructeur OK " << std::endl;

		}

	///////////////////////////
	// Get_cache
	////////////////////////// 
	/* Lorsque process_tile le demande (ask_cache à 1), get_cache
	 * remplit le cache avec une zone de taille T_W par T_H de l'image à 
	 * l'adresse RAM wb_tab[4] située autour du pixel de coordonnées cache_center_l, 
	 * cache_center_c.
	 */
	tmpl(void)::get_cache() {

		//Adresse de l'image en RAM
		uint32_t img_addr = 0;

		//Numéro de la tuile en train d'être
		//traitée. Si vaut 0, c'est que l'on est entre
		//deux images. Si vaut n, on traite la nième
		//tuile de l'image
		uint32_t tile_nb = 0;
		/*DEB*/ std::cout << "VCALC get_cache" << std::endl;

		for (;;) {
			cache_rdy = false;
			img_r = false;
			if (reset_n == false) {
				tile_nb = 0;
				img_r = false;
				/*DEB*/ std::cout <<"VCALC get_cache reset ok" << std::endl;
			}
			else {
				//Si on commence une image, il faut
				//que le processeur indique l'adresse
				//de l'image à lire
				if (tile_nb == 0) {
					/*DEB*/ std::cout <<"VCALC get_cache va attendre adresse" << std::endl;
					while (wb_tab[5] != 1) {
						wait();
					}
					img_addr = wb_tab[4];
					wb_tab[5] = 0;
					tile_nb = 1;
					/*DEB*/ std::cout <<"VCALC get_cache img addr " << img_addr << std::endl;
				}
				//On possède une adresse d'image à lire correcte
				else {
					//On attend que process_tile ait fait une demande
					//de remplissage de cache. On sait alors que cache_center_c
					//et cache_center_l sont positionnés correctement
					/*DEB*/ std::cout <<"VCALC get_cache attend ask_cache tuile" <<tile_nb << std::endl;
					while (!ask_cache.read()) wait();
					//Remplissage du cache
					/*DEB*/ std::cout <<"VCALC get_cache va remplir cache" << std::endl;
					cache_fill(img_addr,tile_nb);
					//On indique que le cache a été rempli
					cache_rdy = true;
					/*DEB*/ std::cout <<"VCALC get_cache a rempli cache" << std::endl;
					//On se prépare à traiter la tuile suivante
					tile_nb = tile_nb%(p_WIDTH*p_HEIGHT/(T_W*T_H));
					//Fin d'une image
					if (tile_nb == 0) img_r = true;
					/*DEB*/ std::cout <<"VCALC get_cache fin cache " << tile_nb<< std::endl;
				}
				wait();
			}

		}
	}

	////////////////////////////
	// Process_tile
	////////////////////////////

	/* Calcule l'antécédent de la tuile courante, une fois que l'antécédent du centre
	 * de la tuile a été calculé, demande le remplissage du cache.
	 * Une fois que le cache a été obtenu (pixel_ready à 1), 
	 * remplit la fifo avec les pixels de la tuile résultat.
	 */
	//TODO CHANGE THIS:  Pour l'instant on se contente de ressortir l'image telle quelle

	tmpl(void)::process_tile() {
		//Numéro de la tuile en cours de traitement
		//Vaut 0 entre deux images
		int tile_nb = 0;
		//Tableau des antécédents des pixels
		//de la tuile traitée
		int invimg_c[T_H][T_W];
		int invimg_l[T_H][T_W];
		
		/*DEB*/ std::cout <<"VCALC process_tile Ici" << std::endl;
		if (reset_n == false) {
			tile_nb = 0;
			/*DEB*/ std::cout <<"VCALC process_tile reset OK" << std::endl;
		}
		else {
			tile_nb++;
			//Calcul du centre
			/*DEB*/ std::cout <<"VCALC process_tile calcul du centre" << std::endl;
			process_center(tile_nb);
			/*DEB*/ std::cout <<"VCALC process_tile calcul du centre fini, va demander cache" << std::endl;
			//Demande de remplissage du cache
			ask_cache = true;
			//Calcul des antécédents
			process_invimg(tile_nb, invimg_c, invimg_l);
			/*DEB*/ std::cout <<"VCALC process_tile process_invimg fini va attendre le cache" << std::endl;
			//On attend que le cache soit rempli
			while (cache_rdy == false) wait();
			ask_cache = false;
			/*DEB*/ std::cout <<"VCALC process_tile le cache est là" << std::endl;
			//On parcourt le tableau des coordonnées
			//des antécédents. On met dans la fifo
			//les pixels du cache correspondants
			for (int l = 0; l<T_H; l++) {
				for (int c = 0; c<T_W; c++) 
				{
					//Si le pixel n'est pas dans
					//le cache, on met un pixel noir
					if (invimg_c[l][c] < (cache_center_c - C_W/2) || invimg_c[l][c] > (cache_center_c + C_W/2) || invimg_l[l][c] < (cache_center_l - C_H/2) || invimg_l[l][c] > (cache_center_l + C_H/2))
						fifo.write(255);
					//Sinon on écrit le pixel du cache correspondant
					else fifo.write(cache[invimg_l[l][c]%C_H][invimg_c[l][c]%C_W]); //XXX TODO CHANGE THIS, C EST PAS CA

				}
			}
			
			/*DEB*/ std::cout <<"VCALC process_tile cache depose dans fifo" << std::endl;

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
					while (wb_tab[7] != 1) wait();
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
					for (int l = 0; l < T_H; l++) {
						//On récupère la ligne de tuile
						for (int c = 0; c < T_W/4; c++) {
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
	tmpl(void)::process_center(int tile_nb) {
		//Nombre de tuiles par ligne
		int tile_line = p_WIDTH/C_W;
		cache_center_c = (tile_nb -1)%tile_line*C_W + C_W/2;
		cache_center_l = (tile_nb -1)/tile_line*C_H + C_H/2;
	}

	//TODO CHANGE THIS : pour l'instant on ne fait
	//aucun calcul sur l'image. Le centre du cache est 
	//le centre de la tuile
	tmpl(void)::process_invimg(int tile_nb, int invimg_c[T_H][T_W], int invimg_l[T_H][T_W]) {
		/*DEB*/ std::cout <<"VCALC process_invimg ICI" << std::endl;
		int tile_line = p_WIDTH/C_W;
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


	tmpl(void)::cache_fill(uint32_t img_addr, int tile_nb) {
		//Coordonnées du point en haut à gauche
		//de la zone de cache à remplir
		//Peut se trouver en dehors de l'image réèlle
		int cache_l = 0;
		int cache_c = 0;
		//Coordonnées du point en haut à gauche du cache
		//qui est vraiment dans l'image
		uint32_t cache_im_l = 0;
		uint32_t cache_im_c = 0;
		//Taille de la zone de cache qui est dans l'image
		uint32_t cache_im_h;
		uint32_t cache_im_w;
		//Ligne de cache en cours de traitement
		uint32_t cache_line = 0;

		//Calcul des coordonnées du pixels en haut à gauche de la zone
		//à traiter
		cache_c = cache_center_c - C_W/2;
		cache_l = cache_center_l - C_H/2;
		//On assure les accès mémoires alignés:
		cache_c -= cache_c%4;
		cache_l -= cache_l%4;

		//ATTENTION : il est possible que la zone de cache n'est
		//pas forcément dans l'image

		//On remplit la zone cache de 0
		for (int i =0; i<C_H; i++) {
			for (int j = 0; j<C_W; j++) {
				cache[i][j] = 0;
			}
		}
		//Calcul de la zone de cache qu'il faut remplir avec 
		//une recherche en mémoire

		//Cas où la zone de cache est en-dessous où à droite de l'image.
		//La zone de cache doit être laissée noire
		if (cache_c > (uint32_t)(p_WIDTH-1) || cache_l > (uint32_t)(p_HEIGHT-1)) return;

		//Sinon on calcule l'intersection entre la zone de cache
		//et l'image
		cache_im_w = C_W;
		//Dépassement à gauche
		if (cache_c < 0) {
			cache_im_c = 0;
			cache_im_w += cache_c;
		}
		//Dépassement à droite
		if (cache_c + C_W > (uint32_t)(p_WIDTH - 1)) cache_im_w -= (cache_c +C_W - p_WIDTH);

		cache_im_h = C_H;
		//Dépassement en haut
		if (cache_l < 0) {
			cache_im_l = 0;
			cache_im_l +=cache_l;
		}
		if (cache_l + C_H > p_HEIGHT - 1) cache_im_h -= (cache_l +C_H - p_HEIGHT);

		//Lectures blocs de chaque ligne de cache
		for (cache_line = 0; cache_line < C_H; cache_line ++) {
			master0.wb_read_blk(img_addr + (tile_nb/(p_WIDTH/T_W))*T_W*T_H + (tile_nb%(p_WIDTH*T_W))*T_W + cache_im_c , 
					cache_im_w, (uint32_t *)&cache[cache_im_l+cache_line][cache_im_c%C_W]); 
		}
		//On s'assure de faire des accès alignés
		cache_l += cache_l%4;
		cache_c += cache_c%4;

		for (cache_line = 0; cache_line < cache_im_h; cache_line++)  
			master0.wb_read_blk(img_addr + ((cache_im_l+cache_line)*p_WIDTH) + cache_im_c, cache_im_w/4, (uint32_t *)&cache[(cache_im_l+cache_line)%C_H][cache_im_c%C_W]); 


	}
}}

