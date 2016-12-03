#include "stm32f4xx.h"
#include "util_macros.h"
#include "stm32f4_regs.h"
#include "errata.h"

// Notes
// =====
// - DAC1_OUT/DAC_OUT1 is on pin PA4 (connected to CS43L22)
// - DAC2_OUT/DAC_OUT2 is on pin PA5
// - Vdda = analog power supply
// - Vssa = ground for analog power supply
// - Can enable output buffers to reduce output impedance.
// - DAC output = Vref * (DOR / 4095)
// - TSELx[2:0] bit cannot be changed when the ENx bit is set.

void dac_basic_init(void)
{
    // Configure PA5 and PA4 to analog (AIN) to avoid parasitic power consumption.
    SET_BITS_AFTER_CLR(GPIOA->MODER, GPIO_MODER_MASK, GPIO_MODER_VAL_ANA, (5 * GPIO_MODER_BITS));
    SET_BITS_AFTER_CLR(GPIOA->MODER, GPIO_MODER_MASK, GPIO_MODER_VAL_ANA, (4 * GPIO_MODER_BITS));

    // Peripheral clock enable for GPIO A (AHB)
    // Port A clock enable
    SET_BITS(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN_VAL_ENABLE, RCC_AHB1ENR_GPIOAEN_SHIFT);

    // Peripheral clock enable for DAC  (APB)
    SET_BITS(RCC->APB1ENR, RCC_APB1ENR_DACEN_VAL_ENABLE, RCC_APB1ENR_DACEN_SHIFT);
    ERRATA_2_1_13;

    // Enable channel 1 on DAC
    SET_BITS(DAC->CR, DAC_CR_EN1_VAL_ENABLE, DAC_CR_EN1_SHIFT);
}

void dac_basic_repeat(void)
{
    uint16_t val = DAC_DHR12R1_MASK;

    do
    {
        DAC->DHR12R1 = val;
        val--;
    } while(val);
}

