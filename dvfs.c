// CPU frequency configuration for MSP430FR5994 and MSP432P401R.
//
// The 11-level frequency table spans 1–48 MHz.  MSP430 uses levels 1–8
// (DCO register combinations via CS_setDCOFreq); MSP432 uses levels 9–11
// (centred DCO via CS_setDCOCenteredFrequency).  Default FreqLevel is 8
// (16 MHz) on MSP430 and 10 (24 MHz) on MSP432.
//
// IMPORTANT for level 11 (48 MHz, MSP432 only):
//   The core voltage must be raised to VCORE1 before switching the DCO.
//   Skipping this step can brick the board (requires factory reset via
//   Energia's backdoor).  Flash wait states must also be bumped to 1 for
//   both banks; see the e2e.ti.com link in the code for the incident report.
//
// setFrequency(level) traps on unknown levels with while(1) so that
// misconfiguration is immediately visible on an oscilloscope or debugger
// rather than silently running at the wrong speed.

#include "portable.h"
#ifdef __TOOLS_MSP__
#include "driverlib.h"
#endif
#include "dvfs.h"

#ifdef __MSP430__
unsigned int FreqLevel = 8;
#elif defined(__MSP432__)
unsigned int FreqLevel = 10;
#endif

/*
 * 10 level of CPU frequency for MSP430 and MSP432
 *
 * Level: 1: 1MHz
 *        2: 2.67MHz
 *        3: 3.33MHz
 *        4: 4MHz
 *        5: 5.33MHz
 *        6: 6.67MHz
 *        7: 8MHz
 *        8: 16MHz
 *        9: 12MHz
 *        10: 24MHz
 *        11: 48MHz
 *
 */
void setFrequency(int level) {
  switch (level) {
#ifdef __MSP430__
    case 1:  // Set DCO frequency to 1 MHz
      CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_0);
      FreqLevel = 1;
      break;
    case 2:  // Set DCO frequency to 2.67 MHz
      CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_1);
      FreqLevel = 2;
      break;
    case 3:  // Set DCO frequency to 3.5 MHz
      CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_2);
      FreqLevel = 3;
      break;
    case 4:  // Set DCO frequency to 4 MHz
      CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_3);
      FreqLevel = 4;
      break;
    case 5:  // Set DCO frequency to 5.33 MHz
      CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_4);
      FreqLevel = 5;
      break;
    case 6:  // Set DCO frequency to 7 MHz
      CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_5);
      FreqLevel = 6;
      break;
    case 7:  // Set DCO frequency to 8 MHz
      CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);
      FreqLevel = 7;
      break;
    case 8:                         // Set DCO frequency to 16 MHz
      FRCTL0 = FRCTLPW | NWAITS_1;  // Up to 16Mhz
      CS_setDCOFreq(CS_DCORSEL_1, CS_DCOFSEL_4);
      FreqLevel = 8;
      break;
#elif defined(__MSP432__)
    case 9:  // Set DCO to 12MHz
      CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
      FreqLevel = 9;
      break;
    case 10:  // set DCO to 24MHz
      CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);
      FreqLevel = 10;
      break;
    case 11:  // set DCO to 48MHz
      // Before we start we have to change VCORE to 1 to support the 48MHz
      // frequency The board will be bricked and require a factory reset without
      // adjusting VCORE!
      // https://e2e.ti.com/support/microcontrollers/msp430/f/msp-low-power-microcontroller-forum/610691/ccs-msp432p401r-_bricked_-launchpad-cannot-program-the-device-anymore-after-using-cs_setdcofrequency
      PCM_setCoreVoltageLevel(PCM_AM_LDO_VCORE1);
      FlashCtl_setWaitState(FLASH_BANK0, 1);
      FlashCtl_setWaitState(FLASH_BANK1, 1);
      CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
      FreqLevel = 11;
      break;
#endif
    default:
      while (1);
  }
}

unsigned long getFrequency(int level) {
  switch (level) {
    case 1:  // Set DCO frequency to 1 MHz
      return 1000000;
    case 2:  // Set DCO frequency to 2.67 MHz
      return 2670000;
    case 3:  // Set DCO frequency to 3.33 MHz
      return 3330000;
    case 4:  // Set DCO frequency to 4 MHz
      return 4000000;
    case 5:  // Set DCO frequency to 5.33 MHz
      return 5330000;
    case 6:  // Set DCO frequency to 6.67 MHz
      return 6670000;
    case 7:  // Set DCO frequency to 8 MHz
      return 8000000;
    case 8:  // Set DCO frequency to 16 MHz
      return 16000000;
    case 9:  // Set DCO frequency to 12MHz
      return 12000000;
    case 10:  // set DCO frequency to 24MHz
      return 24000000;
    case 11:  // set DCO frequency to 48MHz
      return 48000000;
    default:
      while (1);
  }
  return 0;
}
