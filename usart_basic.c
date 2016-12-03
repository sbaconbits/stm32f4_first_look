
#include "stm32f4xx.h"
#include "util_macros.h"
#include "stm32f4_regs.h"
#include "errata.h"

#include "usart_basic.h"

#include "version.inc"

// Pins
// ====
// - USART2_RX - PA3
// - USART2_TX - PA2

void usart_basic_init(void)
{
    uint32_t brr_integer, brr_fraction;

    // Enable GPIO clock
    // RCC : Reset and Clock Control
    // AHB : Advanced High performance Bus
    // Reset clock control, advanced high performance bus 1, enable register.
    SET_BITS(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN_VAL_ENABLE, RCC_AHB1ENR_GPIOAEN_SHIFT);

    // Enable USART clock
    // APB : Advanced Peripheral Bus
    // Reset clock control, advanced peripherla bus 1, enable register.
    SET_BITS(RCC->APB1ENR, RCC_APB1ENR_USART2EN_VAL_ENABLE, RCC_APB1ENR_USART2EN_SHIFT);
    ERRATA_2_1_13;

    // Connect USART pins to AF7 (Alternate function)
    // !!! NOTE: chip specific data sheet has the alternate function mapping table !!!!!
    //  PA2 for USART2_TX = AF7
    //  PA3 for USART2_RX = AF7
    //TODO index x/8. 4 is the number of bit for a function
    //TODO write a function for this !!
    SET_BITS_AFTER_CLR(GPIOA->AFR[0], GPIO_AFR_MASK, 0x7, (2 * GPIO_AFR_BITS));
    SET_BITS_AFTER_CLR(GPIOA->AFR[0], GPIO_AFR_MASK, 0x7, (3 * GPIO_AFR_BITS));

    // Configure USART Tx and Rx as alternate function push-pull
    // TODO write a function for this
    SET_BITS_AFTER_CLR(GPIOA->MODER, GPIO_MODER_MASK, GPIO_MODER_VAL_ALT, (2 * GPIO_MODER_BITS));
    SET_BITS_AFTER_CLR(GPIOA->OTYPER, GPIO_OTYPER_MASK, GPIO_OTYPER_VAL_PP, (2 * GPIO_OTYPER_BITS));
    SET_BITS_AFTER_CLR(GPIOA->OSPEEDR, GPIO_OSPEEDR_MASK, GPIO_OSPEEDR_VAL_VHIGH, (2 * GPIO_OSPEEDR_BITS));
    SET_BITS_AFTER_CLR(GPIOA->PUPDR, GPIO_PUPDR_MASK, GPIO_PUPDR_VAL_PU, (2 * GPIO_PUPDR_BITS));

    SET_BITS_AFTER_CLR(GPIOA->MODER, GPIO_MODER_MASK, GPIO_MODER_VAL_ALT, (3 * GPIO_MODER_BITS));
    SET_BITS_AFTER_CLR(GPIOA->OTYPER, GPIO_OTYPER_MASK, GPIO_OTYPER_VAL_PP, (3 * GPIO_OTYPER_BITS));
    SET_BITS_AFTER_CLR(GPIOA->OSPEEDR, GPIO_OSPEEDR_MASK, GPIO_OSPEEDR_VAL_VHIGH, (3 * GPIO_OSPEEDR_BITS));
    SET_BITS_AFTER_CLR(GPIOA->PUPDR, GPIO_PUPDR_MASK, GPIO_PUPDR_VAL_PU, (3 * GPIO_PUPDR_BITS));

    // Configure USART
    // CRn : Control register
    SET_BITS_AFTER_CLR(USART2->CR2, USART_CR2_STOP_MASK,
                       USART_CR2_STOP_VAL_1_STOP_BIT, USART_CR2_STOP_SHIFT);
    // 8 data bits, no parity, enable tx+rx
    SET_BITS(USART2->CR1, USART_CR1_TE_VAL_TX_ENABLE,    USART_CR1_TE_SHIFT);
    SET_BITS(USART2->CR1, USART_CR1_RE_VAL_RX_ENABLE,    USART_CR1_RE_SHIFT);
    // Hardware flow control disabled.
    USART2->CR3 = 0;

    // Baud rate calculation
    // 115200, bit width 8.68uS (currently seeing 27uS)
    // TxRx baud = fCK / (8 * (2 - over8) * USARTDIV)
    // USARTDIV = DIV_Mantissa + (DIV_Fraction / 8 * (2 - over8))
    // fCK = SYSCLK / AHB_prescaler / APB1_prescaler
    //     = 168,000,000 / 1 / 4
    //     = 43,000,000
    // 43E6 / (8 * 2 * 115200) = 23.3289930556
    // The fractional part is the number of 1/16 when in default sample mode
    // (no oversample). Therefore:
    // 16 * 0.3289930556 = 5.2638888896
    brr_integer = 23;
    brr_fraction = 5;

    USART2->BRR = 0;
    SET_BITS(USART2->BRR, (brr_integer  & USART_BRR_MANTISSA_MASK), USART_BRR_MANTISSA_SHIFT);
    SET_BITS(USART2->BRR, (brr_fraction & USART_BRR_FRACTION_MASK), USART_BRR_FRACTION_SHIFT);


    // Enable USART
    SET_BITS(USART2->CR1, USART_CR1_UE_VAL_USART_ENABLE, USART_CR1_UE_SHIFT);

    print_string(version_str);
}

void print_putc(uint8_t c)
{
    // While not transmit data register empty.
    while(! (USART2->SR & USART_SR_TXE) ) {}
    USART2->DR = c;
}

void print_hex_32(uint32_t d)
{
    uint8_t shift = 32;
    uint8_t c;

    do
    {
        shift -= 4;
        c = (d >> shift) & 0xf;
        if((c >= 0xa) && (c <= 0xf))
            print_putc('a'-0xa+c);
        else
            print_putc('0'+c);
    }while(shift);
}

void print_string(char* str)
{
    while(*str != '\0')
        print_putc(*(str++));
}

void usart_basic_repeat(void)
{
    char* str = "Hello\r\n";

    print_string(str);
}

