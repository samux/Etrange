#ifndef _IRQ_HANDLERH_
#define _IRQ_HANDLER_H_

#include "segmentation.h"
#include "utils.h"
#include <stdint.h>

#define VIDEO_OUT *((volatile uint32_t *)VIDEO_OUT_BASE)
#define VIDEO_IN *((volatile uint32_t *)VIDEO_IN_BASE)
#define CALC_HARD *((volatile uint32_t *)CALC_HARD_BASE)

extern image_t images[NB_MAX_IMAGES];
extern image_t images_processed[NB_MAX_IMAGES];
extern COEFF_INCR coeff_incr_array[2][NB_TILE_HEIGHT][NB_TILE_WIDTH];


extern volatile uint32_t nb_image;
extern volatile uint32_t nb_image_processed;
extern volatile uint32_t nb_image_out;
extern uint8_t first_image;
extern uint8_t first_image_processed;

void video_out_handler();

void video_in_handler();

void calc_hard_handler();

#endif
