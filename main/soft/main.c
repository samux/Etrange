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
 * Copyright (c) UPMC, Lip6, SoC
 *         Nicolas Pouillon <nipo@ssji.net>, 2006-2007
 *
 * Maintainers: tarik.graba@telecom-paristech.fr
 */


#include <stdio.h>
#include "lm32_sys.h"
#include "irq_handler.h"
#include "utils.h"
#include "poly.h"
#include <stdint.h>

image_t images[NB_MAX_IMAGES] = {{{0}}};
image_t images_processed[NB_MAX_IMAGES] = {{{0}}};
COEFF_INCR coeff_incr_array[2][NB_TILE_HEIGHT][NB_TILE_WIDTH];

volatile uint32_t nb_image;
volatile uint32_t nb_image_processed;
volatile uint32_t nb_image_out;
uint8_t first_image;
uint8_t first_image_processed;


int main(void)
{
  irq_enable();

  RegisterIrqEntry(1, &video_out_handler);
  RegisterIrqEntry(2, &video_in_handler);
  RegisterIrqEntry(3, &calc_hard_handler);

  nb_image = 0;
  nb_image_processed = 0;
  nb_image_out = 0;
  first_image = 1;
  first_image_processed = 1;

  init_poly();

  //First address to store the image
  VIDEO_IN = (uint32_t)&images[0];

  while(1);

  return 0;
}
