#include "stm32f4xx.h"

// www.st.com/resource/en/errata_sheet/dm00037591.pdf
// - ES0182: STM32F405/407xx and STM32F415/417xx device limitations
//     - STM32F40x and STM32F41x Errata sheet
//         - 2.1.13 Delay after an RCC peripheral clock enabling
#define ERRATA_2_1_13 __DSB()


