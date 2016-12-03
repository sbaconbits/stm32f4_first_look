#include "stm32f4xx.h"

#include "util_macros.h"
#include "stm32f4_regs.h"

#define VECT_TAB_OFFSET  0x00

// The following defines are calculated using apps/clock_calc.c
#define PLL_P RCC_PLLCFGR_PLLP_VAL_2
#define PLL_Q 7
#define PLL_M 4
#define PLL_N 168

static void set_system_clock(void);

void system_init(void)
{
    // FPU (Floating Point Unit) settings.
    // SCB : System Control Block
    // CPACR : CoProcessor access controler register
    // set CP10 (FPU) and CP11 (NEON/SIMD??) Full Access.
    // NOTE: both must be done in one operation.
    SCB->CPACR |= SCB_CPACR_CP11_VAL_FULL_ACCESS << SCB_CPACR_CP11_SHIFT |
                  SCB_CPACR_CP10_VAL_FULL_ACCESS << SCB_CPACR_CP10_SHIFT;

    // Reset the RCC clock configuration to the default reset state
    // RCC : Reset and Clock Control
    // CR : Clock Control Register
    // High-Speed Internal clock ON.
    //RCC->CR |= (uint32_t)0x00000001;
    SET_BITS(RCC->CR, RCC_CR_HSION_VAL_ON, RCC_CR_HSION_SHIFT);

    // Reset CFGR register
    // CFGR : clock ConFiGuRation register.
    // - System clock (SYSCLK) selected.
    // - MCO2 no division
    // - MCO1 no division
    // - PLLI2S clock used as I2S clock source
    // - MCO1 HSI clock selected
    // - HSE (High Speed External) division factor for RTC, no clock.
    // - APB (Advanced Peripheral Bus) (APB2) high-speed prescaler, AHB
    //   (Advanced High performance Bus) clock not divided.
    // - APB Low-speed prescaler (APB1) AHB clock not divided.
    // - AHB prescaler, system clock not divided.
    // - System clock switch status HSI osc used as the system clock. (read only)
    // - System clock switch HSI osc selected as system clock.
    RCC->CFGR = 0x00000000;

    // Reset HSEON, CSSON and PLLON bits
    // - HSE osc off.
    // - Clock Security System off.
    // - Main PLL off.
    RCC->CR &= ~(RCC_CR_HSEON_MASK << RCC_CR_HSEON_SHIFT |
                 RCC_CR_CSSON_MASK << RCC_CR_CSSON_SHIFT |
                 RCC_CR_PLLON_MASK << RCC_CR_PLLON_SHIFT);

    // Reset PLLCFGR register
    RCC->PLLCFGR = (   4 << RCC_PLLCFGR_PLLQ_SHIFT  |
                     192 << RCC_PLLCFGR_PLLN_SHIFT  |
                     16  << RCC_PLLCFGR_PLLM_SHIFT    );

    // Reset HSEBYP bit
    // HSE Clock bypass, not bypassed
    CLR_BITS(RCC->CR, RCC_CR_HSEBYP_MASK, RCC_CR_HSEBYP_SHIFT);

    // Disable all clock interrupts.
    // - PLLSAI ready
    // - PLLI2S ready
    // - PLL main ready
    // - HSE ready
    // - HSI ready
    // - LSE (Low Speed External) ready
    // - LSI (Low Speed Internal) ready
    RCC->CIR = 0x00000000;

    // Configure the System clock source, PLL Multiplier and Divider factors,
    // AHB/APBx prescalers and Flash settings
    set_system_clock();

    // Vector Table Relocation in Internal FLASH
    SET_BITS(SCB->VTOR, (FLASH_BASE | VECT_TAB_OFFSET), SCB_VTOR_TBLOFF_SHIFT);
}

static void set_system_clock(void)
{
    volatile uint32_t StartUpCounter = 0, HSEStatus = 0;

    // Enable HSE
    SET_BITS(RCC->CR, RCC_CR_HSEON_VAL_ON, RCC_CR_HSEON_SHIFT);

    // Wait until HSE is ready and if Time out is reached exit
    do
    {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;
    } while((HSEStatus == 0) && (StartUpCounter < HSE_STARTUP_TIMEOUT));

    // If HSE is ready (has stabilised)
    if (RCC->CR & RCC_CR_HSERDY)
    {
        // Select regulator voltage output Scale 1 mode, System frequency up to 168 MHz
        // Peripheral clock enable register, power interface clock enable.
        SET_BITS(RCC->APB1ENR, RCC_APB1ENR_PWREN_VAL_ENABLE, RCC_APB1ENR_PWREN_SHIFT);
        // Power control register, regulator voltage scaling (used for power optimisation)
        SET_BITS(PWR->CR, PWR_CR_VOS_VAL_SCALE1, PWR_CR_VOS_SHIFT);

        // Clock configuration register, system clock not divided
        // HPRE : AHB prescaler
        // HCLK = SYSCLK / 1
        SET_BITS_AFTER_CLR(RCC->CFGR, RCC_CFGR_HPRE_MASK,
                           RCC_CFGR_HPRE_VAL_NO_DIVIDE, RCC_CFGR_HPRE_SHIFT);

        // PPR2: APB high-speed prescaler (APB2) "must not exceed 84MHz"
        // NOTE: this register has previously been cleared.
        // PCLK2 = HCLK / 2 = 84MHz
        SET_BITS(RCC->CFGR, RCC_CFGR_PPRE2_VAL_DIV2, RCC_CFGR_PPRE2_SHIFT);

        // PPR1: APB Low-speed prescaler (APB1) "must not exceed 42MHz"
        // PCLK1 = HCLK / 4 = 42MHz
        SET_BITS(RCC->CFGR, RCC_CFGR_PPRE1_VAL_DIV4, RCC_CFGR_PPRE1_SHIFT);

        // Configure the main PLL
        RCC->PLLCFGR = (PLL_M << RCC_PLLCFGR_PLLM_SHIFT                         |
                        PLL_N << RCC_PLLCFGR_PLLN_SHIFT                         |
                        PLL_P << RCC_PLLCFGR_PLLP_SHIFT                         |
                        RCC_PLLCFGR_PLLSRC_VAL_HSE << RCC_PLLCFGR_PLLSRC_SHIFT  |
                        PLL_Q << RCC_PLLCFGR_PLLQ_SHIFT);

        // Enable the main PLL
        SET_BITS(RCC->CR, RCC_CR_PLLON_VAL_ON, RCC_CR_PLLON_SHIFT);

        // Wait till the main PLL is ready
        while((RCC->CR & RCC_CR_PLLRDY) == 0)
        {}

        // Configure Flash prefetch, Instruction cache, Data cache and wait state
        // ACR : Access Control register
        FLASH->ACR = (FLASH_ACR_ICEN_VAL_ENABLE << FLASH_ACR_ICEN_SHIFT |
                      FLASH_ACR_DCEN_VAL_ENABLE << FLASH_ACR_DCEN_SHIFT |
                      FLASH_ACR_LATENCY_VAL_5WS << FLASH_ACR_LATENCY_SHIFT);

        // Select the main PLL as system clock source
        // SW : System clock switch
        SET_BITS_AFTER_CLR(RCC->CFGR, RCC_CFGR_SW_MASK, RCC_CFGR_SW_VAL_PLL, RCC_CFGR_SW_SHIFT)

        // Wait until the main PLL is used as system clock source
        while( (RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL )
        {}
    }
    else
    {
        // NOTE:
        // Handle the failure of HSE not stabilise if we get here.
    }
}



