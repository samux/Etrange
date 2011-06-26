/*!
 * \file video_calc.h
 * \brief This file contains the module which processes an image
 * \author Samuel Mokrani & Florian Thorey
 * \date 20/06/2011
 *
 * First, this module loads all coefficients and information to fill his cache. 
 * Then it fills the cache and can begin the incremental computation for the tile processed.
 * To finish, it computes the bilinear interpolation and put each pixel in a fifo to be write in RAM.
 */


#ifndef VIDEO_CALC_H
# define VIDEO_CALC_H

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"

# define T_W 16
/**< Tile width */
# define T_H 16
/**< Tile height */
// Largeur du cache
# define C_W 32
/**< Cache width */
# define C_H 32
/**< Cache height */
# define F_SIZE 2
/**< Number of tiles that can contain the fifo */
# define T_NB (640 * 480) / (16 * 16)
/**< Number of tiles per image */
# define T_L_NB p_WIDTH / T_W
/**< Number of tiles per line */
# define PIXEL_BLANC 255
/**< White pixel */
# define PIXEL_NOIR 0
/*!< Black pixel */
# define NB_COEFF 26
/*!< Number of coefficients for the incremental computation */

#define tmpl(x) template<typename wb_param> x VideoCalc<wb_param>

/*!
 * \union cic_u
 * \brief Coefficients for the incremantal computation and cache information
 *
 * Contains the four P and Q coefficients, the three R coefficients, the two S coefficients 
 * and the coordinates of the pixel which represents the beginning of the cache
 */
typedef union
{
  int32_t raw[NB_COEFF + 2]; /*!< All the coefficients and the beginning of the cache */
  struct
  {
	 int32_t Px[4]; 	/*!< the four P coefficients in the x-direction */
	 int32_t Qx[4]; 	/*!< the four Q coefficients in the x-direction */
	 int32_t Rx[3]; 	/*!< the three R coefficients in the x-direction */
	 int32_t Sx[2]; 	/*!< the two S coefficients in the x-direction */
	 int32_t cache_x; /*!< x-coordinate of the biginning of the cache */
	 int32_t Py[4]; 	/*!< the four P coefficients in the y-direction */
	 int32_t Qy[4]; 	/*!< the four Q coefficients in the y-direction */
	 int32_t Ry[3]; 	/*!< the three R coefficients in the y-direction */
	 int32_t Sy[2]; 	/*!< the two S coefficients in the y-direction */
	 int32_t cache_y;	/*!< y-coordinate of the beginning of the cache */
  } reg;
} cic_u;

using namespace sc_core;
using namespace std;


namespace soclib { namespace caba {
    template<typename wb_param>
      class VideoCalc:sc_module
    {
      public:

      sc_core::sc_in<bool> p_clk; /*!< System clock (100 MHz) */
      sc_core::sc_in<bool> p_resetn; /*!< Reset signal */
      sc_out<bool> p_interrupt; /*!< To indicates that an image has been processed */

      soclib::caba::WbMaster<wb_param> p_wb_read;
      soclib::caba::WbMaster<wb_param> p_wb_write;


		/*!
		 * \brief Constructor
		 *
		 * VideoCalc constructor
		 *
		 * \param tab: the link with the processor
		 * \param w: image width (640 by default)
		 * \param h: image height (480 by default)
		 * */
		VideoCalc (sc_core::sc_module_name insname,
			 uint32_t * tab,
			 const int w = 640,
			 const int h = 480
			 );

		/*!
		 * \brief To load the cache
		 *
		 * Wait information from the processor, overwrite coefficients 
		 * sent by the processor in its own array and fill the cache. 
		 */
      void get_tile();

		/*!
		 * \brief Process a tile
		 *
		 * Incremental computation and bilinear interpolation in fixed point representation.
		 * Write in a fifo the good intensity.
		 */
      void process_tile();

		/*!
		 * \brief Store a processed tile in RAM
		 *
		 * read intensities from the fifo and write them in RAM
		 */ 
      void store_tile();

		/*!
		 * \brief Fill the cache
		 *
		 * \param deb_im_in: image start address
		 */
      void fill_cache(uint32_t deb_im_in);

		/*!
		 * \brief Coefficient initialization
		 * 
		 * Read in RAM the coefficients sent by the processor
		 */
      void init_coeff();

		/*!
		 * \brief Compute the multiplication between two int32_t which are interpretated as fixed point (16.16)
		 *
		 * \param A: first fixed point
		 * \param B: second fixed point
		 */
		int32_t fx_mul(int32_t A, int32_t B);

      private:

      const uint32_t p_WIDTH ;
		/*!< Image width */
      const uint32_t p_HEIGHT ;
		/*!< Image height */

      sc_fifo<uint8_t> fifo;
		/*!< Fifo which contains pixels to be stored in RAM */

      uint32_t deb_im_in;
		/*!< The address of the image which will be processed */
      uint32_t deb_im_out;
		/*!< The address where will be stored the processed image */

      uint32_t * wb_tab;
		/*!< To communicate with the processor */

      uint8_t cache[C_H][C_W];
		/*!< The cache */

      int16_t cache_x;
		/*!< x-coordinate of the biginning of the cache (sent by the processor)*/
      int16_t cache_y;
		/*!< y-coordinate of the biginning of the cache (sent by the processor)*/

      bool ask_cache;
		/*!< Process_tile asks to Get_Tile to fill the cache */

      bool cache_rdy;
		/*!< The cache is loaded */

      cic_u coeff[T_NB];
		/*!< array containing coefficients of each tile */
      cic_u coeff_image[T_NB];
		/*!< Copy of the coefficients array */

      WbMasterModule<wb_param> master0;
		/*!< wishbone master to read in RAM */

      WbMasterModule<wb_param> master1;
		/*!< wishbone master to write in RAM */

      protected:
      SC_HAS_PROCESS(VideoCalc);

    };
  }}
#endif //VIDEO_CALC_H




