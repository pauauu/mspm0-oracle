/*
 * This file contains the declarations for for common functions that
 * we will use in ELEC327. See LICENSE file for details on licensing.
*/

#ifndef oracle_helper_include
#define oracle_helper_include

#include <ti/devices/msp/msp.h>

#define POWER_STARTUP_DELAY (16)


#ifdef __cplusplus
extern "C" {
#endif

//Writes value to specified register
__STATIC_INLINE void update_reg(
    volatile uint32_t *reg, uint32_t val, uint32_t mask)
{
    uint32_t tmp;

    tmp  = *reg;
    tmp  = tmp & ~mask;
    *reg = tmp | (val & mask);
}


void delay_cycles(uint32_t cycles);

//Initializes clocks
void InitializeProcessor(void);

//Initialize GPIOs
void InitializeGPIO(void);
#define SW1 ((uint32_t) 0x1 << 13)
#define SW2 ((uint32_t) 0x1 << 14)
#define SW3 ((uint32_t) 0x1 << 15)
void InitializeTimerG0(void);
void InitializeTimerA1_PWM(void);


#ifdef __cplusplus
}
#endif

#endif


