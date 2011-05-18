#ifndef VIDEO_IN_H

#define VIDEO_IN_H

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"
#define p_NB_PACK 8
#define VIN_offset 0 //Adresse du stockage en RAM

using namespace sc_core;
using namespace std;

namespace soclib { namespace caba {

    template<typename wb_param>
	class Video_in: sc_module
	{
		public:
			// IO PORTS

			sc_in_clk           clk;
			sc_in<bool>         reset_n;

			sc_in <bool>        line_valid;
			sc_in <bool>        frame_valid;

			sc_in<unsigned char> pixel_in;

			//Wishbone 
			sc_core::sc_in<bool> p_clk;
			sc_core::sc_in<bool> p_resetn;
			soclib::caba::WbMaster<wb_param> p_wb;


			////////////////////////////////////////////////////
			//	constructor
			////////////////////////////////////////////////////

			Video_in (sc_module_name insname, 
					uint32_t * tab,
					const int w = 640,          // largeur d'image par defaut
					const int h = 480,          // hauteur par defaut
					const int lsync = 160,      // synchro ligne par defaut
					const int fsync = 40        // synchro trame par defaut
				 );
			////////////////////////////////////////////////////
			//	destructor
			////////////////////////////////////////////////////

			//~Video_in ();

			////////////////////////////////////////////////////
			//	methods and structural parameters
			////////////////////////////////////////////////////
			void read_pixels();
			void store_pixels();


		private:

			// paramètres de l'image
			const uint32_t  p_WIDTH      ;
			const uint32_t  p_HEIGHT     ;
			const uint32_t  p_LINE_SYNC  ;
			const uint32_t  p_FRAME_SYNC ;

			unsigned int pixel_c; 
			unsigned int pixel_l; 

			//Pour stocker les pixels entrants
			sc_fifo<unsigned char> fifo;
			unsigned char image[480][640];

			//Tableau de Wb_slave
			uint32_t * wb_tab;
			

            		WbMasterModule<wb_param> master0;

		protected:
			SC_HAS_PROCESS(Video_in);

	};

}}
#endif //VIDEO_IN_H

