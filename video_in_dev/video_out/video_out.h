/*************************************************************
 *
 *      File : video_gen.cpp
 *      Author: T. Graba
 *      Credits: A. Polti
 *      Telecom ParisTECH
 *
 ************************************************************/


#ifndef VIDEO_OUT_H
#define VIDEO_OUT_H

#define VOUT_OFFSET 1
#define VOUT_PACK 64

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"

using namespace sc_core;
using namespace std;

namespace soclib { namespace caba {

    template<typename wb_param>
      class VideoOut
      :sc_module {

      public:
      // IO PORTS
      sc_in_clk           clk;
      sc_in_clk           clk_out;
      sc_in<bool>         reset_n;

      sc_out<bool>        line_valid;
      sc_out<bool>        frame_valid;

      sc_out<unsigned char> pixel_out;

      sc_out<bool> p_interrupt;

      //Wishbone
      sc_core::sc_in<bool> p_clk;
      sc_core::sc_in<bool> p_resetn;
      WbMaster<wb_param>   p_wb;

      ////////////////////////////////////////////////////
      //	constructor
      ////////////////////////////////////////////////////
      VideoOut(sc_module_name insname,
               uint32_t * wb_tab,
               const int w = 640,          // largeur d'image par defaut
               const int h = 480,          // hauteur par defaut
               const int lsync = 160,      // synchro ligne par defaut
               const int fsync = 40        // synchro trame par defaut
        );

      ////////////////////////////////////////////////////
      //	methods and structural parameters
      ////////////////////////////////////////////////////
      void                gen_sorties();
      void                read_image();

      private:

      // paramètres de l'image
      const uint32_t  p_WIDTH      ;
      const uint32_t  p_HEIGHT     ;
      const uint32_t  p_LINE_SYNC  ;
      const uint32_t  p_FRAME_SYNC ;

      sc_fifo<uint8_t>   fifo;
      uint32_t * wb_tab;

      WbMasterModule<wb_param> master0;

      protected:
      SC_HAS_PROCESS(VideoOut);
    };
  }}
#endif //VIDEO_OUT_H

