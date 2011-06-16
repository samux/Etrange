/************************************************************
 *
 * File mfixed.h
 *
 * Simple Fixed point math
 *
 * 16 bis.16bits
 *
 ************************************************************/

#ifndef _MFIXED_H_
#define _MFIXED_H_

// fixed point taype definition
// This works only for big endian
typedef union {
    struct {
        short h;
        unsigned short l;
    } ;
    int all;
}mfixed;

// fixed point multiplication
inline mfixed fx_mul  (mfixed A, mfixed B);

// fixed point addition
inline mfixed fx_add (mfixed A, mfixed B);

inline mfixed i2fx (int A);

#endif

