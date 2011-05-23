/*
 * =============================================================================
 *
 *       Filename:  wb_master_module.cpp
 *
 *    Description:  generic wb module
 *
 *         Author:  Tarik Graba (TG), tarik.graba@telecom-paristech.fr
 *        Company:  Telecom Paris TECH
 *
 * =============================================================================
 */

#include <systemc>
#define DEBUG_WB 0
#define DEBUG_WB2 0

#include "wb_master_module.h"

namespace soclib { namespace caba {

    ////////////////////////////////////////////////////////////////////////////
    // single read/write operation
    ////////////////////////////////////////////////////////////////////////////

    template<typename wb_param>
        uint32_t WbMasterModule<wb_param>::wb_read_at
        ( uint32_t addr )
        {
            uint32_t tmp;
            sc_core::wait(p_clk.negedge_event());
            p_wb.ADR_O = addr;
            p_wb.SEL_O = 0xF;
            p_wb.STB_O = true;
            p_wb.CYC_O = true;
            p_wb.WE_O  = false;

            // sc_core::wait for ack
            WaitWbAck();
            tmp = (uint32_t) p_wb.DAT_I.read();
            // clean request after clk falling edge
            sc_core::wait(p_clk.negedge_event());
            CleanWb();
            num_reads++;
            return tmp;
        }

    template<typename wb_param>
        void     WbMasterModule<wb_param>::wb_write_at
        (uint32_t addr, uint8_t mask, uint32_t data)
        {
            // set request on clk falling edge
            sc_core::wait(p_clk.negedge_event());
            p_wb.DAT_O = data;
            p_wb.ADR_O = addr;
            p_wb.SEL_O = mask;
            p_wb.STB_O = true;
            p_wb.CYC_O = true;
            p_wb.WE_O  = true;

            // sc_core::wait for ack
            WaitWbAck();
            // clean request after clk falling edge
            sc_core::wait(p_clk.negedge_event());
            CleanWb();
            num_writes++;
        }

    ////////////////////////////////////////////////////////////////////////////
    // burst read/write operation to/from an uint32_t array
    ////////////////////////////////////////////////////////////////////////////

    template<typename wb_param>
        void WbMasterModule<wb_param>::wb_read_blk
        ( uint32_t saddr, uint32_t num, uint32_t *dest)
        {
#if DEBUG_WB
			std::cout << "WBMaster lit "<< num << "mots en " << saddr << std::endl; 
#endif
            for (uint32_t i = 0; i< num; i++)
            {
                sc_core::wait(p_clk.negedge_event());
                p_wb.ADR_O = saddr;
                p_wb.SEL_O = 0xF;
                p_wb.STB_O = true;
                p_wb.CYC_O = true;
                p_wb.WE_O  = false;

                // sc_core::wait for ack
                WaitWbAck();
#if DEBUG_WB2
				std::cout << "Ack WBM lecture OK" << std::endl;
#endif
                num_reads++;
                *dest++ = (uint32_t) p_wb.DAT_I.read();
                // next data
                saddr = saddr + 4;
            }
            // clean request after clk falling edge
            sc_core::wait(p_clk.negedge_event());
            CleanWb();
#if DEBUG_WB
			std::cout << "WBMaster lecture OK" << std::endl;
#endif
        }

    template<typename wb_param>
        void     WbMasterModule<wb_param>::wb_write_blk
        ( uint32_t saddr, uint8_t *mask, uint32_t *data, uint32_t num)
        {
#if DEBUG_WB
			std::cout << "WBMaster ecrit "<< num << "mots en " << saddr << std::endl; 
#endif
            for (uint32_t i = 0; i< num; i++)
            {
                // set request on clk falling edge
                sc_core::wait(p_clk.negedge_event());
                p_wb.DAT_O = *data++;
                p_wb.ADR_O = saddr;
                p_wb.SEL_O = *mask++;
                p_wb.STB_O = true;
                p_wb.CYC_O = true;
                p_wb.WE_O  = true;
                // sc_core::wait for ack
                WaitWbAck();
#if DEBUG_WB2
				std::cout << "Ack WBM ecriture OK" << std::endl;
#endif
                num_writes++;
                // next data
                saddr = saddr + 4;
            }
            // clean request after clk falling edge
            sc_core::wait(p_clk.negedge_event());
            CleanWb();
#if DEBUG_WB
			std::cout << "WBMaster ecriture OK" << std::endl;
#endif
        }

    // Reset
    template<typename wb_param>
        void     WbMasterModule<wb_param>:: reset()
        {
            trans_wait_cycles    = 0;
            num_writes           = 0;
            num_reads            = 0;
            CleanWb();
        }

    // print simulation statistics
    template<typename wb_param>
        void     WbMasterModule<wb_param>:: print_stats()
        {
            std::cout << *this << std::endl;
        }

    ////////////////////////////////////////////////////////////////////////////
    //  Constructor
    ////////////////////////////////////////////////////////////////////////////

    template<typename wb_param>
        WbMasterModule<wb_param>::WbMasterModule ( 
                sc_core::sc_in<bool> &p_clk,
				sc_core::sc_in<bool> &p_resetn,
                soclib::caba::WbMaster<wb_param> &p_wb
                ): p_clk(p_clk),p_resetn(p_resetn),p_wb(p_wb)
        {
        // Evetually add some messages
        }
}}

