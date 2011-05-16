/*
 * =============================================================================
 *
 *       Filename:  master.h
 *
 *    Description:  generic wb master
 *                  This module demonstrate WbMasterModule usage
 *
 *         Author:  Tarik Graba (TG), tarik.graba@telecom-paristech.fr
 *        Company:  Telecom Paris TECH
 *
 * =============================================================================
 */

#ifndef __MASTER__
#define __MASTER__


#include <systemc>

#include "wb_master_module.h"

namespace soclib { namespace caba {

    template<typename wb_param>
        class Master: sc_core::sc_module
    {
        public:
            sc_core::sc_in<bool> p_clk;
            sc_core::sc_in<bool> p_resetn;
            soclib::caba::WbMaster<wb_param> p_wb;

        private:
            // main thread
            void process();
            
            void     wait_cycles (uint32_t delay);
            WbMasterModule<wb_param> master0;

        protected:
            SC_HAS_PROCESS(Master);

        public:
            // constructor
            Master ( sc_module_name name );
    };
}}

#endif //__MASTER__
