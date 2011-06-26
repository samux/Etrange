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


#include "video_out.h"
#define tmpl(x) template<typename wb_param> x VideoOut<wb_param>
namespace soclib { namespace caba {

    /////////////////////////////////////////////////////////////////////
    //	Constructor
    ////////////////////////////////////////////////////////////////////

    tmpl(/**/)::VideoOut (sc_core::sc_module_name insname,
                          uint32_t * wb_tab,
                          const int w ,
                          const int h ,
                          const int lsync ,
                          const int fsync ):
               sc_core::sc_module(insname),
               p_WIDTH(w),
               p_HEIGHT(h),
               p_LINE_SYNC(lsync),
               p_FRAME_SYNC(fsync),
               wb_tab(wb_tab),
               p_clk("p_clk"),
               p_resetn("p_resetn"),
               master0(p_clk,p_resetn, p_wb),
               fifo(48*5)
    {
      SC_THREAD(read_image);
      sensitive << p_clk.pos();
      dont_initialize();

      SC_THREAD(gen_sorties);
      sensitive << pixel_clk.pos();
      dont_initialize();

      std::cout << name()
                << " was created succesfully " << std::endl;
    }

    ///////////////////////////
    // GEN_SORTIES
    //////////////////////////

    tmpl(void)::gen_sorties()
    {
      std::cout << " VOUT GEN_SORTIES: START " << std::endl;

      unsigned char pixel_tmp;

      for(;;)
      {

        if(p_resetn == false)
        {
          reset:

          pixel_out = 0;
          line_valid = false;
          frame_valid = false;
          p_interrupt = 0;
          std::cout << " VOUT GEN_SORTIES: RESET " << std::endl;
          wait();
        }

        else
        {
          while (fifo.num_available() == 0)
            wait();

          for(unsigned int i=0; i<( p_HEIGHT + p_FRAME_SYNC ); i++)
            for(unsigned int j=0; j<( p_WIDTH + p_LINE_SYNC ); j++)
            {

              // If we are in the active windows, the current pixel is processed.
              if((i<p_HEIGHT) && (j>p_LINE_SYNC-1))
              {
                if (!fifo.nb_read(pixel_tmp))
                {
                  std::cout<< "VOUT GEN_SORTIES: FIFO VIDE" <<std::endl;
                }
                else
                  pixel_out = pixel_tmp;
              }
              else
                pixel_out = 0xBB;

              // Generation of line valid
              line_valid = (i<p_HEIGHT) && (j>p_LINE_SYNC-1);

              // Generation of frame vad
              frame_valid = (i<p_HEIGHT);

              // Then we wait for the next clock rising edge or a reset
              wait();

              // If we are woke up by a reset, we restart from the beginning
              if(p_resetn == false)
                goto reset;
            }

          p_interrupt = 1;
          wb_tab[3] = 0;
          std::cout << " VOUT GEN_SORTIES: INTERRUPTION SENT " << std::endl;
          wait();
          wait();
          wait();
          p_interrupt = 0;
        }
      }
    }
    // Reading of the frame in the RAM from the address given
    // by the processor through the wishbone bus
    tmpl(void)::VideoOut::read_image()
    {
      // Pixels are packed 4 by 4 because
      // it's the lenght of the bursts that will
      // be read from the ram
      uint32_t im_addr;
      uint32_t buffer[VOUT_PACK];

      std::cout << " VOUT READ_IMAGE: START " << std::endl;

      for (;;) {

        if (p_resetn == false)
        {
          im_addr = 0;
          wb_tab[3] = 0;
          wb_tab[2] = 0;
          while (fifo.read());
        }

        // If we got a new frame, we update the
        // address of the frame in ram
        while (!wb_tab[3])
          wait();

        im_addr = wb_tab[2];
        wb_tab[3] = 0;

        std::cout << " VOUT READ_IMAGE: NOUVELLE ADRESSE " << wb_tab[2] << std::endl;

        for (unsigned int i = 0; i < (p_HEIGHT * p_WIDTH) / (VOUT_PACK * 4); i++)
        {
          master0.wb_read_blk(im_addr + 4 * i * VOUT_PACK,
                              VOUT_PACK,
                              buffer);

          for (int j = 0; j < VOUT_PACK; j++)
          {
            for (int k = 3; k>=0; k--)
            {
              fifo.write(buffer[j] >> 8 * k);
              buffer[j] = buffer[j] - ((buffer[j] >> 8 * k) << 8 * k);
            }
          }

        }
        wb_tab[3] = 0;
      }
    }

  }}

