#include "irq_handler.h"
#include "stdio.h"

/**
 * When we receive an interruption from video_out, we send
 * the address of a new image processed
 */
void video_out_handler()
{
  printf("&&&&& Coucou de VOUT handler\n");
  //if(nb_image_processed > nb_image_out && nb_image_processed - nb_image_out < 3)
  nb_image_out++;

  //VOUT = (uint32_t)&images_processed[nb_image_out%NB_MAX_IMAGES];
  VOUT = RAM_FIRST_IMAGE + (nb_image_out%NB_MAX_IMAGES) * 640 * 480;
  VOUT_CRL = 1;
}


/**
 * When we receive an interruption from video_in, we send 
 * the address where will be stored the next image
 */
void video_in_handler()
{
  printf("coucou de VIN handler\n");
  //The difference between nb_image and nb_image_processed
  //must be < than 2 to avoid owerwrting
  //if(nb_image - nb_image_processed < 2)
  nb_image_in++;

  //VIN = (uint32_t)&images[(nb_image)%NB_MAX_IMAGES];
  VIN = RAM_FIRST_IMAGE + (nb_image_in%NB_MAX_IMAGES) * 640 * 480;
  VIN_CRL = 1;
  if(first_image)
  {
	 //HARD = (uint32_t)&images[0];
	 //HARD = (uint32_t)&images_processed[0];
	 //HARD = RAM_FIRST_IMAGE;
	 //HARD = RAM_FIRST_IMAGE_PROCESSED;
	 //HARD = (uint32_t)&coeff_incr_array[0][0][0];

	 first_image = 0;
	 VOUT = RAM_FIRST_IMAGE;
	 VOUT_CRL = 1;
  }
}


/**
 * When we received an interruption from calc_hard, we send
 * to this module: 
 * 	- the address where the next image will be read
 * 	- the address where the next image processed will be stored
 * 	- the address of coeff
 */
void video_calc_read_handler()
{
  nb_image_processed_in++;
  //We send the address of the new image to be read
  //HARD = (uint32_t)&images[nb_image_processed%NB_MAX_IMAGES];
  //HARD = RAM_FIRST_IMAGE + (nb_image_processed%NB_MAX_IMAGES) * 640 * 480 / 4;
  //We send the address of the new image to be stored
  //HARD = (uint32_t)&images_processed[nb_image_processed%NB_MAX_IMAGES];
  //HARD = RAM_FIRST_IMAGE_PROCESSED + (nb_image_processed%NB_MAX_IMAGES) * 640 * 480 / 4;
  //We send the address of the coeff
  //HARD = (uint32_t)&coeff_incr_array[0][0][0];
  printf("Coucou de video_calc_in_handler");
  if(first_image_processed_in)
  {
	 //VOUT = (uint32_t)&images_processed[0];
	 VCALC_R = RAM_FIRST_IMAGE;
	 VCALC_R_CRL = 1;
	 first_image_processed_in = 0;
  }
}

void video_calc_write_handler()
{
  nb_image_processed_out++;
  //We send the address of the new image to be read
  //HARD = (uint32_t)&images[nb_image_processed%NB_MAX_IMAGES];
  //HARD = RAM_FIRST_IMAGE + (nb_image_processed%NB_MAX_IMAGES) * 640 * 480 / 4;
  //We send the address of the new image to be stored
  //HARD = (uint32_t)&images_processed[nb_image_processed%NB_MAX_IMAGES];
  //HARD = RAM_FIRST_IMAGE_PROCESSED + (nb_image_processed%NB_MAX_IMAGES) * 640 * 480 / 4;
  //We send the address of the coeff
  //HARD = (uint32_t)&coeff_incr_array[0][0][0];

  printf("Coucou de video_calc_write_handler");
  if(first_image_processed_out)
  {
	 //VOUT = (uint32_t)&images_processed[0];
	 //VOUT = RAM_FIRST_IMAGE_PROCESSED;
	 //VOUT_CRL = 1;
	 first_image_processed_out = 0;
  }
}

