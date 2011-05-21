#ifndef VIDEO_CALC_H
#define VIDEO_CALC_H

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"
#define NBPACK 32
#define T_W  16, //largeur de la tuile traitée
#define T_H  16,  //hauteur de la tuile traitée
#define C_W  16, //largeur de la zone en cache,
#define C_H  16, //hauteur de la zone en cache

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

			/////////////////////////////////////
			// Constructeur
			/////////////////////////////////////

			VideoCalc (sc_module_name insname,
					uint32_t * tab,
					);

			//////////////////////////////////////
			// Methodes et parametres internes
			/////////////////////////////////////
			void read_tule();
			void store_tule();

		private:

			//Pour contenir la zone en cache
			unsigned char cache[T_H][T_W];
			
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

			


