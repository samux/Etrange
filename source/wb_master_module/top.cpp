/*
 *
 * SOCLIB_LGPL_HEADER_BEGIN
 *
 * This file is part of SoCLib, GNU LGPLv2.1.
 *
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 *
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * SOCLIB_LGPL_HEADER_END
 *
 */


// C/C++ std libs
#include <iostream>
#include <cstdlib>
#include <cstdio>

// SocLib
#include "mapping_table.h"
#include "vci_ram.h"

#include "wb_signal.h"

// WB->VCI wrappers
#include "wb_slave_vci_initiator_wrapper.h"

// locals
#include "segmentation.h"

#include "master.h"

#define DO_TRACES

// real SystemC main
int _main(int argc, char *argv[])
{
    using namespace sc_core;
    // Avoid repeating these everywhere
    using soclib::common::IntTab;
    using soclib::common::Segment;

    // Define our VCI parameters
    typedef soclib::caba::VciParams<4,8,32,1,1,1,8,1,1,1> vci_param;

    // Define our WB parameters
    typedef soclib::caba::WbParams<32,32> wb_param;

    soclib::common::MappingTable maptab(32, IntTab(8), IntTab(8), 0x80000000);

    maptab.add(Segment("ram" , RAM_BASE , RAM_SIZE , IntTab(1), true));

    // Gloabal signals
    sc_time     clk_periode(10, SC_NS); // clk period
    sc_clock	signal_clk("signal_clk",clk_periode);
    sc_signal<bool> signal_resetn("signal_resetn");

    // interconnection signals
    soclib::caba::VciSignals<vci_param> signal_vci_ram("signal_vci_vciram");

    // WB interconnect signals
    soclib::caba::WbSignal<wb_param> signal_wb_ram("signal_wb_ram");
    soclib::caba::WbSignal<wb_param> signal_wb_m  ("signal_wb_m");

    // memories
    soclib::caba::VciRam<vci_param> ram("ram", IntTab(1), maptab);


    // Esclave simple
    soclib::caba::Master<wb_param> wbmaster("master");

    ////////////////////////////////////////////////////////////
    /////////////////// WB -> VCI Wrappers /////////////////////
    ////////////////////////////////////////////////////////////

    // ram
    soclib::caba::WbSlaveVciInitiatorWrapper<vci_param, wb_param> ram_w ("ram_w") ;
    ram_w.p_clk               (signal_clk);
    ram_w.p_resetn            (signal_resetn);
    ram_w.p_vci               (signal_vci_ram);
    ram_w.p_wb                (signal_wb_m);

    ////////////////////////////////////////////////////////////
    ///////////////////// WB Net List //////////////////////////
    ////////////////////////////////////////////////////////////

    wbmaster.p_clk   (signal_clk);
    wbmaster.p_resetn(signal_resetn);
    wbmaster.p_wb    (signal_wb_m);

    ////////////////////////////////////////////////////////////
    //////////////////// VCI Net List //////////////////////////
    ////////////////////////////////////////////////////////////

    // ram
    ram.p_clk(signal_clk);
    ram.p_resetn(signal_resetn);
    ram.p_vci(signal_vci_ram);

    ////////////////////////////////////////////////////////////
    //////////////// Traces ////////////////////////////////////
    ////////////////////////////////////////////////////////////

#ifdef DO_TRACES
    sc_trace_file *TRACEFILE;
    TRACEFILE = sc_create_vcd_trace_file("vcd_traces");
    sc_trace (TRACEFILE, signal_resetn,  "resetn"  );
    sc_trace (TRACEFILE, signal_clk,     "clk"     );
    sc_trace (TRACEFILE, signal_wb_m,    "wb_m");
    sc_trace (TRACEFILE, signal_vci_ram, "ram");
#endif

    ////////////////////////////////////////////////////////////
    //////////////// Start Simulation //////////////////////////
    ////////////////////////////////////////////////////////////

    // reset
    sc_start(sc_core::sc_time(0, SC_NS));
    signal_resetn = false;
    sc_start(sc_core::sc_time(100, SC_NS));
    signal_resetn = true;

    // print maping table
    std::cout << "Let's go..." << std::endl;
    std::cout << maptab;

    // run SystemC simulator
    sc_start( );

#ifdef DO_TRACES
    sc_close_vcd_trace_file(TRACEFILE);
#endif

    return EXIT_SUCCESS;
}

// fake sc_man to catch exceptions
int sc_main(int argc, char *argv[])
{
    try {
        return _main(argc, argv);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown exception occured" << std::endl;
        throw;
    }
    return 1;
}

