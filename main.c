

#include "stm32f4xx.h"
#include "util_macros.h"
#include "stm32f4_regs.h"
#include "errata.h"

extern void usart_basic_init(void);
extern void usart_basic_repeat(void);
extern void dac_basic_init(void);
extern void dac_basic_repeat(void);
extern void gpio_fast_init(void);
extern void gpio_fast_repeat(void);
extern void dac_dma_init(void);
extern void dac_dma_repeat(void);


//TODO Use a real timer
void ms_delay(int ms)
{
   while (ms-- > 0) {
      volatile int x=6000;
      while (x-- > 0)
         __asm("nop");
   }
}

int main(void)
{
    // Enable the clock for GPIOD
    SET_BITS(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN_VAL_ENABLE, RCC_AHB1ENR_GPIODEN_SHIFT);
    ERRATA_2_1_13;
    // Set pin 13 to be general purpose output
    SET_BITS_AFTER_CLR(GPIOD->MODER, GPIO_MODER_MASK, GPIO_MODER_VAL_OUT, (13 * GPIO_MODER_BITS));

    usart_basic_init();
    //dac_basic_init();
    //gpio_fast_init();
    dac_dma_init();

    while(1)
    {
        ms_delay(500);
        // Toggle LED on pin 13
        GPIOD->ODR ^= (1 << (13 * GPIO_ODR_BITS));

        //usart_basic_repeat();
        //dac_basic_repeat();
        //gpio_fast_repeat();
        dac_dma_repeat();
    }
}
