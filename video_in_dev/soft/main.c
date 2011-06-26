
/**
 * \file main.c
 * \brief Main procedure
 * \author Samuel Mokrani
 * \date 20/06/2011
 *
 * This file contains the main procedure: it computes all initialization coefficients for incremental computations.
 * After this, it sends to V_IN the address where will be stored the first image. And that's all. After, there are just
 * interrupt handlers which maintain the good functionning of the system.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "lm32_sys.h"
#include "irq_handler.h"
#include "poly.h"
#include "utils.h"
#include "../segmentation.h"
#include <stdint.h>

extern char inbyte();

/** number of images stored by V_IN */
volatile uint32_t nb_image_in;
/** number of images read by V_OUT */
volatile uint32_t nb_image_out;

/**  
 * To indicate if we have already processed a picture coming from V_IN.
 * Useful to send to V_CALC the signal that the first picture is ready
 * */
uint8_t first_image_processed;

/**  
 * To indicate if this is the first image stored by V_IN
 * Useful to send to V_OUT the signal that the first image is ready (if USE_COPRO = 0).
 * */
uint8_t first_image;

/**  Contains all coefficients of each tile */
COEFF_INCR coeff_incr_array[2][NB_TILE_HEIGHT][NB_TILE_WIDTH];

/** Coefficients for the transformation in x-direction */

mfixed coeff_x[4][4] = { 	{(mfixed)0,(mfixed)0 , (mfixed)(2<<4), (mfixed)0},
                                {(mfixed)(1<<16), (mfixed)0, (mfixed)0, (mfixed)0},
                                {(mfixed)(2<<4), (mfixed)0, (mfixed)0, (mfixed)0},
                                {(mfixed)0, (mfixed)0, (mfixed)0, (mfixed)0}
};
/** Coefficients for the transformation in y-direction */
mfixed coeff_y[4][4] = { 	{(mfixed)0, (mfixed)(1<<16), (mfixed)(2<<4), (mfixed)0},
                                {(mfixed)0, (mfixed)0, (mfixed)0, (mfixed)0},
                                {(mfixed)(2<<4), (mfixed)0, (mfixed)0, (mfixed)0},
                                {(mfixed)0, (mfixed)0, (mfixed)0, (mfixed)0}
};





/** The address of the first image stored in RAM */
uint32_t * RAM_FIRST_IMAGE;
/** The address of the first image processed stored in RAM */
uint32_t * RAM_FIRST_IMAGE_PROCESSED;

int main(void)
{
  printf("Bonjour du LM32\n");

#if USE_COPRO
  printf("on utilise le coprocesseur\n");
  mfixed cos_plus;
  cos_plus.h = 1;
  cos_plus.l =  (1<<14);
  mfixed sin_moins;
  sin_moins.h = -1;
  sin_moins.l = 0;
  mfixed sin_plus;
  sin_plus.h = 1;
  sin_plus.l = 0;

  mfixed decalage_x;
  decalage_x.h = -20*16;
  decalage_x.l = 0;
  coeff_x[0][0] = decalage_x;

  mfixed decalage_y;
  decalage_y.h = 4*16;
  decalage_y.l = 0;
  coeff_y[0][0] = decalage_y;

  coeff_x[1][0] = cos_plus;
  coeff_x[0][1] = sin_plus;

  coeff_y[0][1] = cos_plus;
  coeff_y[1][0] = sin_moins;


  init_poly();
  //print_poly();
  printf("Coeff OK\n");
#else
  printf("on n'utilise pas le copro\n");
#endif

  irq_enable();

  RegisterIrqEntry(0, &tty_handler);
  RegisterIrqEntry(1, &video_in_handler);
  RegisterIrqEntry(2, &video_out_handler);
  RegisterIrqEntry(3, &video_calc_handler);

  nb_image_in = 0;
  nb_image_out = 0;

  first_image_processed = 1;
  first_image = 1;

  RAM_FIRST_IMAGE = (uint32_t *) malloc( 10 * sizeof(uint32_t) * WIDTH * HEIGHT / NB_BYTE_IN_A_WORD );
  RAM_FIRST_IMAGE_PROCESSED = (uint32_t *) malloc(10 * sizeof(uint32_t) * WIDTH * HEIGHT / NB_BYTE_IN_A_WORD);

  //First address to store the image
  VIN = (uint32_t) RAM_FIRST_IMAGE;
  VIN_CRL = 1;

  while(1);

  return 0;
}
