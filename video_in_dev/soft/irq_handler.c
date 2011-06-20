/**
 * \file irq_handler.c
 * \brief Contains interrupt handlers
 * \author Samuel Mokrani
 * \date 20/06/2011
 *
 * This file contains four interrupt handlers : 
 * 	video_in_handler
 * 	video_out_handler
 * 	video_calc_handler
 * 	tty_handler
 *
 */
#include "irq_handler.h"
#include "lm32_sys.h"
#include "stdio.h"

void video_in_handler()
{
  uint32_t addr_v_in = 0;
  uint32_t addr_v_calc_in = 0;
  uint32_t addr_v_calc_out = 0;

  if (nb_image_in - nb_image_out < 2)
  {
	 nb_image_in++;
	 addr_v_in = 				(uint32_t) RAM_FIRST_IMAGE + 
									(nb_image_in % NB_MAX_IMAGES) * WIDTH * HEIGHT;

	 addr_v_calc_in =  		(uint32_t) RAM_FIRST_IMAGE + 
									((nb_image_in-1) % NB_MAX_IMAGES) * WIDTH * HEIGHT;

	 addr_v_calc_out =  		(uint32_t) RAM_FIRST_IMAGE_PROCESSED + 
									((nb_image_in-1) % NB_MAX_IMAGES) * WIDTH * HEIGHT;
	 printf(" Coucou de VIN handler : %ld\n", addr_v_in);
  }

  VCALC_POLY = (uint32_t) &(coeff_incr_array[0][0][0].P0.all);
  VIN = addr_v_in;
  VIN_CRL = 1;

  VCALC_IN = addr_v_calc_in;
  VCALC_IN_CRL = 1;
  VCALC_OUT = addr_v_calc_out;
  VCALC_OUT_CRL = 1;
 }

void video_calc_handler()
{
  if(first_image_processed)
  {
    first_image_processed = 0;
    VOUT = (uint32_t) RAM_FIRST_IMAGE_PROCESSED;
    VOUT_CRL = 1;
  }

}

void video_out_handler()
{
  uint32_t addr_v_out = 0;
  if(nb_image_out < nb_image_in - 1)
  {
	 nb_image_out++;
	 addr_v_out = 	(uint32_t) RAM_FIRST_IMAGE_PROCESSED + 
						(nb_image_out % NB_MAX_IMAGES) * WIDTH * HEIGHT; 

	 printf(" Coucou de VOUT handler : %ld \n", addr_v_out);
  }

  VOUT = addr_v_out;
  VOUT_CRL = 1;
}

void tty_handler()
{
  char c = inbyte();
  printf("char received %c\n",c);

}

