#ifndef _POLY_H_
#define _POLY_H_

#include "utils.h"

extern COEFF_INCR coeff_incr_array[2][NB_TILE_HEIGHT][NB_TILE_WIDTH];

extern mfixed coeff_x[4][4];
extern mfixed coeff_y[4][4];

void init_poly();
void ante(int X_, int Y_, mfixed * X_ante, mfixed * Y_ante);

/************
 * Pour le debug
 * *********/
void print_poly();



#endif
