/*
 * =============================================================================
 *
 *       Filename:  lm32_irq.h
 *
 *    Description:  LM32 Interruption handling
 *
 *        Version:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Tarik Graba (TG), tarik.graba@telecom-paristech.fr
 *        Company:  Telecom Paris TECH
 *
 *           Note: The code is inspired by the FreeRTOS community contribution
 *                 port for the LM32 done by Klaus Fetsche
 * =============================================================================
 */



#ifndef _LM32_IRQ_H_
#define _LM32_IRQ_H_

// Lm32 has 32 IRQs from 0 to 31
#define    IRQ_NUM_MAX        31

#define    ZERO          0
#define    ONE           1

// Disable all IRQs
#define irq_disable()  asm volatile ( "wcsr IE,r0")

// Enable IRQs
// Each IRQ has to be unmasked
#define irq_enable()  do { \
                         unsigned long IE ;\
                         IE = 0x1 ;  \
                         asm volatile ( "wcsr IE,%0"::"r"(IE) );\
                         } while (0)

// get cycle counter
unsigned long get_cc ( void );

// IRQ callback function type
typedef void(*IrqCallback_t)( void ); 

// Enable an IRQ
// returns ONE if ok else returns ZERO
unsigned long enable_irq ( unsigned long irqN );

// Disable an IRQ
// returns ONE if ok else returns ZERO
unsigned long disable_irq ( unsigned long irqN );

// Main IRQ handler called by the interrupt vector
void     IRQ_handler ( void );

// Register a callback function for a particular IRQ
// If the callback is NULL the IRQ will be disabled
// else it will be enabled
// returns ONE if ok else returns ZERO
unsigned long RegisterIrqEntry ( unsigned long irqN, IrqCallback_t func );


#endif
