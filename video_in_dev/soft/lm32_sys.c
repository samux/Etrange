/*
 * =============================================================================
 *
 *       Filename:  lm32_irq.c
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


#include "lm32_sys.h"

// IRQ entry table
static IrqCallback_t IrqCallback [IRQ_NUM_MAX+1];

// get cycle counter
unsigned long get_cc ( void )
{
    unsigned long CC;
    asm volatile ("rcsr %0, CC" :"=r"(CC));
    return CC;
}

// Register a callback function for a particular IRQ
// returns ONE if ok else returns ZERO
// If the callback is NULL the IRQ will be disabled
// else it will be enabled
unsigned long RegisterIrqEntry ( unsigned long irqN, IrqCallback_t func )
{
    unsigned long mask;
    unsigned long imask;

    // Check if it is an acceptable IRQ
    if (irqN > IRQ_NUM_MAX)
        return (ZERO);
    else
    {
        mask = 0x1 << irqN;

        IrqCallback[irqN] = func;

        // read Interrupt mask
        asm volatile ("rcsr %0, IM" :"=r"(imask));
        // if the callback function is NULL than disable the interrupt
        // else enable it
        imask = (func == 0)? (imask & ~mask) : (imask | mask);
        // write back Interrupt mask
        asm volatile ("wcsr IM, %0" ::"r"(imask));

        return (ONE);
    }
}

// Main IRQ handler
// For each pending interrupt calls the registred callback.
// If the callback does not exist mask the corresponding IRQ.
// The pending IRQ are thoes set in IP and IM CPU regsters
void     IRQ_handler ( void )
{
    unsigned long ipending, imask, mask, irqN;

    // Read interrupt mask
    asm volatile ("rcsr %0,IM":"=r"(imask));
    // Read pending interrupts
    asm volatile ("rcsr %0,IP":"=r"(ipending));
    // Mask the disabled IRQs
    ipending = ipending & imask;

    do
    {

        for (   irqN = 0, mask = 0x1;
                irqN < IRQ_NUM_MAX+1;
                irqN++, mask = mask << 0x1)
        {
            if(mask & ipending)
            {
                // If this IRQ has a registrated callback
                if(IrqCallback[irqN] != 0)
                {
                    // Execute callback
                    (IrqCallback[irqN])();
                    // Ack IRQ
                    asm volatile ("wcsr IP, %0"::"r"(mask));
                    ipending = ipending & ~mask;
                }
                else
                {
                    // The IRQ has no callback function
                    asm volatile ("rcsr %0,IM":"=r"(imask));
                    imask = imask & ~mask;
                    // Mask this IRQ
                    asm volatile ("wcsr IM, %0"::"r"(imask));
                    // ACK the IRQ
                    asm volatile ("wcsr IP, %0"::"r"(mask));
                    ipending = ipending & ~mask;
                }
            }
        }
    }
    while( ipending );
}


// Enable an IRQ and returns ONE
// Takes IRQ number as argument
// Returns ZERO if fails
unsigned long enable_irq ( unsigned long irqN )
{
    unsigned long mask;
    unsigned long imask;

    // Check if it is an acceptable IRQ
    if (irqN > IRQ_NUM_MAX)
        return (ZERO);
    else
    {
        mask = 0x1 << irqN;
        asm volatile ("rcsr %0, IM" :"=r"(imask));
        // Enabling IRQ by setting corresponding bit
        imask = imask | mask;
        asm volatile ("wcsr IM, %0" ::"r"(imask));

        return (ONE);
    }
}

// Disable an IRQ and returns ONE
// Takes IRQ number as argument
// Returns ZERO if fails
unsigned long disable_irq ( unsigned long irqN )
{
    unsigned long mask;
    unsigned long imask;

    // Check if it is an acceptable IRQ
    if (irqN > IRQ_NUM_MAX)
        return (ZERO);
    else
    {
        mask = ~(0x1 << irqN);
        asm volatile ("rcsr %0, IM" :"=r"(imask));
        // Disabling IRQ by unsetting corresponding bit
        imask = imask & mask;
        asm volatile ("wcsr IM, %0" ::"r"(imask));

        return (ONE);
    }
}

