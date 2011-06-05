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

//wb_slave
#include "wb_simple_slave.h"

//Video
#include "video_gen.h"
#include "video_in.h"
#include "video_out.h"
#include "video_calc.h"
#include "display.h"

// SystemC main
int sc_main(int argc, char *argv[])
{
  using namespace sc_core;
  using namespace soclib::caba;

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
  sc_time	clk_periode_pixel(40, SC_NS); // clk period 25 MHz
  sc_time	clk_periode_system(10, SC_NS); // clk period 100 MHz
  sc_clock	signal_clk("signal_clk", clk_periode_pixel);
  sc_clock	system_clk("system_clk", clk_periode_system);

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
  soclib::caba::WbSignal<wb_param> signal_wb_vcalc  ("signal_wb_vcalc");

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

  sc_signal<unsigned char> pixel_in("pixel_val_in");

  sc_signal<bool>        line_valid_out("line_val_out");
  sc_signal<bool>        frame_valid_out("frame_val_out");

  sc_signal<unsigned char> pixel_out("pixel_val_out");

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
  soclib::caba::WbInterco<wb_param> wbinterco("wbinterco",maptab, 4,4);

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
  soclib::caba::WbSimpleSlave<wb_param> simple_slave ("WB_simple_slave", WBS_BASE, WBS_SIZE);
  simple_slave.p_clk(system_clk);
  simple_slave.p_resetn(signal_resetn);
  simple_slave.p_wb(signal_wb_slave);

  ////////////////////////////////////////////////////////////
  ///////////////////Video modules ///////////////////////////
  ////////////////////////////////////////////////////////////
  VideoGen my_videogen ("video_gen");
  my_videogen.clk (signal_clk);
  my_videogen.reset_n(signal_resetn);
  my_videogen.line_valid(line_valid_in);
  my_videogen.frame_valid(frame_valid_in);
  my_videogen.pixel_out(pixel_in);

  Video_in<wb_param> my_video_in ("video_in", simple_slave.data_tab);
  my_video_in.clk (system_clk);
  my_video_in.clk_in (signal_clk);
  my_video_in.reset_n(signal_resetn);
  my_video_in.line_valid(line_valid_in);
  my_video_in.frame_valid(frame_valid_in);
  my_video_in.pixel_in(pixel_in);
  my_video_in.p_clk   (signal_clk);
  my_video_in.p_resetn(signal_resetn);
  my_video_in.p_wb    (signal_wb_vin);
  my_video_in.p_interrupt    (signal_video_in_irq);

  VideoOut<wb_param> my_video_out ("video_out", simple_slave.data_tab);
  my_video_out.clk (system_clk);
  my_video_out.p_clk   (signal_clk);
  my_video_out.clk_out (signal_clk);
  my_video_out.reset_n(signal_resetn);
  my_video_out.line_valid(line_valid_out);
  my_video_out.frame_valid(frame_valid_out);
  my_video_out.pixel_out(pixel_out);
  my_video_out.p_resetn(signal_resetn);
  my_video_out.p_wb    (signal_wb_vout);
  my_video_out.p_interrupt    (signal_video_out_irq);

  VideoCalc<wb_param> my_video_calc ("video_calc", simple_slave.data_tab);
  my_video_calc.clk (system_clk);
  my_video_calc.p_clk   (signal_clk);
  my_video_calc.p_resetn(signal_resetn);
  my_video_calc.reset_n(signal_resetn);
  my_video_calc.p_wb    (signal_wb_vcalc);
  my_video_calc.img_rdy    (signal_video_calc_irq);

  Display my_display ("My_display");
  my_display.clk (signal_clk);
  my_display.reset_n(signal_resetn);
  my_display.line_valid(line_valid_out);
  my_display.frame_valid(frame_valid_out);
  my_display.pixel_in(pixel_out);

  ////////////////////////////////////////////////////////////
  ///////////////////// WB Net List //////////////////////////
  ////////////////////////////////////////////////////////////
  wbinterco.p_clk(signal_clk);
  wbinterco.p_resetn(signal_resetn);

  wbinterco.p_from_master[0](signal_wb_lm32);
  wbinterco.p_from_master[1](signal_wb_vin);
  wbinterco.p_from_master[2](signal_wb_vout);
  wbinterco.p_from_master[3](signal_wb_vcalc);

  wbinterco.p_to_slave[0](signal_wb_rom);
  wbinterco.p_to_slave[1](signal_wb_ram);
  wbinterco.p_to_slave[2](signal_wb_tty);
  wbinterco.p_to_slave[3](signal_wb_slave);

  // Lm32
  lm32.p_clk(signal_clk);
  lm32.p_resetn(signal_resetn);
  lm32.p_wb(signal_wb_lm32);
  // !! the real LM32 irq are active low
  // To avoid adding inverters here, we consider
  // them active high
  lm32.p_irq[0] (signal_tty_irq);
  lm32.p_irq[1] (signal_video_in_irq);
  lm32.p_irq[2] (signal_video_out_irq);
  lm32.p_irq[3] (signal_video_calc_irq);
  for (int i=4; i<32; i++)
    lm32.p_irq[i] (unconnected_irq);

  ////////////////////////////////////////////////////////////
  //////////////////// VCI Net List //////////////////////////
  ////////////////////////////////////////////////////////////

  // Rom
  rom.p_clk(signal_clk);
  rom.p_resetn(signal_resetn);
  rom.p_vci(signal_vci_rom);

  // Ram
  ram.p_clk(signal_clk);
  ram.p_resetn(signal_resetn);
  ram.p_vci(signal_vci_ram);

  // Uart
  vcitty.p_clk(signal_clk);
  vcitty.p_resetn(signal_resetn);
  vcitty.p_vci(signal_vci_tty);
  vcitty.p_irq[0](signal_tty_irq);

  ////////////////////////////////////////////////////////////
  //////////////// Traces ////////////////////////////////////
  ////////////////////////////////////////////////////////////
  /*sc_trace_file *TRACEFILE;
    TRACEFILE = sc_create_vcd_trace_file("vcd_traces");
    //sc_trace (TRACEFILE, signal_resetn, "resetn" );
    //sc_trace (TRACEFILE, signal_clk,    "clk"    );
    sc_trace (TRACEFILE, signal_wb_lm32,"lm32_wb");
    //sc_trace (TRACEFILE, signal_wb_ram, "ram_wb" );
    //sc_trace (TRACEFILE, signal_wb_rom, "rom_wb" );
    //sc_trace (TRACEFILE, signal_wb_tty, "tty_wb" );
    sc_trace (TRACEFILE, signal_tty_irq, "tty_wb" );
    //sc_trace (TRACEFILE, signal_wb_vin,  "Vin" );
    //sc_trace (TRACEFILE, signal_wb_vout, "Vout" );
    //sc_trace (TRACEFILE, signal_video_in_irq, "Vin_irq" );
    //sc_trace (TRACEFILE, signal_video_out_irq, "Vout_irq" );*/

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
  sc_start(sc_time(500000, SC_US));
#endif

  // Sc_close_vcd_trace_file(TRACEFILE);

  return EXIT_SUCCESS;
}
