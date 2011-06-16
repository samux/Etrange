#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include "mfixed.h"

#define WIDTH 640
#define HEIGHT 480

#define NB_MAX_IMAGES 8

#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define NB_TILE_WIDTH WIDTH/TILE_WIDTH
#define NB_TILE_HEIGHT HEIGHT/TILE_HEIGHT
#define NB_TILE NB_TILE_WIDTH*NB_TILE_HEIGHT

typedef struct coeff_incr_t {
  mfixed P0;
  mfixed P1;
  mfixed P2;
  mfixed P3;
  mfixed Q0;
  mfixed Q1;
  mfixed Q2;
  mfixed Q3;
  mfixed R0;
  mfixed R1;
  mfixed R2;
  mfixed S0;
  mfixed S1;
} COEFF_INCR;


#endif
