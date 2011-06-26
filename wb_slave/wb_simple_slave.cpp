#include "wb_simple_slave.h"
#include <iostream>

namespace soclib { namespace caba {
	// Constructor
	template <typename wb_param> \
		WbSimpleSlave<wb_param>::WbSimpleSlave(sc_core::sc_module_name insname, uint32_t slave_addr, uint32_t size) : 
			sc_core::sc_module(insname), 
			slave_addr(slave_addr),
			size(size)
	{
		data_tab = (uint32_t *)malloc(size*sizeof(uint32_t));
		if (data_tab == NULL) return;

		std::cout << "WishBone simple slave "  << name() 
			<< " created sucsesfully" << std::endl;

		SC_METHOD(transition);
		sensitive << p_clk.pos();

		SC_METHOD(genMealy);
		sensitive << p_clk.neg();
		sensitive << p_wb;

	}


	// Synchronoue methode
	template <typename wb_param> \
		void WbSimpleSlave<wb_param>::transition() {
			//On force wb_data_1 à 0
			data_tab[1] = 0;
			data_tab[3] = 0;

			if (p_resetn == false) {
				// reset cycle couter
				cycle = 0;
#if 0
				std::cout << name() << " "
					<< "RESET:: waiting for requests"
					<< std::endl;
#endif
				return;
			}
			cycle++;
			if (p_wb.STB_I ) {
				if (p_wb.CYC_I ) {
				#if 0
					std::cout << name() << " "
						<< "Recived a valid strobe" << std::endl
						<< p_wb
						<< " at cycle "<< std::dec << cycle
						<< std::endl;
					#endif
					if (p_wb.WE_I && (p_wb.ADR_I.read() >= slave_addr) && (p_wb.ADR_I.read() < (slave_addr + 4*size))) {
						data_tab[(p_wb.ADR_I.read()- slave_addr)/4] = p_wb.DAT_I.read();
					}
					else std::cout << "addr non attrib"<< p_wb.ADR_I.read() << std::endl;
				}
				else  {
					std::cout << name() << " "
						<< "Recived an invalid strobe" << std::endl
						<< p_wb
						<< " at cycle "<< std::dec << cycle
						<< std::endl;
				}
			}
			//Signaux = contenu du tableau
			wb_data_0 = data_tab[0];
			wb_data_1 = data_tab[1];
			wb_data_2 = data_tab[2];
			wb_data_3 = data_tab[3];
		}

	// Synchronoue methode
	template <typename wb_param> \
		void WbSimpleSlave<wb_param>::genMealy() {
			// always acknowledge requests
			p_wb.ACK_O = p_wb.STB_I && p_wb.STB_I;
			// read write request counter
			if (p_wb.ADR_I.read() >= slave_addr && p_wb.ADR_I.read() < (slave_addr + 4*size) ) 
				p_wb.DAT_O = data_tab[p_wb.ADR_I.read() - slave_addr] ;
			else
				p_wb.DAT_O = 42;
		}
}}
