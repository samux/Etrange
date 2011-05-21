#include "poly.h"
#include "stdio.h"

int coeff_x[4][4] = { {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1} };
int coeff_y[4][4] = { {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1} };


static int poly_3(int X, int Y, int a[4][4]);

static int poly_3(int X, int Y, int coeff[4][4])
{
  return 	coeff[3][0]*X*X*X + coeff[0][3]*Y*Y*Y + 
	 			coeff[2][1]*X*X*Y + coeff[1][2]*X*Y*Y + 
	 			coeff[2][0]*X*X + coeff[1][1]*X*Y + 
	 			coeff[0][2]*Y*Y + coeff[1][0]*X + 
	 			coeff[0][1]*Y + coeff[0][0];
}


void init_poly()
{
  int X, Y;
  int X_p, Y_p;
  for(Y = 0; Y < NB_TILE_HEIGHT; Y++)
  {
	 Y_p = Y * TILE_WIDTH;

	 for(X = 0; X < NB_TILE_WIDTH; X++)
	 {

		X_p = X * TILE_HEIGHT;

		//x coefficient
		//P
		coeff_incr_array[0][Y][X].P0 = 	6 * coeff_x[3][0];
		coeff_incr_array[0][Y][X].P1 = 	6 * coeff_x[3][0] * (X_p + 1) + 
		  											2 * coeff_x[2][1] * Y_p + 
		  											2 * coeff_x[2][0];
		coeff_incr_array[0][Y][X].P2 = 	coeff_x[3][0] * (3 * X_p * X_p + 3 * X_p + 1) + 
		  											coeff_x[2][0] * (1 + 2 * X_p) + 
		  											coeff_x[2][1] * (2 * X_p * Y_p + Y_p) + 
													coeff_x[1][2] * Y_p * Y_p + 
		  											coeff_x[1][1] * Y_p + 
													coeff_x[1][0];
		coeff_incr_array[0][Y][X].P3 = 	poly_3(X_p,Y_p,coeff_x);

		//Q
		coeff_incr_array[0][Y][X].Q0 = 	6 * coeff_x[0][3];
		coeff_incr_array[0][Y][X].Q1 = 	2 * coeff_x[1][2] * X + 
		  											6 * coeff_x[0][3] * (Y + 1) + 
													2 * coeff_x[0][2];
		coeff_incr_array[0][Y][X].Q2 =  	coeff_x[2][1] * X * X + 
		  											2 * coeff_x[1][2] * X * Y + 
													3 * coeff_x[0][3] * Y * Y + 
		  											(coeff_x[1][2] + coeff_x[1][1]) * X + 
													(3 * coeff_x[0][3] + 2 * coeff_x[0][2]) * Y + 
													coeff_x[0][3] + coeff_x[0][2] + coeff_x[0][1];
		coeff_incr_array[0][Y][X].Q3 = 	coeff_incr_array[0][Y][X].P3;


		//R
		coeff_incr_array[0][Y][X].R0 = 	2 * coeff_x[1][2];
		coeff_incr_array[0][Y][X].R1 = 	coeff_x[2][1] * (2 * X + 1) + 
		  											coeff_x[1][2] * (2 * Y + 1) + 
													coeff_x[1][1];
		coeff_incr_array[0][Y][X].R2 = 	coeff_incr_array[0][Y][X].P2;

		//S
		coeff_incr_array[0][Y][X].S0 = 	2 * coeff_x[2][1];
		coeff_incr_array[0][Y][X].S1 = 	coeff_incr_array[0][Y][X].P1;


		//y coefficient
		//P
		coeff_incr_array[1][Y][X].P0 = 	6 * coeff_y[3][0];
		coeff_incr_array[1][Y][X].P1 = 	6 * coeff_y[3][0] * (X_p + 1) + 
		  											2 * coeff_y[2][1] * Y_p + 
		  											2 * coeff_y[2][0];
		coeff_incr_array[1][Y][X].P2 = 	coeff_y[3][0] * (3 * X_p * X_p + 3 * X_p + 1) + 
		  											coeff_y[2][0] * (1 + 2 * X_p) + 
		  											coeff_y[2][1] * (2 * X_p * Y_p + Y_p) + 
													coeff_y[1][2] * Y_p * Y_p + 
		  											coeff_y[1][1] * Y_p + 
													coeff_y[1][0];
		coeff_incr_array[1][Y][X].P3 = 	poly_3(X_p,Y_p,coeff_y);

		//Q
		coeff_incr_array[1][Y][X].Q0 = 	6 * coeff_y[0][3];
		coeff_incr_array[1][Y][X].Q1 = 	2 * coeff_y[1][2] * X + 
		  											6 * coeff_y[0][3] * (Y + 1) + 
													2 * coeff_y[0][2];
		coeff_incr_array[1][Y][X].Q2 =  	coeff_y[2][1] * X * X + 
		  											2 * coeff_y[1][2] * X * Y + 
													3 * coeff_y[0][3] * Y * Y + 
		  											(coeff_y[1][2] + coeff_y[1][1]) * X + 
													(3 * coeff_y[0][3] + 2 * coeff_y[0][2]) * Y + 
													coeff_y[0][3] + coeff_y[0][2] + coeff_y[0][1];
		coeff_incr_array[1][Y][X].Q3 = 	coeff_incr_array[0][Y][X].P3;


		//R
		coeff_incr_array[1][Y][X].R0 = 	2 * coeff_y[1][2];
		coeff_incr_array[1][Y][X].R1 = 	coeff_y[2][1] * (2 * X + 1) + 
		  											coeff_y[1][2] * (2 * Y + 1) + 
													coeff_y[1][1];
		coeff_incr_array[1][Y][X].R2 = 	coeff_incr_array[0][Y][X].P2;

		//S
		coeff_incr_array[1][Y][X].S0 = 	2 * coeff_y[2][1];
		coeff_incr_array[1][Y][X].S1 = 	coeff_incr_array[0][Y][X].P1;


	 }

  } 

}
