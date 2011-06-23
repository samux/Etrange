/**
 * \file poly.h
 * \brief Contains all functions concerning polynomes and antecedents
 * \author Samuel Mokrani
 * \date 20/06/2011
 *
 */
#ifndef _POLY_H_
#define _POLY_H_

#include "utils.h"

extern COEFF_INCR coeff_incr_array[2][NB_TILE_HEIGHT][NB_TILE_WIDTH];

extern mfixed coeff_x[4][4];
extern mfixed coeff_y[4][4];

/**
 * \fn void init_poly()
 * \brief Coefficients initialization
 *
 * Compute the coefficients initialization in fixed point
 */
void init_poly();

/**
 * \fn void ante(int X, int Y, mfixed * X_ante, m_fixed * Y_ante)
 * \brief Compute the antecedant of a pixel
 *
 * Compute the antecedant of a pixel and store it in two mfixed
 */
void ante(float X_, float Y_, mfixed * X_ante, mfixed * Y_ante);

/************
 * Pour le debug
 * *********/
/**
 * \fn void print_poly()
 * \brief Print coefficients
 *
 * Warning: Only for debug
 * Print the coefficients
 */
void print_poly();



#endif
