#ifndef _IRQ_HANDLERH_
#define _IRQ_HANDLER_H_

#include "segmentation.h"
#include "utils.h"
#include <stdint.h>

#define VIN *((volatile uint32_t *)WBS_BASE)
#define VIN_CRL *((volatile uint32_t *)(WBS_BASE + 4))
#define VOUT *((volatile uint32_t *)(WBS_BASE + 8))
#define VOUT_CRL *((volatile uint32_t *)(WBS_BASE + 12))
#define VCALC_R *((volatile uint32_t *)(WBS_BASE + 16))
#define VCALC_R_CRL *((volatile uint32_t *)(WBS_BASE + 20))
#define VCALC_W *((volatile uint32_t *)(WBS_BASE + 24))
#define VCALC_W_CRL *((volatile uint32_t *)(WBS_BASE + 28))

extern volatile uint32_t nb_image_in;
extern volatile uint32_t nb_image_processed_in;
extern volatile uint32_t nb_image_processed_out;
extern volatile uint32_t nb_image_out;
extern uint8_t first_image;
extern uint8_t first_image_processed_in;
extern uint8_t first_image_processed_out;

void video_out_handler();

void video_in_handler();

void video_calc_read_handler();
void video_calc_write_handler();

#endif
