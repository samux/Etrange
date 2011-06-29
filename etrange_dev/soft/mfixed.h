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

/**
 * \union mfixed
 * \brief Fixed point type definition
 *
 * Fixed point type definition: 16 bits for entire
 * part and 16 bits for fractionnal part.
 * Warning: This work only for big endian
 */
typedef union {
    struct {
        short h; /**< the entire part (16 bits)*/
        unsigned short l; /**< The fractionnal part (16 bits)*/
    } ;
    int all; /**< all the number (32 bits)*/
}mfixed;

/**
 * \fn inline mfixed (mfixed A, mfixed B)
 * \brief Compute the multiplication between two mfixed
 */
inline mfixed fx_mul  (mfixed A, mfixed B);

/**
 * \fn inline mfixed (mfixed A, mfixed B)
 * \brief Compute the addition between two mfixed
 */
inline mfixed fx_add (mfixed A, mfixed B);

inline mfixed mf (float x);
#endif

