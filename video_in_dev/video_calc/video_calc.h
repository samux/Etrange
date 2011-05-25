#ifndef VIDEO_CALC_H
#define VIDEO_CALC_H

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"
#define NBPACK 32
#define p_WIDTH 640
#define p_HEIGHT 480
#define T_W  16 //largeur de la tuile traitée
#define T_H  16  //hauteur de la tuile traitée
#define C_W  64 //largeur de la zone en cache,
#define C_H  64 //hauteur de la zone en cache
#define F_SIZE 4 //Taille de la fifo en nombre de tuile
#define tmpl(x) template<typename wb_param> x VideoCalc<wb_param>

using namespace sc_core;
using namespace std;

//Pour l'instant on se contente de lire puis de stocker les
//tules en ram sans effectuer aucune operation.
//La methode process_tule ne fait rien

namespace soclib { namespace caba {
	template<typename wb_param>
		class VideoCalc:sc_module
	{
		public:
			// Ports E/S
			
			sc_in_clk	clk;
			sc_in<bool> reset_n;

			//Wishbone
			sc_core::sc_in<bool> p_clk;
			sc_core::sc_in<bool> p_resetn;
			soclib::caba::WbMaster<wb_param> p_wb;

			//Interruptions de fin de lecture
			//et de fin d'écriture de l'image
			sc_out<bool> img_r;
			sc_out<bool> img_w;

			/////////////////////////////////////
			// Constructeur
			/////////////////////////////////////

			VideoCalc (sc_core::sc_module_name insname,
					uint32_t * tab,
					);

			//////////////////////////////////////
			// Methodes et parametres internes
			/////////////////////////////////////
			void get_cache();
			void process_tile();
			void store_tile();
			void process_center(int tile_nb);
			void process_invimg(int tile_nb, int invimg_c[T_H][T_W], int invimg_l[T_H][T_W]);
			
			void cache_fill(uint32_t img_addr,int tile_nb);


		private:

			//Pour contenir la zone en cache
			unsigned char cache[C_H][C_W];

			//Coordonnées du pixel au centre de la zone de cache
			int cache_center_c;
			int cache_center_l;

			//Pour indiquer qu'il est temps de remplir
			//le cache
			sc_signal<bool> ask_cache;

			//Pour indique que le cache a été rempli
			sc_signal<bool> cache_rdy;

			//Fifo de tuile à stocker en RAM
			sc_fifo<unsigned char> fifo;

			//Adresse de lecture et ecriture en RAM
			uint32_t * wb_tab;

			//Maître wishbone pour l'écriture et la lecture en RAM
			WbMasterModule<wb_param> master0;

		protected:
			SC_HAS_PROCESS(VideoCalc);

	};
}}
#endif //VIDEO_CALC_H

			


