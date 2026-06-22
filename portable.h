// Platform-detection macros used by the tools/ layer (myuart, dvfs, extfram).
//
// __TOOLS_MSP__: defined when targeting either MSP430 or MSP432.  Used to
//   gate TI driverlib includes and UART/SPI initialisation code without
//   needing to check both compiler macros at every use site.
//
// STM32_HAL_HEADER: resolves to the correct STM32 HAL include path for the
//   selected STM32 variant.  Add new STM32 device macros here and update
//   myuart.c / extfram.c accordingly.

#pragma once

#if defined(__MSP430__) || defined(__MSP432__)
#define __TOOLS_MSP__
#endif

#ifdef __STM32__

#ifdef STM32L496xx
#define STM32_HAL_HEADER "stm32l4xx_hal.h"
#else
#error "Please verify and add corresponding macros and headers"
#endif

#endif
