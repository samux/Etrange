#include "poly.h"
#include "stdio.h"



static int P_0(int coeff[4][4]);
static int P_1(int X, int Y, int coeff[4][4]);
static int P_2(int X, int Y, int coeff[4][4]);
static int P_3(int X, int Y, int coeff[4][4]);

static int Q_0(int coeff[4][4]);
static int Q_1(int X, int Y, int coeff[4][4]);
static int Q_2(int X, int Y, int coeff[4][4]);

static int R_0(int coeff[4][4]);
static int R_1(int X, int Y, int coeff[4][4]);

static int S_0(int coeff[4][4]);

/***********
 * P coeff
 * ********/

static int P_0(int coeff[4][4])
{
  return 	6 * coeff[3][0];

}

static int P_1(int X, int Y, int coeff[4][4])
{
  return 	6 * coeff[3][0] * (X + 2) + 
		  		2 * coeff[2][1] * Y + 
				2 * coeff[2][0];

}

static int P_2(int X, int Y, int coeff[4][4])
{
  return 	coeff[3][0] * (3 * X * X + 3 * X + 1) + 
		  		coeff[2][0] * (1 + 2 * X) + 
				coeff[2][1] * (2 * X * Y + Y) + 
				coeff[1][2] * Y * Y + 
				coeff[1][1] * Y + 
				coeff[1][0];

}

static int P_3(int X, int Y, int coeff[4][4])
{
  return 	coeff[3][0]*X*X*X + coeff[0][3]*Y*Y*Y + 
	 			coeff[2][1]*X*X*Y + coeff[1][2]*X*Y*Y + 
	 			coeff[2][0]*X*X + coeff[1][1]*X*Y + 
	 			coeff[0][2]*Y*Y + coeff[1][0]*X + 
	 			coeff[0][1]*Y + coeff[0][0];
}

/***********
 * Q coeff
 * ********/

static int Q_0(int coeff[4][4])
{
  return 	6 * coeff_x[0][3];
}

static int Q_1(int X, int Y, int coeff[4][4])
{
  return 	2 * coeff[1][2] * X + 
		  		6 * coeff[0][3] * (Y + 1) + 
				2 * coeff[0][2];

}

static int Q_2(int X, int Y, int coeff[4][4])
{
  return 	coeff[2][1] * (X * X) + 
		  		coeff[1][2] * ( 2 * X * Y + X) + 
				coeff[0][3] * (3 * Y * Y + 3 * Y + 1) +
				coeff[1][1] * X + 
				coeff[0][2] * (2 * Y + 1) + 
				coeff[0][1]; 

}

/***********
 * R coeff
 * ********/

static int R_0(int coeff[4][4])
{
  return 	2 * coeff[1][2];

}

static int R_1(int X, int Y, int coeff[4][4])
{
  return 	coeff[2][1] * (2 * X + 1) + 
		  		coeff[1][2] * (2 * Y + 1) + 
				coeff[1][1];

}

/***********
 * S coeff
 * ********/

static int S_0(int coeff[4][4])
{
  return 	2 * coeff[2][1];

}

/****************************************
 * This function computes the antecedant
 * of (X,Y) and put it in (X_ante,Y_ante)
 * **************************************/
void ante(int X_, int Y_, int * X_ante, int * Y_ante)
{
  * X_ante = P_3(X_, Y_, coeff_x);
  * Y_ante = P_3(X_, Y_, coeff_y);
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
		coeff_incr_array[0][Y][X].P0 = P_0(coeff_x);
		coeff_incr_array[0][Y][X].P1 = P_1(X_p, Y_p, coeff_x); 
		coeff_incr_array[0][Y][X].P2 = P_2(X_p, Y_p, coeff_x); 
		coeff_incr_array[0][Y][X].P3 = P_3(X_p ,Y_p, coeff_x);

		//Q
		coeff_incr_array[0][Y][X].Q0 = Q_0(coeff_x); 
		coeff_incr_array[0][Y][X].Q1 = Q_1(X_p, Y_p, coeff_x);
		coeff_incr_array[0][Y][X].Q2 = Q_2(X_p, Y_p, coeff_x); 
		coeff_incr_array[0][Y][X].Q3 = coeff_incr_array[0][Y][X].P3;


		//R
		coeff_incr_array[0][Y][X].R0 = R_0(coeff_x);
		coeff_incr_array[0][Y][X].R1 = R_1(X_p, Y_p, coeff_x); 
		coeff_incr_array[0][Y][X].R2 = coeff_incr_array[0][Y][X].P2;

		//S
		coeff_incr_array[0][Y][X].S0 = S_0(coeff_x);
		coeff_incr_array[0][Y][X].S1 = coeff_incr_array[0][Y][X].P1;


		//y coefficient
		//P
		coeff_incr_array[1][Y][X].P0 = P_0(coeff_y);
		coeff_incr_array[1][Y][X].P1 = P_1(X_p, Y_p, coeff_y);
		coeff_incr_array[1][Y][X].P2 = P_2(X_p, Y_p, coeff_y); 
		coeff_incr_array[1][Y][X].P3 = P_3(X_p, Y_p, coeff_y);

		//Q
		coeff_incr_array[1][Y][X].Q0 = Q_0(coeff_y);
		coeff_incr_array[1][Y][X].Q1 = Q_1(X_p, Y_p, coeff_y); 
		coeff_incr_array[1][Y][X].Q2 = Q_2(X_p, Y_p, coeff_y); 
		coeff_incr_array[1][Y][X].Q3 = coeff_incr_array[0][Y][X].P3;


		//R
		coeff_incr_array[1][Y][X].R0 = R_0(coeff_y);
		coeff_incr_array[1][Y][X].R1 = R_1(X_p, Y_p, coeff_y);	
		coeff_incr_array[1][Y][X].R2 = coeff_incr_array[0][Y][X].P2;

		//S
		coeff_incr_array[1][Y][X].S0 = S_0(coeff_y); 
		coeff_incr_array[1][Y][X].S1 = coeff_incr_array[0][Y][X].P1;


	 }

  } 

}
