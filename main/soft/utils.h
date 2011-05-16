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

typedef uint32_t image_t[HEIGHT][WIDTH/4];

typedef struct coeff_incr_t {
  uint32_t P0;
  uint32_t Q0;
  uint32_t Q1;
  uint32_t Q2;
  uint32_t Q3;
  uint32_t R0;
  uint32_t R1;
  uint32_t R2;
  uint32_t S0;
  uint32_t S1;
} COEFF_INCR;


#endif
