// UART output API for MSP430/MSP432.
//
// BuadRate [sic]: the UART baud rate constant (9600 baud).  Both MSP430 and
// MSP432 drivers use this to compute the baud-rate divisor register value.
//
// uartinit(): configure the USCI/eUSCI peripheral (baud rate, 8N1 framing)
//   and set up the TX/RX GPIO pins in their peripheral-module function.
//   Must be called once before any print2uart calls.
//
// print2uart(fmt, ...): printf-style formatter that writes each character to
//   the UART TX register and busy-waits for the transmit-complete flag.
//   Used for normal (non-verbose) output that should appear in all builds.
//
// print2uart_new(fmt, ...): improved version used as my_printf on MCU targets
//   (mapped via the my_debug.h alias).  Handles format parsing more robustly
//   than the original and is the one that should be called for all debug
//   output macros.
//
// dummyprint: no-op variadic stub for when DEBUG is not defined; allows
//   dprint2uart calls to be compiled away without #ifdef at the call sites.
//
// print2uartlength: sends a fixed-length (possibly non-NUL-terminated) buffer.
//
// convert / convertl: integer-to-string helpers used internally by the
//   print2uart implementations; exposed in case they are needed elsewhere.

#define BuadRate 9600

#ifdef __cplusplus
extern "C" {
#endif

extern int uartsetup;

void uartinit();
void print2uart(const char* format, ...);
void print2uart_new(const char* format, ...);
void dummyprint(const char* format, ...);
#ifdef DEBUG
#define dprint2uart print2uart
#else
#define dprint2uart dummyprint
#endif

void print2uartlength(char* str, int length);
char* convert(unsigned int num, int base);
char* convertl(unsigned long num, int base);

#ifdef __cplusplus
}
#endif
