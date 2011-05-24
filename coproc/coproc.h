#ifndef COPROC_H
# define COPROC_H

#include <systemc>
#include"wb_master.h"
#include"wb_slave.h"

# define TILE_IN_SIZE 32
# define TILE_OUT_SIZE 16

# define FRAME_LENGTH 640
# define FRAME_HEIGHT 480
# define PIXEL_SIZE 8

# define DIGIT_INT 16
# define DIGIT_FRAC 16

# define ADR_RAM 0xf
# define BUS_SIZE 32

# define TOTAL_TILE_NUMBER 1200
# define LINE_TILE_NUMBER 40
# define TOTAL_COEFF_NUMBER 26
# define SATURATION 255

enum state {
  IDLE,
  INIT_ADR_COEFF,
  PROC_COEFF_BUF,
  INIT_ADR_TILE_IN,
  PROC_TILE_IN_BUF,
  PROC_CALCUl,
  INIT_ADR_TILE_OUT,
  PROC_TILE_OUT_RAM,
};


union u_coeff_inc
{
  int raw[26];
  struct
  {
    int Px[4], Qx[4], Rx[3], Sx[2];
    int Py[4], Qy[4], Ry[3], Sy[2];
  } reg;
};

namespace soclib { namespace caba {
    template <typename wb_param>
      class coproc: public sc_core::sc_module
    {

      protected:
      SC_HAS_PROCESS(coproc);

      public:
      sc_core::sc_in<bool>         p_clk;
      sc_core::sc_in<bool>         p_resetn;
      sc_core::sc_in<bool>         frame_in_rdy;
      sc_core::sc_out<bool>        frame_out_rdy;

      WbMaster<wb_param>           p_mwb;
      WbSlave<wb_param>            p_swb;

      // Constructor
      coproc (sc_core::sc_module_name insname);

      // Methods
      void transition();
      void genMoore();

      private:

      // Registers
      unsigned int _ADR_COEFF_IN;
      unsigned int _ADR_FRAME_IN;
      unsigned int _ADR_FRAME_OUT;
      unsigned int _ADR_RAM_COEFF;
      unsigned int _ADR_RAM_FRAME_IN;
      unsigned int _ADR_RAM_FRAME_OUT;

      unsigned int nb_cycles;

      unsigned int coeff_count;
      unsigned int total_tile_count;
      unsigned int coeff_in;
      unsigned int pixel_in;
      union u_coeff_inc coeff_buf;
      enum state c_state;

      unsigned int line_index, line_init;
      unsigned int raw_index, raw_init;

      unsigned char tile_in[TILE_IN_SIZE * TILE_IN_SIZE];
      unsigned char tile_out[TILE_OUT_SIZE * TILE_OUT_SIZE];
      bool out_of_frame;
      unsigned int pixel_X, pixel_Y;
      unsigned int nb_line_out;
      unsigned int pixel_out_count;
      unsigned int line_tile_count;


    };

  }
}

#endif
