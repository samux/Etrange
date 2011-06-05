/*
 *
 * SOCLIB_GPL_HEADER_BEGIN
 *
 * This file is part of SoCLib, GNU GPLv2.
 *
 * SoCLib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * SOCLIB_GPL_HEADER_END
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

volatile uint32_t nb_image_in;
volatile uint32_t nb_image_processed;
volatile uint32_t nb_image_out;
uint8_t first_image;
uint8_t first_image_processed;

COEFF_INCR coeff_incr_array[2][NB_TILE_HEIGHT][NB_TILE_WIDTH];

int coeff_x[4][4] = { {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1} };
int coeff_y[4][4] = { {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1} };

uint32_t * RAM_FIRST_IMAGE;
uint32_t * RAM_FIRST_IMAGE_PROCESSED;

int main(void)
{

  char c;

  printf("Bonjour du LM32\n");
  init_poly();
  printf("Coeff OK\n");

  irq_enable();

  RegisterIrqEntry(1, &video_in_handler);
  RegisterIrqEntry(2, &video_out_handler);
  RegisterIrqEntry(3, &video_calc_handler);
  RegisterIrqEntry(0, &tty_handler);

  nb_image_in = 0;
  nb_image_processed = 0;
  nb_image_out = 0;

  first_image = 1;
  first_image_processed = 1;

  RAM_FIRST_IMAGE = (uint32_t *) malloc( 5 * sizeof(uint32_t) * 640 * 480 / 4);
  RAM_FIRST_IMAGE_PROCESSED = (uint32_t *)malloc(5 * sizeof(uint32_t) * 640 * 480 / 4);

  //First address to store the image
  VIN = (uint32_t) RAM_FIRST_IMAGE;
  VIN_CRL = 1;

  while(1);
  {
	 printf("test getchar\n");
	 switch(c = inbyte())
	 {
		case '/':
		  break;
		case '*':
		  break;
		case '+':
		  break;
		case '-':
		  break;
		default:
		  break;
	 }
  }


  while(1);


  return 0;
}
