#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>

#define WIDTH 640
#define HEIGHT 480

#define NB_MAX_IMAGES 4

#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define NB_TILE_WIDTH WIDTH/TILE_WIDTH
#define NB_TILE_HEIGHT HEIGHT/TILE_HEIGHT
#define NB_TILE NB_TILE_WIDTH*NB_TILE_HEIGHT

#define RAM_FIRST_IMAGE 				RAM_BASE
#define RAM_FIRST_IMAGE_PROCESSED 	(RAM_BASE + 5*640*480/4)

typedef uint32_t image_t[HEIGHT][WIDTH/4];

typedef struct coeff_incr_t {
  float P0;
  float P1;
  float P2;
  float P3;
  float Q0;
  float Q1;
  float Q2;
  float Q3;
  float R0;
  float R1;
  float R2;
  float S0;
  float S1;
} COEFF_INCR;


#endif
