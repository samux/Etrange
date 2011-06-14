#ifndef WB_SIMPLE_SLAVE_H_
#define WB_SIMPLE_SLAVE_H_

#include <systemc.h>
#include"wb_slave.h"

namespace soclib { namespace caba {

    template <typename wb_param>

        class WbSimpleSlave: public sc_core::sc_module{
            protected:
                SC_HAS_PROCESS(WbSimpleSlave);
            public:
                sc_core::sc_in<bool>               p_clk;
                sc_core::sc_in<bool>               p_resetn;
                WbSlave <wb_param>    p_wb;

				//Les données reçues du processeur
				//Pour les modules SystemC
				uint32_t * data_tab;	

				//Pour les modules Verilog (pour l'instant
				//uniquement Video_in
				sc_core::sc_out<sc_uint<32> > wb_data_0;
				sc_core::sc_out<sc_uint<32> > wb_data_1;
				

                // constructor
                WbSimpleSlave (sc_core::sc_module_name insname, uint32_t slave_addr, uint32_t size);

                void transition();
                void genMealy();
				void tab_to_sign();
                //void genMoore();
            private:
                uint32_t cycle;
				uint32_t slave_addr;
				uint32_t size;
        };

}}
#endif
