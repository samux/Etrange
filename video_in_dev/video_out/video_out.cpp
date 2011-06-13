/************************************************************
 *
 *      File : video_gen.cpp
 *      Author: T. Graba
 *      Credits: A. Polti
 *      Telecom ParisTECH
 *
 ************************************************************/

#include "video_out.h"
#define tmpl(x) template<typename wb_param> x VideoOut<wb_param>
namespace soclib { namespace caba {

	// le constructeur
	tmpl(/**/)::VideoOut (sc_core::sc_module_name insname,
			uint32_t * wb_tab,
			const int w ,
			const int h ,
			const int lsync ,
			const int fsync ):
		sc_core::sc_module(insname),
		p_WIDTH(w),
		p_HEIGHT(h),
		p_LINE_SYNC(lsync),
		p_FRAME_SYNC(fsync),
		wb_tab(wb_tab),
		p_clk("p_clk"),
		p_resetn("p_resetn"),
		master0(p_clk,p_resetn, p_wb),
		fifo(10000)
	{
		// Lecture des pixels en RAM
		SC_THREAD(read_image);
		sensitive << p_clk.pos();
		dont_initialize();

		// Génération de la sortie vidéo
		SC_THREAD(gen_sorties);
		sensitive << pixel_clk.pos();
		dont_initialize();

		std::cout << name()
			<< " was created succesfully " << std::endl;
	}

	// Génère les sorties en lisant en continu
	// dans la fifo
	tmpl(void)::gen_sorties()
	{
		std::cout << " VOUT GEN_SORTIES: START " << std::endl;

		unsigned char pixel_tmp;

		for(;;)
		{

			if(p_resetn == false)
			{
reset:
				// On met les sorties à zero
				pixel_out = 0;
				line_valid = false;
				frame_valid = false;
				p_interrupt = 0;
				std::cout << " VOUT GEN_SORTIES: RESET " << std::endl;
				wait();
			}

			else
			{
				while (fifo.num_available() == 0)
					wait();

				for(unsigned int i=0; i<( p_HEIGHT + p_FRAME_SYNC ); i++)
					for(unsigned int j=0; j<( p_WIDTH + p_LINE_SYNC ); j++)
					{
						// Si on est dans la fenêtre active, on sort le pixel courant
						// Rappel : une trame video fait ( p_WIDTH + p_LINE_SYNC )*( p_HEIGHT + p_FRAME_SYNC ),
						// l'image active est de p_WIDTH*p_HEIGHT
						if((i<p_HEIGHT) && (j>p_LINE_SYNC-1))
						{
							if (!fifo.nb_read(pixel_tmp))
							{
								std::cout<< "VOUT GEN_SORTIES: FIFO VIDE" <<std::endl;
							}
							else
								pixel_out = pixel_tmp;
						}
						else
							pixel_out = 0xBB;

						// Generation de line valid
						line_valid = (i<p_HEIGHT) && (j>p_LINE_SYNC-1);

						// Generation de frame valid
						frame_valid = (i<p_HEIGHT);

						// Puis on attend le prochain coup d'horloge, ou un reset
						wait();

						// Si on est reveillé par un reset, on revient à la case départ
						// (les goto sont dans ce genre de cas bien pratique...)
						if(p_resetn == false)
							goto reset;
					}

				p_interrupt = 1;
                                wb_tab[3] = 0;
                                std::cout << " VOUT GEN_SORTIES: INTERRUPTION SENT " << std::endl;
				wait();
				wait();
				wait();
				p_interrupt = 0;
			}
		}
	}

	// Lecture de l'image en RAM à partir de l'adresse
	// spécifiée par le processeur par whisbone (contenu
	// dans wb_tab) puis écriture dans la fifo
	tmpl(void)::VideoOut::read_image()
	{
		//Les pixels de type uint8_t sont groupés
		//par 4 dans la mémoire.
		uint32_t im_addr;
		uint32_t buffer[VOUT_PACK];

		std::cout << " VOUT READ_IMAGE: START " << std::endl;

		for (;;) {

			if (p_resetn == false)
			{
                          im_addr = 0;
                          wb_tab[3] = 0;
                          wb_tab[2] = 0;
                          while (fifo.read());
			}

			//Nouvelle image, on met à jour l'adresse de l'image
			//en ram
			while (!wb_tab[3])
                          wait();

			im_addr = wb_tab[2];
			wb_tab[3] = 0;

			std::cout << " VOUT READ_IMAGE: NOUVELLE ADRESSE " << wb_tab[2] << std::endl;

			for (unsigned int i = 0; i < (p_HEIGHT * p_WIDTH) / (VOUT_PACK * 4); i++)
			{
				//std::cout << " VOUT READ_IMAGE: LECTURE DE" << VOUT_PACK << "mots en " << im_addr+i*VOUT_PACK << std::endl;

				master0.wb_read_blk(im_addr + 4 * i * VOUT_PACK,
						VOUT_PACK,
						buffer);

				for (int j = 0; j < VOUT_PACK; j++)
				{
					for (int k = 3; k>=0; k--)
					{
						fifo.write(buffer[j] >> 8 * k);
						buffer[j] = buffer[j] - ((buffer[j] >> 8 * k) << 8 * k);
					}
				}

			}
                        wb_tab[3] = 0;
		}
	}

}}

