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
		const int w , const int h ,
		const int lsync , const int fsync ):
	 sc_core::sc_module(insname),
	 p_WIDTH(w), p_HEIGHT(h), p_LINE_SYNC(lsync), 
	 p_FRAME_SYNC(fsync), fifo(64), wb_tab(wb_tab),
	 p_clk("p_clk"), p_resetn("p_resetn"),
	 master0(p_clk,p_resetn, p_wb)
  {
	 SC_THREAD(gen_sorties);
	 sensitive << clk_out.pos();
	 dont_initialize();

	 SC_THREAD(read_image);
	 sensitive << clk.pos();
	 dont_initialize();

	 std::cout << "Video_out"  << name()
		<< " was created succesfully " << std::endl;
  }

  // Génère les sorties en lisant en continu
  // dans la fifo
  tmpl(void)::gen_sorties()
  {
	 unsigned char pixel_tmp;
	 while(1)
	 {
		if(reset_n == false)
		{
reset:

#ifdef SOCLIB_MODULE_DEBUG
		  cout << name() << " Reset ..." << endl;
#endif
		  // on met les sorties à zero
		  pixel_out = 0;
		  line_valid = false;
		  frame_valid = false;
		  p_interrupt = 0;

		  // Puis on attend le prochain coup d'horloge, ou un reset
		  wait();
		}
		else
		{
		  while (fifo.num_available() == 0) wait();
		  unsigned int i,j;
		  for(i=0; i<( p_HEIGHT + p_FRAME_SYNC ); i++)
			 for(j=0; j<( p_WIDTH + p_LINE_SYNC ); j++)
			 {
				// Si on est dans la fenêtre active, on sort le pixel courant
				// Rappel : une trame video fait ( p_WIDTH + p_LINE_SYNC )*( p_HEIGHT + p_FRAME_SYNC ),
				// l'image active est de p_WIDTH*p_HEIGHT
				if((i<p_HEIGHT) && (j>p_LINE_SYNC-1)) {
				  p_interrupt = 0;
				  if (!fifo.nb_read(pixel_tmp)) {
					 //	std::cout<< "Video_out: Rien a lire dans la fifo" <<std::endl;
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
				if(reset_n == false)
				  goto reset;
			 }
		  p_interrupt = 1;
#ifdef SOCLIB_MODULE_DEBUG
		  cout << name() << " Fin image " << endl;
#endif
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
	 wb_tab[2] = RAM_BASE;
	 wb_tab[3] = 0;
	 im_addr = wb_tab[2];


	 for (;;) {
		if (reset_n == false) {
		  im_addr = wb_tab[2];
		  wb_tab[3] = 0;
		  while (fifo.read());
		}
		//Nouvelle image, on met à jour l'adresse de l'image
		//en ram
		while (wb_tab[3] == 0) wait();
		im_addr = wb_tab[2];
		wb_tab[3] = 0;
		//std::cout << "VOUT Lit une nouvelle image" << std::endl;

		for (int i = 0; i < (p_HEIGHT*p_WIDTH)/(VOUT_PACK*4); i++) {
		  //std::cout << "Video_out va lire" << VOUT_PACK << "mots en " << im_addr+i*VOUT_PACK << std::endl;
		  master0.wb_read_blk(im_addr+ 4*i*VOUT_PACK, VOUT_PACK, buffer);
		  for (int j = 0; j < VOUT_PACK; j++) {
			 for (int k = 3; k>=0; k--) {
				fifo.write(buffer[j] >> 8*k);
				buffer[j] = buffer[j] - ((buffer[j] >> 8*k) << 8*k);
			 }
		  }

		}
	 }
  }

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:
//1073741824