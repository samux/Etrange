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
#include "iss2_simhelper.h"
#include "lm32.h"
#include "vci_ram.h"
#include "vci_rom.h"
#include "vci_multi_tty.h"

#include "wb_signal.h"

#include "wb_xcache_wrapper.h"

// WB interconnect
#include "wb_interco.h"

// WB->VCI wrappers
#include "wb_slave_vci_initiator_wrapper.h"

// locals
#include "segmentation.h"

#include "hdl/include/video_in.h"
#include "hdl/include/video_out.h"
//wb_slave
#include "hdl/include/wb_simple_slave.h"

//Video
#include "video_gen.h"
#include "video_calc.h"
#include "display.h"

// SystemC main
int _main(int argc, char *argv[])
{
  using namespace sc_core;
  //using namespace soclib::caba;

  // Avoid repeating these everywhere
  using soclib::common::IntTab;
  using soclib::common::Segment;

  // Define our VCI parameters
  typedef soclib::caba::VciParams<4,8,32,1,1,1,8,1,1,1> vci_param;

  // Define our WB parameters
  typedef soclib::caba::WbParams<32,32> wb_param;

  // Mapping table
  soclib::common::MappingTable maptab(32, IntTab(8), IntTab(8), 0x80000000);

  maptab.add(Segment("rom" , ROM_BASE , ROM_SIZE , IntTab(0), true));
  maptab.add(Segment("ram" , RAM_BASE , RAM_SIZE , IntTab(1), true));
  maptab.add(Segment("tty"  , TTY_BASE  , TTY_SIZE  , IntTab(2), false));
  maptab.add(Segment("wb_slave"  , WBS_BASE  , WBS_SIZE  , IntTab(3), false));

  // Global signals
  sc_time   clk_periode_pixel(40, SC_NS); // clk period 25 MHz
  sc_time	clk_periode_system(10, SC_NS); // clk period 100 MHz
  sc_clock	signal_clk("signal_clk",clk_periode_pixel);
  sc_clock	system_clk("system_clk",clk_periode_system);

  sc_signal<bool> signal_resetn("signal_resetn");

  // Interconnection signals
  soclib::caba::VciSignals<vci_param> signal_vci_tty("signal_vci_tty");
  soclib::caba::VciSignals<vci_param> signal_vci_rom("signal_vci_vcirom");
  soclib::caba::VciSignals<vci_param> signal_vci_ram("signal_vci_vciram");

  // WB interconnect signals
  soclib::caba::WbSignal<wb_param> signal_wb_lm32("signal_wb_lm32");
  soclib::caba::WbSignal<wb_param> signal_wb_ram("signal_wb_ram");
  soclib::caba::WbSignal<wb_param> signal_wb_rom("signal_wb_rom");
  soclib::caba::WbSignal<wb_param> signal_wb_tty("signal_wb_tty");
  soclib::caba::WbSignal<wb_param> signal_wb_slave("signal_wb_slave");
  soclib::caba::WbSignal<wb_param> signal_wb_vin  ("signal_wb_vin");
  soclib::caba::WbSignal<wb_param> signal_wb_vout  ("signal_wb_vout");
  soclib::caba::WbSignal<wb_param> signal_wb_vcalc_read  ("signal_wb_vcalc_read");
  soclib::caba::WbSignal<wb_param> signal_wb_vcalc_write  ("signal_wb_vcalc_write");

  // WB slave data
  sc_signal<sc_uint<32> > wb_data_0("wb_data_0");
  sc_signal<sc_uint<32> > wb_data_1("wb_data_1");
  sc_signal<sc_uint<32> > wb_data_2("wb_data_2");
  sc_signal<sc_uint<32> > wb_data_3("wb_data_3");
  sc_signal<sc_uint<32> > wb_data_4("wb_data_4");
  sc_signal<sc_uint<32> > wb_data_5("wb_data_5");
  sc_signal<sc_uint<32> > wb_data_6("wb_data_6");
  sc_signal<sc_uint<32> > wb_data_7("wb_data_7");

  /**********************************************
   * IRQ
   *********************************************/
  // Irq from uart
  sc_signal<bool> signal_tty_irq("signal_tty_irq");
  // Irq from video_in
  sc_signal<bool> signal_video_in_irq("signal_video_in_irq");
  // Irq from video_out
  sc_signal<bool> signal_video_out_irq("signal_video_out_irq");
  // Irq from video_calc
  sc_signal<bool> signal_video_calc_irq("signal_video_calc_irq");
  // Unconnected irqs
  sc_signal<bool> unconnected_irq ("unconnected_irq");

  // Video signals
  sc_signal<bool>        line_valid_in("line_val_in");
  sc_signal<bool>        frame_valid_in("frame_val_in");

  sc_signal<sc_uint<8> > pixel_in("pixel_val_in");

  sc_signal<bool>        line_valid_out("line_val_out");
  sc_signal<bool>        frame_valid_out("frame_val_out");

  sc_signal<sc_uint<8> > pixel_out("pixel_val_out");

  // Components
  // lm32 real cache configuration can be:
  // Ways 1 or 2
  // Sets 128,256,512 or 1024
  // Bytes per line 4, 8 or 16
  // Here we have 2 way, 128 set and 8 bytes per set
  // To simulate a processor without a these parameters should be
  // changed to 1,1,4
  soclib::caba::WbXcacheWrapper
    <wb_param, soclib::common::Iss2Simhelper<soclib::common::LM32Iss <false > > >
    lm32("lm32", 0, maptab,IntTab(0), 2,128,8, 2,128,8);

  // Elf loader
  soclib::common::Loader loader("soft/soft.elf");

  // Memories
  soclib::caba::VciRom<vci_param> rom("rom", IntTab(0), maptab, loader);
  soclib::caba::VciRam<vci_param> ram("ram", IntTab(1), maptab, loader);
  soclib::caba::VciMultiTty<vci_param> vcitty("vcitty",	IntTab(2), maptab, "tty.log", NULL);

  // WB interconnect
  //                                           sc_name    maptab  masters slaves
  soclib::caba::WbInterco<wb_param> wbinterco("wbinterco",maptab, 5,4);

  ////////////////////////////////////////////////////////////
  /////////////////// WB -> VCI Wrappers /////////////////////
  ////////////////////////////////////////////////////////////

  // Ram
  soclib::caba::WbSlaveVciInitiatorWrapper<vci_param, wb_param> ram_w ("ram_w") ;
  ram_w.p_clk               (system_clk);
  ram_w.p_resetn            (signal_resetn);
  ram_w.p_vci               (signal_vci_ram);
  ram_w.p_wb                (signal_wb_ram);

  // Rom
  soclib::caba::WbSlaveVciInitiatorWrapper<vci_param, wb_param> rom_w ("rom_w") ;
  rom_w.p_clk               (system_clk);
  rom_w.p_resetn            (signal_resetn);
  rom_w.p_vci               (signal_vci_rom);
  rom_w.p_wb                (signal_wb_rom);

  // Tty
  soclib::caba::WbSlaveVciInitiatorWrapper<vci_param, wb_param> tty_w ("tty_w") ;
  tty_w.p_clk               (system_clk);
  tty_w.p_resetn            (signal_resetn);
  tty_w.p_vci               (signal_vci_tty);
  tty_w.p_wb                (signal_wb_tty);

  ////////////////////////////////////////////////////////////
  /////////////////////WB Slave //////////////////////////////
  ////////////////////////////////////////////////////////////
  wb_simple_slave simple_slave ("WB_simple_slave", "wb_simple_slave");

  simple_slave.p_clk(system_clk);
  simple_slave.p_resetn(signal_resetn);
  simple_slave.p_wb_DAT_I(signal_wb_slave.MWDAT);
  simple_slave.p_wb_DAT_O(signal_wb_slave.MRDAT);
  simple_slave.p_wb_ADR_I(signal_wb_slave.ADR);
  simple_slave.p_wb_ACK_O(signal_wb_slave.ACK);
  simple_slave.p_wb_CYC_I(signal_wb_slave.CYC);
  simple_slave.p_wb_ERR_O(signal_wb_slave.ERR);
  simple_slave.p_wb_LOCK_I(signal_wb_slave.LOCK);
  simple_slave.p_wb_RTY_O(signal_wb_slave.RTY);
  simple_slave.p_wb_SEL_I(signal_wb_slave.SEL);
  simple_slave.p_wb_STB_I(signal_wb_slave.STB);
  simple_slave.p_wb_WE_I(signal_wb_slave.WE);
  simple_slave.wb_data_0(wb_data_0);
  simple_slave.wb_data_1(wb_data_1);
  simple_slave.wb_data_2(wb_data_2);
  simple_slave.wb_data_3(wb_data_3);

  ////////////////////////////////////////////////////////////
  ///////////////////Video modules ///////////////////////////
  ////////////////////////////////////////////////////////////
  soclib::caba::VideoGen my_videogen ("video_gen");

  my_videogen.clk (signal_clk);
  my_videogen.reset_n(signal_resetn);
  my_videogen.line_valid(line_valid_in);
  my_videogen.frame_valid(frame_valid_in);
  my_videogen.pixel_out(pixel_in);

  video_in my_video_in ("Video_in","video_in");

  my_video_in.clk(system_clk);
  my_video_in.clk_in(signal_clk);
  my_video_in.reset_n(signal_resetn);
  my_video_in.line_valid(line_valid_in);
  my_video_in.frame_valid(frame_valid_in);
  my_video_in.pixel_in(pixel_in);
  //signaux wishbone
  my_video_in.p_wb_STB_O(signal_wb_vin.STB);
  my_video_in.p_wb_CYC_O (signal_wb_vin.CYC);
  my_video_in.p_wb_LOCK_O(signal_wb_vin.LOCK);
  my_video_in.p_wb_WE_O(signal_wb_vin.WE);
  my_video_in.p_wb_SEL_O(signal_wb_vin.SEL);
  my_video_in.p_wb_ADR_O(signal_wb_vin.ADR);
  my_video_in.p_wb_ACK_I (signal_wb_vin.ACK);
  my_video_in.p_wb_DAT_O (signal_wb_vin.MWDAT);
  my_video_in.interrupt(signal_video_in_irq);
  my_video_in.wb_reg_ctr (wb_data_1);
  my_video_in.wb_reg_data (wb_data_0);

  video_out my_video_out ("Video_out", "video_out");

  my_video_out.clk (system_clk);
  my_video_out.clk_out   (signal_clk);
  my_video_out.nRST (signal_resetn);

  my_video_out.wb_reg_data(wb_data_2);
  my_video_out.wb_reg_ctr(wb_data_3);

  my_video_out.line_valid(line_valid_out);
  my_video_out.frame_valid(frame_valid_out);
  my_video_out.pixel_out(pixel_out);

  my_video_out.p_wb_STB_O    (signal_wb_vout.STB);
  my_video_out.p_wb_CYC_O    (signal_wb_vout.CYC);
  my_video_out.p_wb_LOCK_O    (signal_wb_vout.LOCK);
  my_video_out.p_wb_SEL_O    (signal_wb_vout.SEL);
  my_video_out.p_wb_WE_O    (signal_wb_vout.WE);
  my_video_out.p_wb_ADR_O	(signal_wb_vout.ADR);
  my_video_out.p_wb_ACK_I    (signal_wb_vout.ACK);
  my_video_out.p_wb_DAT_I    (signal_wb_vout.MRDAT);
  my_video_out.interrupt    (signal_video_out_irq);
 
soclib::caba::VideoCalc<wb_param> my_video_calc ("video_calc", simple_slave.data_t);
  my_video_calc.p_clk   (system_clk);
  my_video_calc.p_resetn(signal_resetn);
  my_video_calc.p_interrupt    (signal_video_calc_irq);
  my_video_calc.p_wb_read    (signal_wb_vcalc_read);
  my_video_calc.p_wb_write   (signal_wb_vcalc_write);

  soclib::caba::Display my_display ("My_display");

  my_display.clk (signal_clk);
  my_display.reset_n(signal_resetn);
  my_display.line_valid(line_valid_out);
  my_display.frame_valid(frame_valid_out);
  my_display.pixel_in(pixel_out);

  ////////////////////////////////////////////////////////////
  ///////////////////// WB Net List //////////////////////////
  ////////////////////////////////////////////////////////////

  wbinterco.p_clk(system_clk);
  wbinterco.p_resetn(signal_resetn);

  wbinterco.p_from_master[0](signal_wb_lm32);
  wbinterco.p_from_master[1](signal_wb_vin);
  wbinterco.p_from_master[2](signal_wb_vout);
  wbinterco.p_from_master[3](signal_wb_vcalc_read);
  wbinterco.p_from_master[4](signal_wb_vcalc_write);

  wbinterco.p_to_slave[0](signal_wb_rom);
  wbinterco.p_to_slave[1](signal_wb_ram);
  wbinterco.p_to_slave[2](signal_wb_tty);
  wbinterco.p_to_slave[3](signal_wb_slave);

  // Lm32
  lm32.p_clk(system_clk);
  lm32.p_resetn(signal_resetn);
  lm32.p_wb(signal_wb_lm32);
  // !! the real LM32 irq are active low
  // To avoid adding inverters here, we consider
  // them active high
  lm32.p_irq[0] (signal_tty_irq);
  lm32.p_irq[1] (signal_video_in_irq);
  lm32.p_irq[2] (signal_video_out_irq);
  lm32.p_irq[3] (signal_video_calc_irq);
  for (int i = 4; i < 32; i++)
    lm32.p_irq[i] (unconnected_irq);

  ////////////////////////////////////////////////////////////
  //////////////////// VCI Net List //////////////////////////
  ////////////////////////////////////////////////////////////

  // Rom
  rom.p_clk(system_clk);
  rom.p_resetn(signal_resetn);
  rom.p_vci(signal_vci_rom);

  // Ram
  ram.p_clk(system_clk);
  ram.p_resetn(signal_resetn);
  ram.p_vci(signal_vci_ram);

  // Uart
  vcitty.p_clk(system_clk);
  vcitty.p_resetn(signal_resetn);
  vcitty.p_vci(signal_vci_tty);
  vcitty.p_irq[0](signal_tty_irq);

  ////////////////////////////////////////////////////////////
  //////////////// Traces ////////////////////////////////////
  ////////////////////////////////////////////////////////////
   //sc_trace_file *TRACEFILE;
   //TRACEFILE = sc_create_vcd_trace_file("vcd_traces");
   //sc_trace (TRACEFILE, signal_resetn, "resetn" );
   //sc_trace (TRACEFILE, signal_clk,    "clk"    );
   //sc_trace (TRACEFILE, system_clk,    "clk"    );
   //sc_trace (TRACEFILE, signal_wb_lm32,"lm32_wb");
   //sc_trace (TRACEFILE, signal_wb_ram, "ram_wb" );
   //sc_trace (TRACEFILE, signal_wb_rom, "rom_wb" );
   //sc_trace (TRACEFILE, signal_wb_tty, "tty_wb" );
   //sc_trace (TRACEFILE, signal_tty_irq, "tty_wb" );
   //sc_trace (TRACEFILE, signal_wb_vin,  "Vin" );
   //sc_trace (TRACEFILE, signal_wb_vout, "Vout" );
   //sc_trace (TRACEFILE, signal_video_in_irq, "Vin_irq" );
   //sc_trace (TRACEFILE, signal_video_out_irq, "Vout_irq" );
   //sc_trace (TRACEFILE, signal_video_calc_irq, "Vcalc_irq" );

  ////////////////////////////////////////////////////////////
  //////////////// Start Simulation //////////////////////////
  ////////////////////////////////////////////////////////////
  // unconnected irqs
  unconnected_irq = false;

  /* Initialisation de la simulation */
  signal_resetn = true;
  sc_start(clk_periode_pixel);
  sc_start(clk_periode_pixel);

  /* Generation d'un reset */
  signal_resetn = false;
  sc_start(clk_periode_pixel);
  sc_start(clk_periode_pixel);
  sc_start(clk_periode_pixel);
  signal_resetn = true;

  // Run SystemC simulator
#ifdef MTI_SYSTEMC
  sc_start();
#else
  sc_start(sc_time(600000, SC_US));
#endif

  //sc_close_vcd_trace_file(TRACEFILE);

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

