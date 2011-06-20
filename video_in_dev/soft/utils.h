/**
 * \file utils.h
 * \brief Contains all useful data
 * \author Samuel Mokrani
 * \date 20/06/2011
 *
 * This file contains all useful parameters concerning tiles, images, coefficients
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include "mfixed.h"

#define WIDTH 640 										/**< Width of images   */
#define HEIGHT 480 										/**< Height of images   */

#define NB_BYTE_IN_A_WORD 4							/**< Number of bytes in a word @ hideinitializer */


#define NB_MAX_IMAGES 8 								/**< Number maximum of images in RAM   */

#define TILE_WIDTH 16 									/**< Width of tiles   */
#define TILE_HEIGHT 16 									/**< Height of images   */
#define NB_TILE_WIDTH WIDTH/TILE_WIDTH 			/**< Number of tiles according to the width   */
#define NB_TILE_HEIGHT HEIGHT/TILE_HEIGHT 		/**< Number of tiles according to the height   */
#define NB_TILE NB_TILE_WIDTH*NB_TILE_HEIGHT 	/**< Number of tiles   */

/**
 * \struct coeff_incr_t
 * \brief All coefficients for incremental computation
 *
 * This structure contains all coefficients for incremental computation
 */
typedef struct coeff_incr_t {
  mfixed P0;/*!< P0 coefficient */
  mfixed P1;/*!< P1 coefficient */
  mfixed P2;/*!< P2 coefficient */
  mfixed P3;/*!< P3 coefficient */
  mfixed Q0;/*!< Q0 coefficient */
  mfixed Q1;/*!< Q1 coefficient */
  mfixed Q2;/*!< Q2 coefficient */
  mfixed Q3;/*!< Q3 coefficient */
  mfixed R0;/*!< R0 coefficient */
  mfixed R1;/*!< R1 coefficient */
  mfixed R2;/*!< R2 coefficient */
  mfixed S0;/*!< S0 coefficient */
  mfixed S1;/*!< S1 coefficient */
} COEFF_INCR;


#endif
