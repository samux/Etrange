/**
 * \file irq_handler.h
 * \brief Contains interrupt handlers
 * \author Samuel Mokrani
 * \date 20/06/2011
 *
 * This file contains four interrupt handlers : 
 * 	video_in_handler
 * 	video_out_handler
 * 	video_calc_handler
 * 	tty_handler
 *
 */
#ifndef _IRQ_HANDLERH_
#define _IRQ_HANDLER_H_

#include "segmentation.h"
#include "utils.h"
#include <stdint.h>



#define VIN *((volatile uint32_t *)WBS_BASE)							
/**< Contains the address of the next stored tile */
#define VIN_CRL *((volatile uint32_t *)(WBS_BASE + 4))
/**< Control register to notify to V_IN that a new address to store an image is available */
#define VOUT *((volatile uint32_t *)(WBS_BASE + 8))
/**< Contains the address of the next displayed tile */
#define VOUT_CRL *((volatile uint32_t *)(WBS_BASE + 12))
/**< Control register to notify to V_OUT that a new address to read an image is available */
#define VCALC_IN *((volatile uint32_t *)(WBS_BASE + 16))
/**< Contains the address of the next read tile to be processed by V_CALC*/
#define VCALC_IN_CRL *((volatile uint32_t *)(WBS_BASE + 20))
/**< Control register to notify to V_CALC that a new address to read an image is available */
#define VCALC_OUT *((volatile uint32_t *)(WBS_BASE + 24))
/**< Contains the address of the next processed tile by V_CALC*/
#define VCALC_OUT_CRL *((volatile uint32_t *)(WBS_BASE + 28))
/**< Control register to notify to V_CALC that a new address to store an image is available */
#define VCALC_POLY *((volatile uint32_t *)(WBS_BASE + 32))


extern char inbyte();

extern volatile uint32_t nb_image_in;
extern volatile uint32_t nb_image_out;
extern uint8_t first_image;
extern uint8_t first_image_processed;

extern uint32_t * RAM_FIRST_IMAGE;
extern uint32_t * RAM_FIRST_IMAGE_PROCESSED;
extern COEFF_INCR coeff_incr_array[2][NB_TILE_HEIGHT][NB_TILE_WIDTH];


/**
 * \fn void video_in_handler()
 * \brief interrupt handler for V_IN
 *
 * When we receive an interruption from V_IN, we send:
 * 	- the address where will be stored the next image for VIN
 * 	- knowing that VCALC is very fast, we send also VCALC_IN, 
 * 		VCALC_OUT and coefficients address
 */
void video_in_handler();


/**
 * \fn void video_calc_handler()
 * \brief interrupt handler for V_CALC
 *
 * It's only an interrupt handler to initialize VOUT for the first image
 */
void video_calc_handler();


/**
 * \fn void video_out_handler()
 * \brief interrupt handler for V_OUT
 *
 * When we receive an interruption from video_out, we send:
 * 	- the address of a new image processed
 */
void video_out_handler();

/**
 * \fn void tty_handler()
 * \brief interrupt handler for TTY
 *
 * This handler can be used to change coefficients of the transformation 
 * by keypad 
 */
void tty_handler();

#endif
