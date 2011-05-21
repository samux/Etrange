#include "irq_handler.h"

/**
 * When we receive an interruption from video_out, we send
 * the address of a new image processed
 */
void video_out_handler()
{
  /*if(nb_image_processed > nb_image_out && nb_image_processed - nb_image_out < 3)*/
	 /*nb_image_out++;*/

  /*VIDEO_OUT = (uint32_t)&images_processed[nb_image_out%NB_MAX_IMAGES];*/

}


/**
 * When we receive an interruption from video_in, we send 
 * the address where will be stored the next image
 */
void video_in_handler()
{
  //The difference between nb_image and nb_image_processed
  //must be < than 2 to avoid owerwrting
  /*if(nb_image - nb_image_processed < 2)*/
	 /*nb_image++;*/

  /*VIDEO_IN = (uint32_t)&images[(nb_image)%NB_MAX_IMAGES];*/
  /*if(first_image)*/
  /*{*/
	 /*CALC_HARD = (uint32_t)&images[0];*/
	 /*CALC_HARD = (uint32_t)&images_processed[0];*/
	 /*CALC_HARD = (uint32_t)&coeff_incr_array[0][0][0];*/

	 /*first_image = 0;*/
  /*}*/
}


/**
 * When we received an interruption from calc_hard, we send
 * to this module: 
 * 	- the address where the next image will be read
 * 	- the address where the next image processed will be stored
 * 	- the address of coeff
 */
void calc_hard_handler()
{
  /*nb_image_processed++;*/
//  We send the address of the new image to be read
  /*CALC_HARD = (uint32_t)&images[nb_image_processed%NB_MAX_IMAGES];*/
  //We send the address of the new image to be stored
  /*CALC_HARD = (uint32_t)&images_processed[nb_image_processed%NB_MAX_IMAGES];*/
  //We send the address of the coeff
  /*CALC_HARD = (uint32_t)&coeff_incr_array[0][0][0];*/

  /*if(first_image_processed)*/
  /*{*/
	 /*VIDEO_OUT = (uint32_t)&images_processed[0];*/
	 /*first_image_processed = 0;*/
  /*}*/

}
