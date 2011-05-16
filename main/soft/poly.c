#include "poly.h"

int coeff[2][10] = {{1, 1, 1, 1, 1,
  							1, 1, 1, 1, 1},
	 					  {1, 1, 1, 1, 1,
							1, 1, 1, 1, 1}};




void init_poly()
{
  int X, Y;
  int X_p, Y_p;
  for(Y = 0; Y < NB_TILE_HEIGHT; Y++)
  {
	 Y_p = Y * TILE_WIDTH;

	 for(X = 0; Y < NB_TILE_WIDTH; X++)
	 {

		X_p = X * TILE_HEIGHT;

		coeff_incr_array[Y][X].P0 = 	6 * coeff[0][0];
		coeff_incr_array[Y][X].Q0 = 	6 * coeff[0][3];
		coeff_incr_array[Y][X].Q1 = 	2 * coeff[0][2] * X_p +
		  										6 * coeff[0][3] * Y_p +
												6 * coeff[0][3] +
												2 * coeff[0][6];
		coeff_incr_array[Y][X].Q2 =	coeff[0][1] * X_p * X_p +
		  										2 * coeff[0][2] * X_p * Y_p +
												3 * coeff[0][3] * Y_p * Y_p +
												(coeff[0][2] + coeff[0][5]) * X_p +
												(3 * coeff[0][3] + 2 * coeff[0][6]) * Y_p +
												coeff[0][3] + coeff[0][6] + coeff[0][8];
		coeff_incr_array[Y][X].Q3 = 0;
		coeff_incr_array[Y][X].R0 = 0;
		coeff_incr_array[Y][X].R1 = 0;
		coeff_incr_array[Y][X].R2 = 0;
		coeff_incr_array[Y][X].S0 = 0;
		coeff_incr_array[Y][X].S1 = 0;


	 }

  } 

}
