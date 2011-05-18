#ifndef WB_SIMPLE_SLAVE_H_
#define WB_SIMPLE_SLAVE_H_

#include <systemc>
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

				uint32_t * data_tab;

                // constructor
                WbSimpleSlave (sc_core::sc_module_name insname, uint32_t slave_addr, uint32_t size);

                void transition();
                void genMealy();
                //void genMoore();
            private:
                uint32_t cycle;
				uint32_t slave_addr;
				uint32_t size;
        };

}}
#endif
