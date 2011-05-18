/************************************************************
 *
 *      Acquisition du flux video entrant, stockage dans une
 *      fifo puis dans un tableau.
 *
 ***********************************************************/

#include "video_in.h"

#define tmpl(x) template<typename wb_param> x Video_in<wb_param>
namespace soclib { namespace caba {

	// le constructeur 
	tmpl(/**/)::Video_in (sc_core::sc_module_name insname, 
			uint32_t * tab,
			const int w , const int h ,
			const int lsync , const int fsync):
		sc_core::sc_module(insname), 
		p_WIDTH(w), p_HEIGHT(h), 
		p_LINE_SYNC(lsync), p_FRAME_SYNC(fsync),
		wb_tab(tab),
		p_clk("p_clk"), p_resetn("p_resetn"),
		master0(p_clk,p_wb)
	{

		//Lecture des pixels entrants
		//Et stockage dans la fifo
		SC_THREAD(read_pixels);
		sensitive << clk.pos();
		dont_initialize();


		//Récupération des pixels de la fifo
		//et mise en RAM
		SC_THREAD(store_pixels);
		sensitive << clk.pos();

		// debut de l'image
		pixel_c = 0;
		pixel_l = 0;

		std::cout << "Video_in "  << name()
			<< " was created successfully " << std::endl;
	}


	// Lit les pixels reçus et les place dans la fifo
	tmpl(void)::read_pixels()
	{
		cout << "read_pixels" << endl;
		while(true)
		{
			if(reset_n == false)
			{
reset:
				pixel_c = 0;
				pixel_l = 0;
				wait();
			}
			else
			{
				// image valide
				if (line_valid && frame_valid)
				{
					//	cout << name() << " valid ..." <<  " lines : " << pixel_l << "col :" << pixel_c << endl;
					if (pixel_c < p_WIDTH && pixel_l < p_HEIGHT)
					{
						//TO DO On stocke le pixel dans la fifo
						fifo.write(pixel_in.read());
						pixel_c++;
					}
					else
					{
						cout << name() << " WARNING: Too much pixels..!!!!!" 
							<< " lines : " << pixel_l << " col : " << pixel_c << endl;
						exit(-1);
					}
				}
				else {
					// synchro horizontale
					if (frame_valid && !line_valid)
					{
						if (pixel_c == p_WIDTH)
						{
							pixel_c = 0;
							pixel_l++;
						}
						else if (pixel_c != 0)
						{
							cout << name() << " Warning.........!!" 
								<< " lines : " << pixel_l << "col :" << pixel_c << endl;
						}
					}
					// synchro verticale
					else if (!frame_valid)
					{
						if((pixel_c == p_WIDTH) && (pixel_l == p_HEIGHT -1))
						{
							pixel_c = 0;
							pixel_l = 0;
						}
					}
				}
				wait();

				if(reset_n == false)
					goto reset;
			}
		}
	}

	//Ce Thread surveille la fifo. Dès que celle-ci contient au moins p_NB_PACK pixels, il les lit et les mets dans un tableau
	//TO DO remplacer le tableau par une écriture en RAM via wishbone
	tmpl(void)::store_pixels() {

		uint32_t pixel_stored_l;
		uint32_t pixel_stored_c;
		//Adresse de début de stockage de l'image
		uint32_t deb_im = RAM_BASE;
		uint32_t to_store;

		cout << "store_pixels" << endl;

		while (true) {
			if (reset_n == false) {
				pixel_stored_c = 0;
				pixel_stored_l = 0;
				deb_im = wb_tab[0];
				if (deb_im< RAM_BASE)
					deb_im = RAM_BASE;
				cout << "deb_im" << endl;
				wait();
			}
			else {
				//Tant que la fifo ne contient pas assez de pixels,
				//on attend
				if ((unsigned int)fifo.num_available() < (p_NB_PACK)) wait();
				else {
					//On stocke p_NB_PACK pixels dans le tableau
					for (unsigned int i = 0; i< p_NB_PACK/4; i++) {
						to_store = 0;
						for (unsigned int j = 0; j < 4; j++) {
							to_store = to_store << 8;
							to_store += fifo.read();
						}
						master0.wb_write_at(deb_im+(p_WIDTH*pixel_stored_l +pixel_stored_c),0xf,to_store); 
						cout << "Video_in : Stockage de " << to_store <<" en " << p_WIDTH*pixel_stored_l + pixel_stored_c << endl;
						pixel_stored_c = (pixel_stored_c + 4) % p_WIDTH;
						if (pixel_stored_c == 0) {
							pixel_stored_l = (pixel_stored_l + 1) % p_HEIGHT;
							if (pixel_stored_l == 0) {
								cout << "Stockage d'une image terminée" << endl;
								deb_im = wb_tab[0];
							}
						}
					}
				}
			}
			wait();
		}

	}


}
}
