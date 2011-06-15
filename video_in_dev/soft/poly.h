#ifndef _POLY_H_
#define _POLY_H_

#include "utils.h"

extern COEFF_INCR coeff_incr_array[2][NB_TILE_HEIGHT][NB_TILE_WIDTH];

extern int coeff_x[4][4];
extern int coeff_y[4][4];

void init_poly();
void ante(int X_, int Y_, int * X_ante, int * Y_ante);



#endif
