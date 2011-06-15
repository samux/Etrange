#include "irq_handler.h"
#include "lm32_sys.h"
#include "stdio.h"

/**
 * When we receive an interruption from video_in, we send:
 * 	- the address where will be stored the next image for VIN
 * 	- knowing that VCALC is very fast, we send also VCALC_IN and VCALC_OUT
 */
void video_in_handler()
{

  if (nb_image_in - nb_image_out < 2)
  {
	 nb_image_in++;
	 printf(" Coucou de VIN handler : %ld\n", (uint32_t) RAM_FIRST_IMAGE + (nb_image_in % NB_MAX_IMAGES) * 640 * 480);
  }

  VIN = (uint32_t) RAM_FIRST_IMAGE + (nb_image_in % NB_MAX_IMAGES) * 640 * 480;
  VIN_CRL = 1;

  VCALC_IN = (uint32_t) RAM_FIRST_IMAGE + ((nb_image_in-1) % NB_MAX_IMAGES) * 640 * 480;
  VCALC_IN_CRL = 1;
  VCALC_OUT = (uint32_t) RAM_FIRST_IMAGE_PROCESSED +((nb_image_in-1) % NB_MAX_IMAGES) * 640 * 480;
  VCALC_OUT_CRL = 1;
  VCALC_POLY = (uint32_t) &coeff_incr_array;
}

/**
 * When we receive an interruption from video_calc,
 * 	It's only an handler to initialize VOUT for the first image
 */
void video_calc_handler()
{
  if(first_image_processed)
  {
    first_image_processed = 0;
    VOUT = (uint32_t) RAM_FIRST_IMAGE_PROCESSED;
    VOUT_CRL = 1;
  }

}

/**
 * When we receive an interruption from video_out, we send:
 * 	- the address of a new image processed
 */
void video_out_handler()
{
  if(nb_image_out < nb_image_in - 1)
  {
	 printf(" Coucou de VOUT handler : %ld \n", (uint32_t) RAM_FIRST_IMAGE_PROCESSED + (nb_image_out % NB_MAX_IMAGES) * 640 * 480);
	 nb_image_out++;
  }

  //VOUT = (uint32_t) RAM_FIRST_IMAGE + (nb_image_out % NB_MAX_IMAGES) * 640 * 480;

  VOUT = (uint32_t) RAM_FIRST_IMAGE_PROCESSED + (nb_image_out % NB_MAX_IMAGES) * 640 * 480;
  VOUT_CRL = 1;
}

void tty_handler()
{
  char c = inbyte();
  printf("char received %c\n",c);

}

