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

#include "video_gen.h"
#include "display.h"

// SystemC main
int sc_main(int argc, char *argv[])
{
    using namespace sc_core;
    using namespace soclib::caba;

    // Gloabal signals
    sc_time     clk_periode(40, SC_NS); // clk period
    sc_clock	signal_clk("signal_clk",clk_periode);

    sc_signal<bool> signal_resetn("signal_resetn");

    sc_signal<bool>        line_valid("line_val");
    sc_signal<bool>        frame_valid("frame_val");

    sc_signal<unsigned char> pixel("pixel_val");


    VideoGen my_videogen ("video_gen");

    my_videogen.clk (signal_clk);
    my_videogen.reset_n(signal_resetn);
    my_videogen.line_valid(line_valid);
    my_videogen.frame_valid(frame_valid);
    my_videogen.pixel_out(pixel);

    Display my_display ("My_display");

    my_display.clk (signal_clk);
    my_display.reset_n(signal_resetn);
    my_display.line_valid(line_valid);
    my_display.frame_valid(frame_valid);
    my_display.pixel_in(pixel);

#if 0
    // open trace file
    sc_trace_file *my_trace_file;
    my_trace_file = sc_create_vcd_trace_file ("simulation_trace");

    // chronogrammes signaux CLK et NRESET
    sc_trace(my_trace_file, signal_clk,         "CLK");
    sc_trace(my_trace_file, signal_resetn,      "NRESET");

    // chronogrammes video
    sc_trace(my_trace_file, line_valid ,        "LINE_VAL");
    sc_trace(my_trace_file, frame_valid,        "FRAME_VAL");
    sc_trace(my_trace_file, pixel,       "PIXEL");
#endif

    /* Initialisation de la simulation */
    signal_resetn = true;
    sc_start(clk_periode);
    sc_start(clk_periode);

    /* Generation d'un reset */
    signal_resetn = false;
    sc_start(clk_periode);
    sc_start(clk_periode);
    sc_start(clk_periode);
    signal_resetn = true;

    // run SystemC simulator
#ifdef MTI_SYSTEMC
    sc_start();
#else
    sc_start(sc_time(500, SC_MS));
#endif

    return EXIT_SUCCESS;
}

