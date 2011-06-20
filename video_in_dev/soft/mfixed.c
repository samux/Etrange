/************************************************************
 *
 * File mfixed.c
 *
 * Simple Fixed point math
 *
 * 16 bis.16bits
 *
 ************************************************************/

#include "mfixed.h"

// fixed point multiplication
inline mfixed fx_mul  (mfixed A, mfixed B)
{

    return (mfixed)( ((A.l*B.l)>> 16) +
            (A.l * B.h )    +
            (A.h * B.l )    +
            ((A.h*B.h) << 16)
            );
}

// fixed point addition
inline mfixed fx_add (mfixed A, mfixed B)
{
    return (mfixed)(A.all + B.all);
}
