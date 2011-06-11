/**********************************************************************
 *
 *  Module de calcul de transformation géométrique
 *  Pour l'instant :
 *  1 TO DO (Va chercher les coeffs en RAM et les mets dans un buffer.)
 *  2 Va chercher des tuiles de B_W * T_H en RAM et les mets dans un buffer.
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
	 SC_THREAD(get_tile);
	 sensitive << clk.pos();

	 // Module de calcul incrémental
	 // * Effectue le calcul incrémental
	 // * Pose les tuiles calculées dans la fifo
	 SC_THREAD(store_tile);
	 sensitive << clk.pos();

	 buffer_img_in = (uint32_t *)malloc((sizeof(uint32_t)*4));
	 std::cout <<  name()
		<< " was created successfully " << std::endl;
  }

  ///////////////////////////
  // Get_tile
  //////////////////////////

  tmpl(void)::get_tile()
  {
	 uint32_t addr;
	 while(1)
	 {
		/****************
		 * RESET
		 * *************/
		if (reset_n == false)
		{
		  std::cout << " VCALC STORE_PIXEL: RESET " << std::endl;
		  wait();
		}

		/**********
		 * On attend une nouvelle addresse ou l'on va stocker l'image
		 **********/
		while(!wb_tab[5])
		{
		  //std::cout << "j'attends une autre image" << std::endl;
		  wait();
		}

		deb_im_in = wb_tab[4];
		wb_tab[5] = 0;
		std::cout << " VCALC GET_TILE: NOUVELLE ADRESSE: " << deb_im_in << std::endl;

		//on parcourt tuile par tuile
		for(unsigned int i = 0; i < T_NB; i++)
		{
		  //Pour chaque tuile, on parcourt ligne par ligne
		  for(unsigned int j = 0; j < T_H; j++)
		  {
			 addr = deb_im_in + j*p_WIDTH + (i % (p_WIDTH/T_W))*T_W + (i/(p_WIDTH/T_W))*T_H*p_WIDTH;
			 //std::cout << addr << std::endl;
			 master0.wb_read_blk(addr, 4, buffer_img_in);
			 for (int l = 0; l < 4; l++)
			 {
				for (int k = 3; k>=0; k--)
				{
				  fifo.write(buffer_img_in[l] >> 8 * k);
				  buffer_img_in[l] = buffer_img_in[l] - ((buffer_img_in[l] >> 8 * k) << 8 * k);
				}
			 }
		  }
		}

		std::cout << "VCALC a finit de lire une image" << std::endl;
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
	 uint32_t to_store[4];
	 uint8_t mask[4];
	 bool stock_ok = false;
	 int nb_line_stocked = 0;
	 int nb_tile = 0;
	 uint32_t addr;

	 for (int i = 0; i < 4; i++)
		mask[i] = 0xf;

	 while(1)
	 {
		/**********
		 * RESET
		 *********/
		if (reset_n == false)
		{
		  nb_line_stocked = 0;
		  p_interrupt = 0;
		  std::cout << " VCALC STORE_PIXEL: RESET " << std::endl;
		  wait();
		}

		/**********
		 * On attend une nouvelle addresse ou l'on va stocker l'image
		 **********/
		while(!wb_tab[7] && !stock_ok)
		{
		  //std::cout << "j'attends une addresse ou l'on va stocker une image" << std::endl;
		  wait();
		}

		if(!stock_ok)
		{
		  stock_ok = true;
		  deb_im_out = wb_tab[6];
		  wb_tab[7] = 0;
		  std::cout << " VCALC STORE_PIXEL: NOUVELLE ADRESSE: " << deb_im_out << std::endl;
		}

		if ((unsigned int) fifo.num_available() > 16)
		{
		  for (unsigned int i = 0; i< 4; i++)
		  {
			 to_store[i] = 0;
			 for (unsigned int j = 0; j < 4; j++)
			 {
				to_store[i] = to_store[i] << 8;
				to_store[i] += fifo.read();
			 }
		  }
		  addr = deb_im_out + (nb_line_stocked%T_H)*p_WIDTH + (nb_tile%(p_WIDTH/T_W))*T_W + (nb_tile/(p_WIDTH/T_W))*T_H*p_WIDTH;
		  master1.wb_write_blk(addr, mask, to_store, 4);
		}
		else
		  std::cout << "VCALC bloque sur FIFO" << std::endl;
		nb_line_stocked++;
		nb_tile += ((nb_line_stocked % 16) ? 0 : 1);

		/**********************
		 * INTERRUPT GENERATION
		 *********************/
		if((unsigned int)nb_line_stocked == (T_NB*T_H))
		{
		  nb_line_stocked = 0;
		  nb_tile = 0;
		  p_interrupt = 1;
		  wait();
		  wait();
		  wait();
		  p_interrupt = 0;
		  stock_ok = false;
		  std::cout << " VCALC STORE_PIXEL: INTERRUPTION SENT " << std::endl;
		}

	 }
  }
}}

