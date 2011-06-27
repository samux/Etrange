/*!
 * \file video_in.cpp
 * \brief Store in RAM the incoming stream
 * \author Caroline Keramsi
 * \date 20/06/2011
 *
 * Two threads: 	- one which reads the pixels from the incoming stream and stores them in a fifo
 * 					- one which reads the fifo and store the pixels in RAM
 *
 */

#include "video_in.h"

#define tmpl(x) template<typename wb_param> x Video_in<wb_param>
namespace soclib { namespace caba {

    // The constructor
    tmpl(/**/)::Video_in (sc_core::sc_module_name insname,
                          uint32_t * tab,
                          const int w ,
                          const int h ,
                          const int lsync ,
                          const int fsync):
               sc_core::sc_module(insname),
               p_WIDTH(w),
               p_HEIGHT(h),
               p_LINE_SYNC(lsync),
               p_FRAME_SYNC(fsync),
               wb_tab(tab),
               p_clk("p_clk"),
               p_resetn("p_resetn"),
               master0(p_clk,p_resetn, p_wb),
               fifo(48*5)
    {

      SC_THREAD(read_pixels);
      sensitive << pixel_clk.pos();
      dont_initialize();

      SC_THREAD(store_pixels);
      sensitive << p_clk.pos();
      dont_initialize();

      // beginning of the image
      pixel_c = 0;
      pixel_l = 0;

      std::cout << name()
		<< " was created successfully " << std::endl;
    }

    tmpl(void)::read_pixels()
    {
      std::cout << " VIN READ_PIXELS: START " << std::endl;

      bool start_fifo = false;

      for(;;)
      {

        if(p_resetn == false)
        {
          reset:
          pixel_c = 0;
          pixel_l = 0;
          wait();
        }

        else
        {
          // valid image
          if (line_valid && frame_valid)
          {
            if (pixel_c < p_WIDTH && pixel_l < p_HEIGHT)
            {
              if (start_fifo)
                fifo.nb_write(pixel_in.read());
              pixel_c++;
            }
            else
            {
              std::cout << name()
                        << " VIN PIXEL_READS: TOO MUCH PIXELS !"
                        << " lines : "
                        << pixel_l
                        << " col : "
                        << pixel_c
                        << std::endl;
              exit(-1);
            }
          }
          else
          {
            // horizontal synchro
            if (frame_valid && !line_valid)
            {
              if (pixel_c == p_WIDTH)
              {
                pixel_c = 0;
                pixel_l++;
              }
              else if (pixel_c != 0)
              {
                std::cout << name()
                          << " VIN PIXEL_READS: TOO MUCH PIXELS!!"
                          << " lines : "
                          << pixel_l
                          << "col :"
                          << pixel_c
                          << std::endl;
              }
            }
            // vertical synchro
            else if (!frame_valid)
            {
              if((pixel_c == p_WIDTH) && (pixel_l == p_HEIGHT -1))
              {
                pixel_c = 0;
                pixel_l = 0;
                if(first_address)
                  start_fifo = true;
              }
            }
          }
          wait();

          if(p_resetn == false)
            goto reset;
        }
      }
    }

    tmpl(void)::store_pixels()
    {
      uint32_t pixel_stored_l = 0;
      uint32_t pixel_stored_c = 0;
      first_address = false;

		//beginning in RAM of the image which will be stored
      uint32_t deb_im;
      uint32_t to_store[p_NB_PACK/4];
      uint8_t mask[p_NB_PACK/4];
      bool stockage_ok;

      for (int i = 0; i < p_NB_PACK/4; i++)
        mask[i] = 0xf;

      p_interrupt = 0;

      std::cout << "VIN STORE_PIXEL: START" << std::endl;

      for (;;)
      {

        if (p_resetn == false)
        {
          stockage_ok = false;
          pixel_stored_c = 0;
          pixel_stored_l = 0;
          p_interrupt = 0;
          first_address = false;
          std::cout << " VIN STORE_PIXEL: RESET " << std::endl;
          while(fifo.read());
        }

        while (!wb_tab[1] && !stockage_ok)
          wait();

        if (!stockage_ok)
        {
          pixel_stored_c = 0;
          pixel_stored_l = 0;
          deb_im = wb_tab[0];
          wb_tab[1] = 0;
          stockage_ok = true;
          if(!first_address)
            first_address = true;
          std::cout << " VIN STORE_PIXEL: NOUVELLE ADRESSE: " << deb_im << std::endl;
        }

        if ((unsigned int) fifo.num_available() > (p_NB_PACK))
        {

          for (unsigned int i = 0; i< p_NB_PACK/4; i++)
          {
            to_store[i] = 0;
            for (unsigned int j = 0; j < 4; j++)
            {
              to_store[i] = to_store[i] << 8;
              to_store[i] += fifo.read();
            }
          }

          master0.wb_write_blk(deb_im + (p_WIDTH * pixel_stored_l + pixel_stored_c), mask, to_store, p_NB_PACK/4);

          pixel_stored_c = pixel_stored_c + p_NB_PACK;
          if (pixel_stored_c == p_WIDTH)
          {
            pixel_stored_c = 0;
            pixel_stored_l++;
          }
        }
        else
          wait();

        if (pixel_stored_l == p_HEIGHT)
        {
          stockage_ok = false;
          std::cout << " VIN STORE_PIXEL: INTERRUPTION SENT " << std::endl;
          p_interrupt = 1;
          wait();
          wait();
          wait();
          p_interrupt = 0;
        }
      }
    }

  }}

