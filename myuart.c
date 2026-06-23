/*
 * UART driver for MSP430/MSP432/STM32.
 *
 * UartParams[]: table of eUSCI_A baud-rate register values for 115200 baud at
 *   each of the 11 clock frequencies (1–48 MHz).  The table is indexed by
 *   FreqLevel - 1.  Values were computed with TI's online baud-rate calculator:
 *   http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/
 *   Changing the baud rate requires recalculating all entries in the table.
 *
 * print2uart: custom hand-rolled printf parser (predates vsnprintf support on
 *   some MSP430 runtimes).  Supports %c, %d, %l, %L, %s, %x.  Recursively
 *   calls itself to print integer strings produced by convert/convertl.
 *   Does NOT support width/precision specifiers.
 *
 * print2uart_new: wrapper around vsnprintf + print2uartlength.  This is the
 *   version mapped to my_printf on MCU targets (see my_debug.h).  It guards
 *   on uartsetup to silently drop output before uartinit() has been called,
 *   which can happen during very early startup on the MSP432.
 *
 * uart_putc: single-character transmit; busy-waits for the TX ready flag via
 *   the driverlib call (EUSCI_A_UART_transmitData / MAP_UART_transmitData).
 *
 * uartsetup global: set to 1 after uartinit() completes; guards against
 *   double-initialisation.
 *
 * EUSCIA0_IRQHandler (MSP432 only): echoes received bytes back to sender,
 *   which makes minicom usable as an interactive terminal.
 */
#include "myuart.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dvfs.h"
#include "portable.h"

#ifdef __TOOLS_MSP__
#include "driverlib.h"
#elif defined(__STM32__)
#include STM32_HAL_HEADER
#else
#error \
    "Please defined __MSP430__, __MSP432__ or __STM32__ according to the target board"
#endif

int uartsetup = 0;

#ifdef __MSP430__
typedef EUSCI_A_UART_initParam EUSCI_CONFIG_PARAMS;
#elif defined(__MSP432__)
typedef eUSCI_UART_Config EUSCI_CONFIG_PARAMS;
#endif

#ifdef __TOOLS_MSP__
// Baud-rate register values for 115200 baud at each clock frequency.
// Values calculated at:
// http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
// See also
// https://dev.ti.com/tirex/explore/node?node=ACmvnDrzuRlhbVcxPmBGTQ__z-lQYNj__LATEST
// for an MSP432 example
const EUSCI_CONFIG_PARAMS UartParams[] = {
    {// 1MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     8,    // clockPrescalar
     0,    // firstModReg
     214,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 2.66MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     1,  // clockPrescalar
     7,  // firstModReg
     4,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 3.5MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     1,   // clockPrescalar
     14,  // firstModReg
     82,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 4MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     2,    // clockPrescalar
     2,    // firstModReg
     187,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 5.33MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     2,   // clockPrescalar
     14,  // firstModReg
     68,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 7MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     3,    // clockPrescalar
     12,   // firstModReg
     221,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 8MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     4,   // clockPrescalar
     5,   // firstModReg
     85,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 16MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     8,    // clockPrescalar
     10,   // firstModReg
     247,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 12MHz
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     6,   // clockPrescalar
     8,   // firstModReg
     32,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 24MHz (24000000 Hz)
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     13,  // clockPrescalar
     0,   // firstModReg
     37,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION},
    {// 48MHz (48000000 Hz)
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     26,   // clockPrescalar
     0,    // firstModReg
     111,  // secondModReg
     EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION}};
#elif defined(__STM32__)
extern UART_HandleTypeDef huart2;
#endif

/* Riotee board routes the MSP430 debug UART over eUSCI_A1 (D1/TX = P2.5),
 * since eUSCI_A0 / P2.0 is the C2C link to the nRF52.  Other MSP430 boards
 * (e.g. the FR5994 LaunchPad) use eUSCI_A0 on P2.0/P2.1. */
#if defined(__MSP430FR5962__)
#define NODPA_UART_BASE EUSCI_A1_BASE
#else
#define NODPA_UART_BASE EUSCI_A0_BASE
#endif

void uart_putc(char c) {
#ifdef __MSP430__
  EUSCI_A_UART_transmitData(NODPA_UART_BASE, (uint8_t)c);
#elif defined(__MSP432__)
  MAP_UART_transmitData(EUSCI_A0_BASE, (uint8_t)c);
#endif
}

void print2uart(const char* format, ...) {
  const char* traverse;
  int i;
  long l;
  unsigned long ul;
  char* s;

  // Module 1: Initializing Myprintf's arguments
  va_list arg;
  va_start(arg, format);

  for (traverse = format; *traverse != '\0'; traverse++) {
    while (*traverse != '%' && *traverse != '\0') {
      uart_putc(*traverse);
      traverse++;
    }

    if (*traverse == '\0') break;

    traverse++;

    // Module 2: Fetching and executing arguments
    switch (*traverse) {
      case 'c':
        i = va_arg(arg, int);  // Fetch char argument
        uart_putc(i);
        break;
      case 'L':
        l = va_arg(arg, long);  // Fetch Decimal/Integer argument
        if (l < 0) {
          l = -l;
          uart_putc('-');
        }
        print2uart(convertl(l, 10));
        break;
      case 'l':
        ul = va_arg(arg, unsigned long);  // Fetch Decimal/Integer argument
        print2uart(convertl(ul, 10));
        break;
      case 'd':
        i = va_arg(arg, int);  // Fetch Decimal/Integer argument
        if (i < 0) {
          i = -i;
          uart_putc('-');
        }
        print2uart(convert(i, 10));
        break;
      case 's':
        s = va_arg(arg, char*);  // Fetch string
        print2uart(s);
        break;
      case 'x':
        i = va_arg(arg, unsigned int);  // Fetch Hexadecimal representation
        print2uart(convert(i, 16));
        break;
    }
  }
  // Module 3: Closing argument list to necessary clean-up
  va_end(arg);
}

static char print2uart_new_buf[64];
void print2uart_new(const char* format, ...) {
  if (uartsetup == 0) {
    return;
  }

  va_list arg;
  va_start(arg, format);

  vsnprintf(print2uart_new_buf, 64, format, arg);
  print2uartlength(print2uart_new_buf, strlen(print2uart_new_buf));

  va_end(arg);
}

void dummyprint(const char* format, ...) { return; }

void print2uartlength(char* str, int length) {
#ifdef __TOOLS_MSP__
  int i;

  for (i = 0; i < length; i++) {
    uart_putc(*(str + i));
  }
#elif defined(__STM32__)
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 0xffff);
#endif
}

char* convert(unsigned int num, int base) {
  static char Representation[] = "0123456789ABCDEF";
  static char buffer[50];
  char* ptr;

  ptr = &buffer[49];
  *ptr = '\0';

  do {
    *--ptr = Representation[num % base];
    num /= base;
  } while (num != 0);

  return (ptr);
}

char* convertl(unsigned long num, int base) {
  static char Representation[] = "0123456789ABCDEF";
  static char buffer[50];
  char* ptr;

  ptr = &buffer[49];
  *ptr = '\0';

  do {
    *--ptr = Representation[num % base];
    num /= base;
  } while (num != 0);

  return (ptr);
}

/* Initialize serial */
void uartinit() {
  if (uartsetup == 0) {
#ifdef __MSP430__
    // Configure UART
    EUSCI_A_UART_initParam param = UartParams[FreqLevel - 1];

    if (STATUS_FAIL == EUSCI_A_UART_init(NODPA_UART_BASE, &param)) return;

    EUSCI_A_UART_enable(NODPA_UART_BASE);

    EUSCI_A_UART_clearInterrupt(NODPA_UART_BASE,
                                EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI RX interrupt
    EUSCI_A_UART_enableInterrupt(
        NODPA_UART_BASE,
        EUSCI_A_UART_RECEIVE_INTERRUPT);  // Enable interrupt

    // Enable globale interrupt
    __enable_interrupt();

#if defined(__MSP430FR5962__)
    // Riotee: UART TXD on D1/TX = P2.5 (UCA1TXD, secondary module function).
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN5,
                                                GPIO_SECONDARY_MODULE_FUNCTION);
#else
    // Select UART TXD on P2.0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0,
                                                GPIO_SECONDARY_MODULE_FUNCTION);
#endif
#elif defined(__MSP432__)
    /* Selecting P1.2 and P1.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    eUSCI_UART_Config param = UartParams[FreqLevel - 1];
    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &param);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);

    /* Enabling interrupts */
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableMaster();
#endif
    uartsetup = 1;
  }
}

#ifdef __MSP432__
// triggered when keyboard input is received in minicom
void EUSCIA0_IRQHandler(void) {
  // Ref: MSP432 example uart_pc_echo_12mhz_brclk
  uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

  MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

  if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) {
    MAP_UART_transmitData(EUSCI_A0_BASE, MAP_UART_receiveData(EUSCI_A0_BASE));
  }
}
#endif
