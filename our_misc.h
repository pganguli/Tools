// Platform-portable cycle-accurate delay loop.
//
// our_delay_cycles(n) spins for approximately n CPU cycles.  It is used to
// generate brief GPIO pulses for oscilloscope measurement and for timing
// intervals on platforms that lack a hardware delay primitive.
//
// Implementation:
//   ARM (Cortex-M4): NOP + SUBS + BNE loop.  Each iteration is 3 cycles
//     (NOP=1, SUBS=1, BNE=1 not-taken / 1+P taken; pipeline makes taken
//     branch cost 1 extra cycle in practice).  The NOP is included to make
//     the cycle count per iteration exactly 4 when branch is taken (3 pipeline
//     stages).  n_cycles is divided by 4 in the macro wrapper.
//
//   MSP430 (GCC): NOP + DEC + JNE loop.  Same 3-instruction structure.
//     The %= in the label generates a unique suffix to avoid link-time label
//     conflicts when the inline function is emitted in multiple translation
//     units.
//
//   MSP430 (non-GCC): declared as extern void; implementation in a .s file.
//
//   PC / other: no-op (division by zero is a non-issue; the macro expands
//     to nothing).

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __arm__
inline void our_delay_cycles_internal(uint32_t n_cycles) {
  // %= guarantees unique local labels when inlined into multiple TUs.
  __asm__ volatile(
      ".Lour_delay_cycles_loop%=:\n"
      "nop\n"
      "subs %[n_cycles], %[n_cycles], #1\n"
      "bne .Lour_delay_cycles_loop%=\n"
      :
      : [n_cycles] "r"(n_cycles));
}
#elif defined(__MSP430__)
#ifdef __GNUC__
inline void our_delay_cycles_internal(uint32_t n_cycles) {
  __asm__ volatile(
      ".Lour_delay_cycles_loop%=:\n"
      "NOP\n"
      "DEC %[n_cycles]\n"
      "JNE .Lour_delay_cycles_loop%=\n"
      :
      : [n_cycles] "r"(n_cycles));
}
#else
void our_delay_cycles_internal(uint16_t n_cycles);
#endif
#else
#define our_delay_cycles_internal(n_cycles)
#endif

// both subs and nop take 1 cycle, bne takes 1 + P cycles, and MSP430/MSP432
// uses a 3-stage pipeline the additional nop instruction makes calculating the
// number of iterations faster - division by 3 involves a slow loop on MSP430
// and a slow instruction on other platforms
#define our_delay_cycles(n_cycles) our_delay_cycles_internal(n_cycles / 4)

#ifdef __cplusplus
}
#endif
