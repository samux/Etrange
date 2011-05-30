#include "irq_handler.h"
#include "lm32_sys.h"
#include "stdio.h"

unsigned long begin_cc = 0;
unsigned long end_cc = 0;

/**
 * When we receive an interruption from video_out, we send
 * the address of a new image processed
 */
void video_out_handler()
{
  printf("&&&&& Coucou de VOUT handler\n");
  end_cc = get_cc();
  //if(nb_image_processed > nb_image_out && nb_image_processed - nb_image_out < 3)
  nb_image_out++;

  //VOUT = (uint32_t)&images_processed[nb_image_out%NB_MAX_IMAGES];
  VOUT = (uint32_t)RAM_FIRST_IMAGE + (nb_image_out%NB_MAX_IMAGES) * 640 * 480;
  VOUT_CRL = 1;
  begin_cc = get_cc();
  printf("freq = %ld Hz\n", 1000000000/(begin_cc - end_cc));
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
  //if(nb_image_in - nb_image_processed < 2)
	 nb_image_in++;

  //VIN = (uint32_t)&images[(nb_image)%NB_MAX_IMAGES];
  VIN = (uint32_t)RAM_FIRST_IMAGE + (nb_image_in%NB_MAX_IMAGES) * 640 * 480;
  VIN_CRL = 1;
  if(first_image)
  {
	 first_image = 0;
	 VOUT = (uint32_t)RAM_FIRST_IMAGE;
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
void video_calc_handler()
{
  printf("Coucou de video_calc_in_handler");

  nb_image_processed++;
  //We send the address of the new image to be read
  VCALC = (uint32_t)RAM_FIRST_IMAGE + (nb_image_processed%NB_MAX_IMAGES) * 640 * 480 / 4;
  //We send the address of the new image to be stored
  VCALC = (uint32_t)RAM_FIRST_IMAGE_PROCESSED + (nb_image_processed%NB_MAX_IMAGES) * 640 * 480 / 4;
  if(first_image_processed)
  {
	 //VOUT = (uint32_t)&images_processed[0];
	 VOUT = (uint32_t)RAM_FIRST_IMAGE_PROCESSED;
	 VOUT_CRL = 1;
	 first_image_processed = 0;
  }
}



void tty_handler()
{
  char c = inbyte();
  printf("char received %c\n",c);

}

