/**
 * \file poly.c
 * \brief Contains all functions concerning polynomes and antecedents
 * \author Samuel Mokrani
 * \date 20/06/2011
 *
 */
#include "poly.h"
#include "stdio.h"
#include "mfixed.h"



/**
 * \fn static mfixed P_0(mfixed coeff[4][4])
 * \brief Compute P_0 coefficient according to coeff
 *
 * \param coeff: coefficients array
 *
 * \return P_0 in mfixed representation
 *
 */
static mfixed P_0(mfixed coeff[4][4]);

/**
 * \fn static mfixed P_1(int X, int Y, mfixed coeff[4][4])
 * \brief Compute P_1 coefficient according to the pixel (X, Y) and coeff
 *
 * \param X: x-coordinate to compute the coefficient
 * \param Y: y-coordinate to compute the coefficient
 * \param coeff: coefficients array
 *
 * \return P_1 in mfixed representation
 *
 */
static mfixed P_1(int X, int Y, mfixed coeff[4][4]);
/**
 * \fn static mfixed P_2(int X, int Y, mfixed coeff[4][4])
 * \brief Compute P_2 coefficient according to the pixel (X, Y) and coeff
 *
 * \param X: x-coordinate to compute the coefficient
 * \param Y: y-coordinate to compute the coefficient
 * \param coeff: coefficients array
 *
 * \return P_2 in mfixed representation
 *
 */
static mfixed P_2(int X, int Y, mfixed coeff[4][4]);

/**
 * \fn static mfixed P_3(int X, int Y, mfixed coeff[4][4])
 * \brief Compute P_3 coefficient according to the pixel (X, Y) and coeff
 *
 * \param X: x-coordinate to compute the coefficient
 * \param Y: y-coordinate to compute the coefficient
 * \param coeff: coefficients array
 *
 * \return P_3 in mfixed representation
 *
 */
static mfixed P_3(int X, int Y, mfixed coeff[4][4]);





/**
 * \fn static mfixed Q_0(mfixed coeff[4][4])
 * \brief Compute Q_0 coefficient according to coeff
 *
 * \param coeff: coefficients array
 *
 * \return Q_0 in mfixed representation
 *
 */
static mfixed Q_0(mfixed coeff[4][4]);

/**
 * \fn static mfixed Q_1(int X, int Y, mfixed coeff[4][4])
 * \brief Compute Q_1 coefficient according to the pixel (X, Y) and coeff
 *
 * \param X: x-coordinate to compute the coefficient
 * \param Y: y-coordinate to compute the coefficient
 * \param coeff: coefficients array
 *
 * \return Q_1 in mfixed representation
 *
 */
static mfixed Q_1(int X, int Y, mfixed coeff[4][4]);

/**
 * \fn static mfixed Q_2(int X, int Y, mfixed coeff[4][4])
 * \brief Compute Q_2 coefficient according to the pixel (X, Y) and coeff
 *
 * \param X: x-coordinate to compute the coefficient
 * \param Y: y-coordinate to compute the coefficient
 * \param coeff: coefficients array
 *
 * \return Q_2 in mfixed representation
 *
 */
static mfixed Q_2(int X, int Y, mfixed coeff[4][4]);







/**
 * \fn static mfixed R_0(mfixed coeff[4][4])
 * \brief Compute R_0 coefficient according to coeff
 *
 * \param coeff: coefficients array
 *
 * \return R_0 in mfixed representation
 *
 */
static mfixed R_0(mfixed coeff[4][4]);

/**
 * \fn static mfixed R_1(int X, int Y, mfixed coeff[4][4])
 * \brief Compute R_1 coefficient according to the pixel (X, Y) and coeff
 *
 * \param X: x-coordinate to compute the coefficient
 * \param Y: y-coordinate to compute the coefficient
 * \param coeff: coefficients array
 *
 * \return R_1 in mfixed representation
 *
 */
static mfixed R_1(int X, int Y, mfixed coeff[4][4]);






/**
 * \fn static mfixed S_0(mfixed coeff[4][4])
 * \brief Compute S_0 coefficient according to coeff
 *
 * \param coeff: coefficients array
 *
 * \return S_0 in mfixed representation
 *
 */
static mfixed S_0(mfixed coeff[4][4]);

/***********
 * P coeff
 * ********/

static mfixed P_0(mfixed coeff[4][4])
{
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
  mfixed a0 = fx_mul(coeff[3][0], (mfixed)( (3*X) << 16));
  mfixed a1 = fx_mul(a0, (mfixed) (X << 16));

  mfixed t1 = fx_add(a1, fx_mul(coeff[3][0], (mfixed)( (3*X + 1) << 16)));

  mfixed t2 = fx_add( t1, fx_mul( (mfixed)( (1 + 2*X) << 16), coeff[2][0]));

  a0 = fx_mul(coeff[2][1], (mfixed)((2*X) << 16));
  a1 = fx_mul(a0, (mfixed)(Y << 16));
  mfixed a2 = fx_add(a1, fx_mul(coeff[2][1], (mfixed)(Y << 16)));

  mfixed t3 = fx_add(t2, a2);

  mfixed a3 = fx_mul(coeff[1][2], (mfixed)(Y << 16));
  mfixed a4 = fx_mul(a3, (mfixed)(Y << 16));

  mfixed t4 = fx_add( t3, a4);
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
  mfixed a0 = fx_mul(coeff[3][0], (mfixed) (X << 16));
  mfixed a1 = fx_mul(a0, (mfixed) (X << 16));
  mfixed a2 = fx_mul(a1, (mfixed) (X << 16));

  mfixed a3 = fx_mul(coeff[0][3], (mfixed) (Y << 16));
  mfixed a4 = fx_mul(a3, (mfixed) (Y << 16));
  mfixed a5 = fx_add(a2, fx_mul(a4, (mfixed) (Y << 16)));

  mfixed a6 = fx_mul(coeff[2][1], (mfixed) (X << 16));
  mfixed a7 = fx_mul(a6, (mfixed) (X << 16));
  mfixed a8 = fx_add(a5, fx_mul(a7, (mfixed) (Y << 16)));

  mfixed a9 = fx_mul(coeff[1][2], (mfixed) (X << 16));
  mfixed a10 = fx_mul(a9, (mfixed) (Y << 16));
  mfixed a11 = fx_add(a8, fx_mul(a10, (mfixed) (Y << 16)));

  mfixed a12 = fx_mul(coeff[2][0], (mfixed) (X << 16));
  mfixed a13 = fx_add(a11, fx_mul(a12, (mfixed) (X << 16)));

  mfixed a14 = fx_mul(coeff[1][1], (mfixed) (X << 16));
  mfixed a15 = fx_add(a13, fx_mul(a14, (mfixed) (Y << 16)));

  mfixed a16 = fx_mul(coeff[0][2], (mfixed) (Y << 16));
  mfixed a17 = fx_add(a15, fx_mul(a16, (mfixed) (Y << 16)));

  mfixed t8 = fx_add( a17, fx_mul( (mfixed)( (X) << 16), coeff[1][0]));
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
  mfixed a0 = fx_mul(coeff[2][1], (mfixed) (X<<16));
  mfixed a1 = fx_mul(a0, (mfixed) (X<<16));

  mfixed a2 = fx_mul(coeff[1][2], (mfixed) ((2*X) << 16));
  mfixed a3 = fx_mul(a2, (mfixed) (Y << 16));
  mfixed a4 = fx_add(a3, fx_mul(coeff[1][2], (mfixed)((X<<16))));
  mfixed a5 = fx_add(a1, a4);

  mfixed a6 = fx_mul(coeff[0][3], (mfixed)((3*Y) << 16));
  mfixed a7 = fx_mul(a6, (mfixed)(Y << 16));
  mfixed a8 = fx_add(a7, fx_mul(coeff[0][3], (mfixed) ((3*Y + 1) << 16)));
  mfixed a9 = fx_add(a8, a5); 


  mfixed t4 = fx_add( a9, fx_mul( (mfixed)( (X) << 16), coeff[1][1]));
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

  mfixed ante_up_left;
  mfixed ante_up_right;
  mfixed ante_bottom_left;
  mfixed ante_bottom_right;
  mfixed min;

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

		//cache management for X
		ante_up_left = coeff_incr_array[0][Y][X].P3;
		min = ante_up_left;

		ante_up_right = P_3(X_p + TILE_WIDTH - 1, Y_p, coeff_x);
		min = (ante_up_right.h < min.h) ? ante_up_right : min;

		ante_bottom_left = P_3(X_p, Y_p + TILE_HEIGHT - 1, coeff_x);
		min = (ante_bottom_left.h < min.h) ? ante_bottom_left : min;

		ante_bottom_right = P_3(X_p + TILE_WIDTH - 1, Y_p + TILE_HEIGHT - 1, coeff_x);
		min = (ante_bottom_right.h < min.h) ? ante_bottom_right : min;

		coeff_incr_array[0][Y][X].begin_cache = min;


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
		coeff_incr_array[1][Y][X].Q3 = coeff_incr_array[1][Y][X].P3;


		//R
		coeff_incr_array[1][Y][X].R0 = R_0(coeff_y);
		coeff_incr_array[1][Y][X].R1 = R_1(X_p, Y_p, coeff_y);
		coeff_incr_array[1][Y][X].R2 = coeff_incr_array[1][Y][X].P2;

		//S
		coeff_incr_array[1][Y][X].S0 = S_0(coeff_y);
		coeff_incr_array[1][Y][X].S1 = coeff_incr_array[1][Y][X].P1;

		//cache management for Y
		ante_up_left = coeff_incr_array[1][Y][X].P3;
		min = ante_up_left;

		ante_up_right = P_3(X_p + TILE_WIDTH - 1, Y_p, coeff_y);
		min = (ante_up_right.h < min.h) ? ante_up_right : min;

		ante_bottom_left = P_3(X_p, Y_p + TILE_HEIGHT - 1, coeff_y);
		min = (ante_bottom_left.h < min.h) ? ante_bottom_left : min;

		ante_bottom_right = P_3(X_p + TILE_WIDTH - 1, Y_p + TILE_HEIGHT - 1, coeff_y);
		min = (ante_bottom_right.h < min.h) ? ante_bottom_right : min;

		coeff_incr_array[1][Y][X].begin_cache = min;

	 }

  }

}


void print_poly()
{
  int i, j;
  for( i = 0; i < 5; i++)
  {
	 for( j = 0; j < 5; j++)
	 {
		printf("------------------TUILE (%d,%d) ----------------------\n", j, i);
		printf("Pour X\n");
		//Pour x
		printf("P[0].x = %d,%d\n", coeff_incr_array[0][i][j].P0.h, coeff_incr_array[0][i][j].P0.l);
		printf("P[1].x = %d,%d\n", coeff_incr_array[0][i][j].P1.h, coeff_incr_array[0][i][j].P1.l);
		printf("P[2].x = %d,%d\n", coeff_incr_array[0][i][j].P2.h, coeff_incr_array[0][i][j].P2.l);
		printf("P[3].x = %d,%d\n", coeff_incr_array[0][i][j].P3.h, coeff_incr_array[0][i][j].P3.l);

		printf("Q[0].x = %d,%d\n", coeff_incr_array[0][i][j].Q0.h, coeff_incr_array[0][i][j].Q0.l);
		printf("Q[1].x = %d,%d\n", coeff_incr_array[0][i][j].Q1.h, coeff_incr_array[0][i][j].Q1.l);
		printf("Q[2].x = %d,%d\n", coeff_incr_array[0][i][j].Q2.h, coeff_incr_array[0][i][j].Q2.l);

		printf("R[0].x = %d,%d\n", coeff_incr_array[0][i][j].R0.h, coeff_incr_array[0][i][j].R0.l);
		printf("R[1].x = %d,%d\n", coeff_incr_array[0][i][j].R1.h, coeff_incr_array[0][i][j].R1.l);

		printf("S[0].x = %d,%d\n", coeff_incr_array[0][i][j].S0.h, coeff_incr_array[0][i][j].S0.l);
		printf("begin_cache.x = %d,%d\n", coeff_incr_array[0][i][j].begin_cache.h, coeff_incr_array[0][i][j].begin_cache.l);

		printf("Pour Y\n");
		//Pour y
		printf("P[1].y = %d,%d\n", coeff_incr_array[0][i][j].P0.h, coeff_incr_array[1][i][j].P0.l);
		printf("P[1].y = %d,%d\n", coeff_incr_array[1][i][j].P1.h, coeff_incr_array[1][i][j].P1.l);
		printf("P[2].y = %d,%d\n", coeff_incr_array[1][i][j].P2.h, coeff_incr_array[1][i][j].P2.l);
		printf("P[3].y = %d,%d\n", coeff_incr_array[1][i][j].P3.h, coeff_incr_array[1][i][j].P3.l);

		printf("Q[1].y = %d,%d\n", coeff_incr_array[0][i][j].Q0.h, coeff_incr_array[1][i][j].Q0.l);
		printf("Q[1].y = %d,%d\n", coeff_incr_array[1][i][j].Q1.h, coeff_incr_array[1][i][j].Q1.l);
		printf("Q[2].y = %d,%d\n", coeff_incr_array[1][i][j].Q2.h, coeff_incr_array[1][i][j].Q2.l);

		printf("R[1].y = %d,%d\n", coeff_incr_array[0][i][j].R0.h, coeff_incr_array[1][i][j].R0.l);
		printf("R[1].y = %d,%d\n", coeff_incr_array[1][i][j].R1.h, coeff_incr_array[1][i][j].R1.l);

		printf("S[1].y = %d,%d\n", coeff_incr_array[1][i][j].S0.h, coeff_incr_array[1][i][j].S0.l);
		printf("begin_cache.y = %d,%d\n", coeff_incr_array[1][i][j].begin_cache.h, coeff_incr_array[1][i][j].begin_cache.l);
	 }
  }
}
