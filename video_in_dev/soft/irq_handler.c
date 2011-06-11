#include "irq_handler.h"
#include "lm32_sys.h"
#include "stdio.h"

/**
 * When we receive an interruption from video_in, we send
 * the address where will be stored the next image
 */
void video_in_handler()
{

  printf("on entre dans vin_handler\n");
  printf("nb_img_in : %d nb_image_processed : %d\n",(int)nb_image_in,(int)nb_image_processed); 
  if (nb_image_in - nb_image_processed < 2)
   {
     printf(" Coucou de VIN handler : %ld\n", (uint32_t) RAM_FIRST_IMAGE + (nb_image_in % NB_MAX_IMAGES) * 640 * 480);
     nb_image_in++;
   }

   VIN = (uint32_t) RAM_FIRST_IMAGE + (nb_image_in % NB_MAX_IMAGES) * 640 * 480;
   VIN_CRL = 1;

   if (first_image)
   {
     first_image = 0;
     VCALC_IN = (uint32_t) RAM_FIRST_IMAGE;
     VCALC_IN_CRL = 1;
     VCALC_OUT = (uint32_t) RAM_FIRST_IMAGE_PROCESSED;
     VCALC_OUT_CRL = 1;
   }

}

/**
 * When we receive an interruption from video_calc,
 * we send the address where :
 *   - is stored the image to read
 *   - will be stored the image processed
 */
void video_calc_handler()
{
  nb_image_processed++;
  printf(" Coucou de VCALC handler : %ld \n",(uint32_t) RAM_FIRST_IMAGE + (nb_image_processed % NB_MAX_IMAGES) * 640 * 480 );

  //We send the address of the new image to be read
  VCALC_IN = (uint32_t) RAM_FIRST_IMAGE + (nb_image_processed % NB_MAX_IMAGES) * 640 * 480;
  VCALC_IN_CRL = 1;

  //We send the address of the new image to be stored
  VCALC_OUT = (uint32_t) RAM_FIRST_IMAGE_PROCESSED + (nb_image_processed % NB_MAX_IMAGES) * 640 * 480;
  VCALC_OUT_CRL = 1;

  if(first_image_processed)
  {
    first_image_processed = 0;
    VOUT = (uint32_t) RAM_FIRST_IMAGE_PROCESSED;
    VOUT_CRL = 1;
  }

}

/**
 * When we receive an interruption from video_out, we send
 * the address of a new image processed
 */
void video_out_handler()
{
  printf("on entre dans vout_handler\n");
  printf("nb_image_out : %d nb_image_processed : %d\n",(int)nb_image_out, (int)nb_image_processed);

  if((nb_image_processed > nb_image_out) && (nb_image_processed - nb_image_out < 3))
  {
    printf(" Coucou de VOUT handler : %ld \n", (uint32_t) RAM_FIRST_IMAGE_PROCESSED + (nb_image_out % NB_MAX_IMAGES) * 640 * 480);
    nb_image_out++;
  }

  /* VOUT = (uint32_t) RAM_FIRST_IMAGE + nb_image_out * 640 * 480; */

  VOUT = (uint32_t) RAM_FIRST_IMAGE_PROCESSED + (nb_image_out % NB_MAX_IMAGES) * 640 * 480;
  VOUT_CRL = 1;
}

void tty_handler()
{
  char c = inbyte();
  printf("char received %c\n",c);

}

