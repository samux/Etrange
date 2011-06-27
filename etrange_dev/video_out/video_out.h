/*!
 * \file video_calc.cpp
 * \brief This file contains the module which processes an image
 * \author Caroline Keramsi
 * \date 20/06/2011
 *
 *  Two threads :  - One that reads the pixel in the RAM and stores them in the fifo.
 *                 - One that reads the fifo and regenerate the video stream
 *
 */


#ifndef VIDEO_OUT_H
#define VIDEO_OUT_H

#define VOUT_PACK 32

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

      sc_core::sc_in<bool>              p_clk;            /*!< System clock (default = 100Mhz) */
      sc_core::sc_in<bool>              p_resetn;         /*!< Reset signal */
      sc_core::sc_out<bool>             p_interrupt;      /*!< to indicate that a frame has been processed */

      sc_core::sc_in<bool>              pixel_clk;        /*!< Pixel clock (default = 25Mhz) */
      sc_core::sc_out<unsigned char>    pixel_out;        /*!< The outcoming pixel */
      sc_core::sc_out<bool>             line_valid;       /*!< Line valid to respect the format of the video stream */
      sc_core::sc_out<bool>             frame_valid;      /*!< Frame valid to respect the format of the video stream */

      WbMaster<wb_param>                p_wb;             /*!<  */

      /*!
       * \brief Constructor
       *
       * VideoOut constructor
       *
       * \param tab: the link with the processor
       * \param w: image width (640 by default)
       * \param h: image height (480 by default)
       * \param lsync: synchronisation with the lines
       * \param fsync: synchronisation with the frames
       * */

      VideoOut(sc_module_name insname,
               uint32_t * wb_tab,
               const int w = 640,
               const int h = 480,
               const int lsync = 160,
               const int fsync = 40
        );

      /*!
       * \brief To regenerate the video stream
       *
       * Wait for the fifo the contains a minimal amount
       * of pixels, then regenerate the video stream by
       * taking the pixels from this fifo
       *
       */

      void                gen_sorties();

      /*!
       * \brief To read frams in the ram and fill the fifo
       *
       * Wait for an address sent by the processor to be valid.
       * As soon as an address is valid, this process starts reading
       * the ram and filling the fifo
       *
       */
      void                read_image();

      private:


      const uint32_t  p_WIDTH      ;
      /*!< Image width */
      const uint32_t  p_HEIGHT     ;
      /*!< Image height */
      const uint32_t  p_LINE_SYNC  ;
      /*!< Line synchronisation */
      const uint32_t  p_FRAME_SYNC ;
      /*!< Frame synchronisation */

      sc_fifo<uint8_t>   fifo;
      /*!< To communicate between the two threads */
      uint32_t * wb_tab;
      /*!< To communicate with the processor */

      WbMasterModule<wb_param> master0;
      /*!< wishbone master to read in RAM */

      protected:
      SC_HAS_PROCESS(VideoOut);
    };
  }}
#endif //VIDEO_OUT_H

