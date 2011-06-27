/*!
 * \file video_in.h
 * \brief Store in RAM the incoming stream
 * \author Caroline Keramsi
 * \date 20/06/2011
 *
 * Two threads: 	- one which reads the pixels and stores them in a fifo
 * 					- one which reads the fifo and store the pixels in RAM
 *
 */

#ifndef VIDEO_IN_H
#define VIDEO_IN_H

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"

#define p_NB_PACK 16*4
/*!< Length of burst which will be done to store in RAM */

using namespace sc_core;
using namespace std;

namespace soclib { namespace caba {

    template<typename wb_param>
      class Video_in: sc_module
    {
      public:

      sc_core::sc_in<bool>             p_clk;		/*!< System clock (100 MHz) */
      sc_core::sc_in<bool>             p_resetn;	/*!< Reset signal */
      sc_core::sc_out<bool>            p_interrupt;/*!< To indicates that an image has been stored in RAM */

      sc_core::sc_in<bool>             pixel_clk;	/*!< pixel clock (25MHz)*/
      sc_core::sc_in<unsigned char>    pixel_in;	/*!< The incoming pixel */
      sc_core::sc_in <bool>            line_valid;	/*!< line_valid signal */
      sc_core::sc_in <bool>            frame_valid;/*!< frame_valid signal */

      soclib::caba::WbMaster<wb_param> p_wb;


		/*!
		 * \brief Constructor
		 *
		 * VideoIn constructor
		 *
		 * \param tab: the link with the processor
		 * \param w: image width (640 by default)
		 * \param h: image height (480 by default)
		 * \param lsync: line synchro (160 by default)
		 * \param fsync: frame synchro (40 by default)
		 * */
      Video_in (sc_module_name insname,
                uint32_t * tab,
                const int w = 640,          /*!< image width (640 by default) */
                const int h = 480,          /*!< image height (480 by default) */
                const int lsync = 160,      /*!< line synchro (160 by default) */
                const int fsync = 40        /*!< frame synchro (40 by default) */
        );

		/*!
		 * \brief To read pixels from the incoming stream and store them in a fifo
		 */
      void read_pixels();

		/*!
		 * \brief To read the fifo and write them in RAM
		 */
      void store_pixels();

      private:

      // Settings of the image
      const uint32_t  p_WIDTH      ;
		/*!< Image width */
      const uint32_t  p_HEIGHT     ;
		/*!< Image height */
      const uint32_t  p_LINE_SYNC  ;
		/*!< Line synchro */
      const uint32_t  p_FRAME_SYNC ;
		/*!< Frame synchro */

      unsigned int pixel_c;
		/*<! pixel column */
      unsigned int pixel_l;
		/*<! pixel line */

      bool first_address;
		/*<! To synchronize the processor with Video_In */

      sc_fifo<unsigned char> fifo;
		/*<! To store the incoming stream */

      // Tableau de Wb_slave
      uint32_t * wb_tab;
		/*!< To communicate with the processor */

      WbMasterModule<wb_param> master0;
		/*!< wishbone master to write in RAM */

      protected:
      SC_HAS_PROCESS(Video_in);
    };
  }}
#endif //VIDEO_IN_H

