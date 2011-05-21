#ifndef _IRQ_HANDLERH_
#define _IRQ_HANDLER_H_

#include "segmentation.h"
#include "utils.h"
#include <stdint.h>

#define VOUT *((volatile uint32_t *)(WBS_BASE + 8))
#define VOUT_CRL *((volatile uint32_t *)(WBS_BASE + 12))
#define VIN *((volatile uint32_t *)WBS_BASE)
#define VIN_CRL *((volatile uint32_t *)(WBS_BASE + 4))
#define CALC_HARD *((volatile uint32_t *)HARD_BASE)

extern volatile uint32_t nb_image;
extern volatile uint32_t nb_image_processed;
extern volatile uint32_t nb_image_out;
extern uint8_t first_image;
extern uint8_t first_image_processed;

void video_out_handler();

void video_in_handler();

void calc_hard_handler();

#endif
