/*
 * =============================================================================
 *
 *       Filename:  master.cpp
 *
 *    Description:  WB master
 *                  This module demonstrate WbMasterModule usage
 *
 *         Author:  Tarik Graba (TG), tarik.graba@telecom-paristech.fr
 *        Company:  Telecom Paris TECH
 *
 * =============================================================================
 */


#include <cstdio>

#include <fstream>
#include <sstream>
#include <iomanip>

#include "master.h"


#define tmpl(x) template<typename wb_param> x Master<wb_param>



namespace soclib { namespace caba {

    tmpl(/**/)::Master (sc_module_name name )
        :sc_core::sc_module(name), p_clk("p_clk"),p_resetn("p_resetn"), master0(p_clk,p_wb)
    {
        // sc thread
        SC_THREAD(process);
        // no sensitivity list, waits have to be explicit
    }

    // main process
    tmpl(void)::process()
    {
        // handle a reset cmd
        bool reset_done = false;

        uint8_t  MASK[20] = {
            0xa, 0x5, 0xa, 0x5, 0xa, 0x5, 0xa, 0x5, 0xa, 0x5, 
            0xa, 0x5, 0xa, 0x5, 0xa, 0x5, 0xa, 0x5, 0xa, 0x5
        };
        uint32_t DATA[20] = {
        0x0000, 0x0011, 0x0022, 0x0033, 0x0044, 0x0055, 0x0066, 0x0077, 0x0088, 0x0099,
        0x00aa, 0x00bb, 0x00cc, 0x00dd, 0x00ee, 0x00ff, 0x1100, 0x1111, 0x1122, 0x1133,
        };

        uint32_t RDATA[20];

        for (;;) {

            if (!p_resetn) // reset
            {
                master0.reset();
                reset_done = true;
            }
            else // clk event
            {
                int val;
                // check if the reset has been done
                if (!reset_done)
                {
#ifdef SOCLIB_MODULE_DEBUG
                    std::cout
                        << name()
                        << " : has not been reseted yet!!!"
                        << std::endl;
#endif
                    wait(p_clk.posedge_event());
                    continue;
                }
                // Call to blocking functions is allowed
                wait_cycles(20);
                // single write of 4 bytes 0xC01dCafe @0x40000000
                master0.wb_write_at(0x40000000,0xf,0xC01dCafe);
                // single read @0x40000000
                val = master0.wb_read_at(0x40000000);
                // Print...
                std::cout << "FIRST READ VAL " << std::hex << val << std::endl;

                wait_cycles(20);
                master0.wb_write_at(0x40000000,0xf,0xbbbbbbbb);
                wait_cycles(20);
                val = master0.wb_read_at(0x40000000);
                std::cout << "SECOND READ VAL " << std::hex << val << std::endl;

                wait_cycles(30);
                // write a data block of 20 "words" starting @0x40001000
                // DATA and MASK are the pointer to the array containning the data
                master0.wb_write_blk(0x40001000,MASK,DATA,20);
                wait_cycles(30);
                // Read a block of 20 words
                // The result is stored in RDATA which has been allocated first
                master0.wb_read_blk(0x40001000,20,RDATA);
                //Print
                for (int i=0; i<20; i++)
                    std::cout << "Read value " << std::dec << i 
                    << " : " << std::hex << RDATA[i] << std::endl;

                wait_cycles(30);
                // Printing statistics
                std::cout << "END OF SIM..." << master0 << std::endl;
                // Or
                master0.print_stats();
                // Reset The master0 resets the its statistic counters 
                master0.reset();
                master0.print_stats();

                // Eventually stop the simulation
                sc_core::sc_stop();

            }

            // wait rising edge of clk
            wait(p_clk.posedge_event());

        } // infinite loop

    }


    // wait cycles
    tmpl(void):: wait_cycles (uint32_t delay)
    {
#ifdef SOCLIB_MODULE_DEBUG
        std::cout
            << name()
            << " wait start @" << sc_time_stamp()
            << std::endl;
#endif
        while (delay--) {
            wait(p_clk.posedge_event());
        }
#ifdef SOCLIB_MODULE_DEBUG
        std::cout
            << name()
            << " wait end @" << sc_time_stamp()
            << std::endl;
#endif
    }

}}// namespace

