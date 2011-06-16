#include "poly.h"
#include "stdio.h"
#include "mfixed.h"



static mfixed P_0(mfixed coeff[4][4]);
static mfixed P_1(int X, int Y, mfixed coeff[4][4]);
static mfixed P_2(int X, int Y, mfixed coeff[4][4]);
static mfixed P_3(int X, int Y, mfixed coeff[4][4]);

static mfixed Q_0(mfixed coeff[4][4]);
static mfixed Q_1(int X, int Y, mfixed coeff[4][4]);
static mfixed Q_2(int X, int Y, mfixed coeff[4][4]);

static mfixed R_0(mfixed coeff[4][4]);
static mfixed R_1(int X, int Y, mfixed coeff[4][4]);

static mfixed S_0(mfixed coeff[4][4]);

/***********
 * P coeff
 * ********/

static mfixed P_0(mfixed coeff[4][4])
{
  // 6*a30
  return 	fx_mul((mfixed) (6 << 16), coeff[3][0]);

}

static mfixed P_1(int X, int Y, mfixed coeff[4][4])
{
  mfixed t1 = fx_mul((mfixed)(6*(X+2) << 16), coeff[3][0]);
  mfixed t2 = fx_add(t1, fx_mul((mfixed)(2*Y << 16), coeff[2][1]));
  return fx_add(t2, fx_mul((mfixed)( 2 << 16), coeff[2][0]));
/*
  return 	6 * coeff[3][0] * (X + 2) + 
		  		2 * coeff[2][1] * Y + 
				2 * coeff[2][0];*/

}

static mfixed P_2(int X, int Y, mfixed coeff[4][4])
{
  mfixed t1 = fx_mul( (mfixed)( (3*X*X + 3*X + 1) << 16), coeff[3][0]);
  mfixed t2 = fx_add( t1, fx_mul( (mfixed)( (1 + 2*X) << 16), coeff[2][0]));
  mfixed t3 = fx_add( t2, fx_mul( (mfixed)( (2*X*Y + Y) << 16), coeff[2][1]));
  mfixed t4 = fx_add( t3, fx_mul( (mfixed)( (Y*Y) << 16), coeff[1][2]));
  mfixed t5 = fx_add( t4, fx_mul( (mfixed)( (Y) << 16), coeff[1][1]));
  return fx_add ( t5, coeff[1][0] );
  /*
  return 	coeff[3][0] * (3 * X * X + 3 * X + 1) + 
		  		coeff[2][0] * (1 + 2 * X) + 
				coeff[2][1] * (2 * X * Y + Y) + 
				coeff[1][2] * Y * Y + 
				coeff[1][1] * Y + 
				coeff[1][0];*/

}

static mfixed P_3(int X, int Y, mfixed coeff[4][4])
{
  mfixed t1 = fx_mul( (mfixed)( (X*X*X) << 16), coeff[3][0]);
  mfixed t2 = fx_add( t1, fx_mul( (mfixed)( (Y*Y*Y) << 16), coeff[0][3]));
  mfixed t3 = fx_add( t2, fx_mul( (mfixed)( (X*X*Y) << 16), coeff[2][1]));
  mfixed t4 = fx_add( t3, fx_mul( (mfixed)( (X*Y*Y) << 16), coeff[1][2]));
  mfixed t5 = fx_add( t4, fx_mul( (mfixed)( (X*X) << 16), coeff[2][0]));
  mfixed t6 = fx_add( t5, fx_mul( (mfixed)( (X*Y) << 16), coeff[1][1]));
  mfixed t7 = fx_add( t6, fx_mul( (mfixed)( (Y*Y) << 16), coeff[0][2]));
  mfixed t8 = fx_add( t7, fx_mul( (mfixed)( (X) << 16), coeff[1][0]));
  mfixed t9 = fx_add( t8, fx_mul( (mfixed)( (Y) << 16), coeff[0][1]));
  return fx_add ( t9, coeff[0][0] );
  /*
  return 	coeff[3][0]*X*X*X + 
	 			coeff[0][3]*Y*Y*Y + 
	 			coeff[2][1]*X*X*Y + 
				coeff[1][2]*X*Y*Y + 
	 			coeff[2][0]*X*X + 
				coeff[1][1]*X*Y + 
	 			coeff[0][2]*Y*Y + 
				coeff[1][0]*X + 
	 			coeff[0][1]*Y + 
				coeff[0][0];*/
}

/***********
 * Q coeff
 * ********/

static mfixed Q_0(mfixed coeff[4][4])
{
  return 	fx_mul((mfixed) (6 << 16), coeff[0][3]);
  //return 	6 * coeff_x[0][3];
}

static mfixed Q_1(int X, int Y, mfixed coeff[4][4])
{
  mfixed t1 = fx_mul((mfixed)(2*X << 16), coeff[1][2]);
  mfixed t2 = fx_add(t1, fx_mul((mfixed)(6*(Y+1) << 16), coeff[0][3]));
  return fx_add(t2, fx_mul((mfixed)( 2 << 16), coeff[0][2]));

  /*return 	2 * coeff[1][2] * X + 
		  		6 * coeff[0][3] * (Y + 1) + 
				2 * coeff[0][2];*/

}

static mfixed Q_2(int X, int Y, mfixed coeff[4][4])
{
  mfixed t1 = fx_mul( (mfixed)( (X*X) << 16), coeff[2][1]);
  mfixed t2 = fx_add( t1, fx_mul( (mfixed)( (2*X*Y + X) << 16), coeff[1][2]));
  mfixed t3 = fx_add( t2, fx_mul( (mfixed)( (3*Y*Y + 3*Y + 1) << 16), coeff[0][3]));
  mfixed t4 = fx_add( t3, fx_mul( (mfixed)( (X) << 16), coeff[1][1]));
  mfixed t5 = fx_add( t4, fx_mul( (mfixed)( (2*Y + 1) << 16), coeff[0][2]));
  return fx_add ( t5, coeff[0][1] );
  /*return 	coeff[2][1] * (X * X) + 
		  		coeff[1][2] * ( 2 * X * Y + X) + 
				coeff[0][3] * (3 * Y * Y + 3 * Y + 1) +
				coeff[1][1] * X + 
				coeff[0][2] * (2 * Y + 1) + 
				coeff[0][1]; */

}

/***********
 * R coeff
 * ********/

static mfixed R_0(mfixed coeff[4][4])
{
  return 	fx_mul((mfixed) (2 << 16), coeff[1][2]);
  //return 	2 * coeff[1][2];
}

static mfixed R_1(int X, int Y, mfixed coeff[4][4])
{
  mfixed t1 = fx_mul((mfixed)((2*X + 1) << 16), coeff[2][1]);
  mfixed t2 = fx_add(t1, fx_mul((mfixed)((2*Y + 1) << 16), coeff[1][2]));
  return fx_add(t2, coeff[1][1]);
  /*
  return 	coeff[2][1] * (2 * X + 1) + 
		  		coeff[1][2] * (2 * Y + 1) + 
				coeff[1][1];*/

}

/***********
 * S coeff
 * ********/

static mfixed S_0(mfixed coeff[4][4])
{
  return 	fx_mul((mfixed) (2 << 16), coeff[2][1]);
  //return 	2 * coeff[2][1];

}

/****************************************
 * This function computes the antecedant
 * of (X,Y) and put it in (X_ante,Y_ante)
 * **************************************/
void ante(int X_, int Y_, mfixed * X_ante, mfixed * Y_ante)
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


void print_poly()
{
  //Pour x
  printf("P[0].x = %d,%d\n", coeff_incr_array[0][0][0].P0.h, coeff_incr_array[0][0][0].P0.l);
  printf("P[1].x = %d,%d\n", coeff_incr_array[0][0][0].P1.h, coeff_incr_array[0][0][0].P1.l);
  printf("P[2].x = %d,%d\n", coeff_incr_array[0][0][0].P2.h, coeff_incr_array[0][0][0].P2.l);
  printf("P[3].x = %d,%d\n", coeff_incr_array[0][NB_TILE_HEIGHT - 1][NB_TILE_WIDTH - 1].P3.h, coeff_incr_array[0][NB_TILE_HEIGHT - 1][NB_TILE_WIDTH - 1].P3.l);

  printf("Q[0].x = %d,%d\n", coeff_incr_array[0][0][0].Q0.h, coeff_incr_array[0][0][0].Q0.l);
  printf("Q[1].x = %d,%d\n", coeff_incr_array[0][0][0].Q1.h, coeff_incr_array[0][0][0].Q1.l);
  printf("Q[2].x = %d,%d\n", coeff_incr_array[0][0][0].Q2.h, coeff_incr_array[0][0][0].Q2.l);
 
  printf("R[0].x = %d,%d\n", coeff_incr_array[0][0][0].R0.h, coeff_incr_array[0][0][0].R0.l);
  printf("R[1].x = %d,%d\n", coeff_incr_array[0][0][0].R1.h, coeff_incr_array[0][0][0].R1.l);

  printf("S[0].x = %d,%d\n", coeff_incr_array[0][0][0].S0.h, coeff_incr_array[0][0][0].S0.l);

  //Pour y
  printf("P[1].y = %d,%d\n", coeff_incr_array[0][0][0].P0.h, coeff_incr_array[1][0][0].P0.l);
  printf("P[1].y = %d,%d\n", coeff_incr_array[1][0][0].P1.h, coeff_incr_array[1][0][0].P1.l);
  printf("P[2].y = %d,%d\n", coeff_incr_array[1][0][0].P2.h, coeff_incr_array[1][0][0].P2.l);
  printf("P[3].y = %d,%d\n", coeff_incr_array[1][0][0].P3.h, coeff_incr_array[1][0][0].P3.l);

  printf("Q[1].y = %d,%d\n", coeff_incr_array[0][0][0].Q0.h, coeff_incr_array[1][0][0].Q0.l);
  printf("Q[1].y = %d,%d\n", coeff_incr_array[1][0][0].Q1.h, coeff_incr_array[1][0][0].Q1.l);
  printf("Q[2].y = %d,%d\n", coeff_incr_array[1][0][0].Q2.h, coeff_incr_array[1][0][0].Q2.l);
 
  printf("R[1].y = %d,%d\n", coeff_incr_array[0][0][0].R0.h, coeff_incr_array[1][0][0].R0.l);
  printf("R[1].y = %d,%d\n", coeff_incr_array[1][0][0].R1.h, coeff_incr_array[1][0][0].R1.l);

  printf("S[1].y = %d,%d\n", coeff_incr_array[1][0][0].S0.h, coeff_incr_array[1][0][0].S0.l);

}
