#include "coproc.h"
#include <iostream>

namespace soclib { namespace caba {
    // Constructor
    template <typename wb_param>
    WbSimpleMaster<wb_param>::coproc(sc_core::sc_module_name insname)
      : sc_core::sc_module(insname)
    {
      std::cout << " coproc module "  << name()
                << " created successfully" << std::endl;
      SC_METHOD(transition);
      dont_initialize();
      sensitive << p_clk.pos();
      SC_METHOD(genMoore);
      dont_initialize();
      sensitive << p_clk.neg();
    }

    // transition
    template <typename wb_param> \
    void  WbSimpleMaster<wb_param>::transition()
    {

      //reset
      if (p_resetn == false)
      {
        // add initialisations
        std::cout << name() << " Reset..."<<std::endl;
        frame_out_rdy = false;
        nb_cycles = 0;
        full_tile_count = 0;
        coeff_count = 0;
        line_index = 0;
        raw_indew = 0;
        c_state = IDLE;
        return;
      }

      // rising edge of clk
      std::cout << name() << " Rising edge of clk"<<std::endl;

      nb_cycles++;

      switch (c_state)
      {
        case IDLE :
          frame_out_rdy = false;

          if (frame_in_rdy)
          {
            _ADR_COEFF     = _ADR_RAM_COEFF_IN;
            _ADR_FRAME_IN  = _ADR_RAM_FRAME_IN;
            _ADR_FRAME_OUT = _ADR_RAM_FRAME_OUT;

            c_state = INIT_ADR_COEFF;
          }
          break;

        case INIT_ADR_COEFF :
          if (!p_mwb.ACK_I)
          {
            p_mwb.WE_O  = false;
            p_mwb.SEL_O = ADR_RAM;
            p_mwb.STB_O = true;
            p_mwb.CYC_O = true;
            p_mwb.ADR_O =
              _ADR_COEFF +
              total_tile_count * (TOTAL_COEFF_NUMBER + 2) * 4 +
              coeff_count * 4;

            c_state = PROC_COEFF_BUF;
          }
          break;

        case PROC_COEFF_BUFF :
          if (p_mwb.ACK_I)
          {
            coeff_in = p_mwb.DAT_I.read();

            if (coeff_count == TOTAL_COEFF_NUMBER)
            {
              line_init = line_index = coeff_in;
              coeff_count++;
              c_state = INIT_ADR_COEFF;
            }

            else if (coeff_count == (TOTAL_COEFF_NUMBER + 1))
            {
              raw_init = raw_index = coeff_in;
              coeff_count = 0;
              c_state = INIT_ADR_TILE_IN;
            }

            else
            {
              coeff_count++;
              coeff_buf.raw[coeff_count] = coeff_in;
              c_state = INIT_ADR_COEFF;
            }

            p_mwb.CYC_O = false;
            p_mwb.STB_O = false;
          }
          break;

        case INIT_ADR_TILE_IN :
          if (!p_wmb.ACK_I)
          {
            p_mwb.WE_O = false;
            p_mwb.SEL_O = ADR_RAM;

            if ((line_index >= FRAME_HEIGHT) || (raw_index >= FRAME_LENGTH))
              out_of_frame = true;
            else
            {
              out_of_frame = false;

              p_mwb.STB_O = true;
              p_mwb.CYC_O = true;
              p_mwb.ADR_O = _ADR_FRAME_IN +
                line_index * FRAME_LENGTH +
                raw_index;
            }

            c_state = PROC_TILE_IN_BUF;
          }
          break;

        case PROC_TILE_IN_BUF :
          if (out_of_frame)
          {
            for (int i = 0; i < 4; i++)
              tile_in[(line_index - line_init) * TILE_IN_SIZE +
                      (raw_index - raw_init) + i] = 0;
          }
          else if (p_mwb.ACK_I)
          {
            pixel_in = p_mwb.DAT_I.read();

            for (int i = 0; i < 4; i++)
              tile_in[(line_index - line_init) * TILE_IN_SIZE +
                      (raw_index - raw_init) + i] =
                (pixel_in >> PIXEL_SIZE*(3-i) & 0xff);

            raw_index += 4;

            if ((raw_index - raw_init) == TILE_IN_SIZE)
            {
              raw_index = raw_init;
              line_index++;
            }

            if ((line_index - line_init) == TILE_IN_SIZE)
            {
              line_index = 0;
              raw_index = 0;
              pixel_X = 0;
              pixel_Y = 0;
              c_state = PROC_INCREMENTAL;
            }
            else
              c_state = INIT_ADR_TILE_IN;

            p_mwb.CYC_O = false;
            p_mwb.STB_O = false;
          }
          break;

        case PROC_CALCUL :
          if (
            (coeff.reg.Px[3] >> DIGIT_FRAC - raw_init) < 0 ||
            (coeff.reg.Px[3] >> DIGIT_FRAC - (raw_init + TILE_IN_SIZE -1)) > 0 ||
            (coeff.reg.Py[3] >> DIGIT_FRAC - line_init) < 0 ||
            (coeff.reg.Py[3] >> DIGIT_FRAC - (line_init + TILE_IN_SIZE - 1)) > 0 ||
            )
            tile_out[pixel_X + pixel_Y * TILE_OUT_SIZE] = SATURATION;
          else
          {
            I[0][0] = tile_in[(coeff.reg.Px[3] >> DIGIT_FRAC - raw_init) +
                              (coeff.reg.Py[3] >> DIGIT_FRAC - line_init) * TILE_IN_SIZE];
            I[0][0] <<=16;

            I[1][0] = tile_in[(coeff.reg.Px[3] >> DIGIT_FRAC + 1 - raw_init) +
                              (coeff.reg.Py[3] >> DIGIT_FRAC - line_init) * TILE_IN_SIZE];
            I[1][0] <<=16;

            I[0][1] = tile_in[(coeff.reg.Px[3] >> DIGIT_FRAC - raw_init) +
                              (coeff.reg.Py[3] >> DIGIT_FRAC + 1 - line_init) * TILE_IN_SIZE];
            I[0][1] <<=16;

            I[1][1] = tile_in[(coeff.reg.Px[3] >> DIGIT_FRAC + 1 - raw_init) +
                              (coeff.reg.Py[3] >> DIGIT_FRAC + 1 - line_init) * TILE_IN_SIZE];
            I[1][1] <<=16;

            i_dx = coeff.reg.Px[3] & 0x0000ffff;
            i_dx = coeff.reg.Py[3] & 0x0000ffff;

            i_dx_1 = (1 << 16) - dx;
            i_dy_1 = (1 << 16) - dy;

            Intensity = (i_dx_1 * i_dy_1) * I[0][0] +
              (i_dx_1 * i_dy)             * I[0][1] +
              (i_dx * i_dy_1)             * I[1][0] +
              (i_dx * i_dy)               * I[1][1];

            tile_out[pixel_X + pixel_Y * TILE_OUT_SIZE] = (unsigned char) (Intensity >> 16);
          }

          for (int i = 3; i > 0; i--)
          {
            coeff.reg.Px[i] += coeff.reg.Px[i-1];
            coeff.reg.Py[i] += coeff.reg.Py[i-1];
          }

          if (pixel_X == 15)
          {
            pixel_X = 0;

            if (pixel_Y == 16)
              c_state = INIT_ADR_TILE_OUT;
            else
              pixel_Y++;

            for(int i = 3; i > 0; i--)
            {
              coeff.reg.Qx[i] += coeff.reg.Qx[i-1];
              coeff.reg.Qy[i] += coeff.reg.Qy[i-1];
            }

            for(int i = 2; i > 0; i--)
            {
              coeff.reg.Rx[i] += coeff.reg.Rx[i-1];
              coeff.reg.Ry[i] += coeff.reg.Ry[i-1];
            }

            coeff.reg.Sx[1] += coeff.reg.Sx[0];
            coeff.reg.Sy[1] += coeff.reg.Sy[0];

            coeff.reg.Px[3] = coeff.reg.Qx[3];
            coeff.reg.Py[3] = coeff.reg.Qy[3];
            coeff.reg.Px[2] = coeff.reg.Rx[2];
            coeff.reg.Py[2] = coeff.reg.Ry[2];
            coeff.reg.Px[1] = coeff.reg.Sx[1];
            coeff.reg.Py[1] = coeff.reg.Sy[1];
          }
          else
            pixel_X++;
          break;

        case INIT_ADR_TILE_OUT :
          if (!p_mwb.ACK_I)
          {
            p_mwb.WE_O = true;
            p_mwb.SEL_O = ADR_RAM;
            p_mwb.STB_O = true;
            p_mwb.CYC_O = true;
            p_mwb.ADR_O =
              _ADR_FRAME_OUT +
              line_tile_count * TILE_OUT_SIZE +
              ((total_tile_count / TILE_NUMBER) * TILE_OUT_SIZE + nb_line_out) * FRAME_LENGTH +
              pixel_out_count;

            p_mwb.DAT_O =
              (tile_out[nb_line_out * TILE_OUT_SIZE + pixel_out_count]     << (BUS_SIZE - PIXEL_SIZE))     |
              (tile_out[nb_line_out * TILE_OUT_SIZE + pixel_out_count + 1] << (BUS_SIZE - PIXEL_SIZE * 2)) |
              (tile_out[nb_line_out * TILE_OUT_SIZE + pixel_out_count + 2] << (BUS_SIZE - PIXEL_SIZE * 3)) |
              (tile_out[nb_line_out * TILE_OUT_SIZE + pixel_out_count + 3]);

            c_state = PROC_TILE_OUT_RAM;
          }
          break;

        case PROC_TILE_OUT_RAM :
          if (p_mwb.ACK_I)
          {
            pixel_out_count += 4;

            if (pixel_out_count == TILE_OUT_SIZE)
            {
              nb_line_out++;
              pixel_out_count = 0;
            }
            else if (nb_line_out == TILE_OUT_SIZE)
            {
              pixel_out_count = 0;
              nb_line_out = 0;
              total_tile_count++;
              line_tile_count++;

              if (line_tile_count == LINE_TILE_NUMBER)
                line_tile_count = 0;

              if (total_tile_count == FULL_TILE_NUMBER)
              {
                total_tile_count = 0;
                output_frame_rdy = true;
                c_state = IDLE;
              }
              else
                c_state = INIT_ADR_COEFF;
            }
            else
            {
              p_mwb.STB_O = false;
              p_mwb.CYC_O = false;
              c_state = INIT_ADR_TILE_OUT;
            }
          }
          break;

        default :
          break;
      }
    }

    // genMoore
    template <typename wb_param> \
    void WbSimpleMaster<wb_param>::genMoore()
    {
      // on the falling edge of clk
      std::cout << name() << " Falling edge of clk"<<std::endl;

    }

  }
}
