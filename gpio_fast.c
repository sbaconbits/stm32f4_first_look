
#include "stm32f4xx.h"
#include "util_macros.h"
#include "stm32f4_regs.h"
#include "errata.h"

// Pins
// ====
// PA6

void gpio_fast_init(void)
{
    // Peripheral clock enable for GPIO A (AHB)
    // Port A clock enable
    SET_BITS(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN_VAL_ENABLE, RCC_AHB1ENR_GPIOAEN_SHIFT);
    ERRATA_2_1_13;

    // PA6 output push-pull
    // TODO write a function for this
    SET_BITS_AFTER_CLR(GPIOA->MODER,    GPIO_MODER_MASK,    GPIO_MODER_VAL_OUT,     (6 * GPIO_MODER_BITS));
    SET_BITS_AFTER_CLR(GPIOA->OTYPER,   GPIO_OTYPER_MASK,   GPIO_OTYPER_VAL_PP,     (6 * GPIO_OTYPER_BITS));
    SET_BITS_AFTER_CLR(GPIOA->OSPEEDR,  GPIO_OSPEEDR_MASK,  GPIO_OSPEEDR_VAL_VHIGH, (6 * GPIO_OSPEEDR_BITS));
    SET_BITS_AFTER_CLR(GPIOA->PUPDR,    GPIO_PUPDR_MASK,    GPIO_PUPDR_VAL_PU,      (6 * GPIO_PUPDR_BITS));

}

void gpio_fast_repeat(void)
{
    int i;
    uint32_t bit_to_set   = (1 << 6 * GPIO_BSRRL_SET_BITS) << GPIO_BSRRL_SET_SHIFT;
    uint32_t bit_to_clear = (1 << 6 * GPIO_BSRRH_CLR_BITS) << GPIO_BSRRH_CLR_SHIFT;

    for(i=0; i<1000; i++)
    {
        // The following is performing a read-modify-write.
        //GPIOA->ODR ^= (1 << (6 * GPIO_ODR_BITS));
        //GPIOA->ODR ^= (1 << (6 * GPIO_ODR_BITS));
        //GPIOA->ODR ^= (1 << (6 * GPIO_ODR_BITS));

        // Set and clear in different register locations, therefore
        // no need for read-modify-write
        GPIOA->BSRRL = bit_to_set;
        GPIOA->BSRRH = bit_to_clear;
        GPIOA->BSRRL = bit_to_set;
        GPIOA->BSRRH = bit_to_clear;
    }
}
