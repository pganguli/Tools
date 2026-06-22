/*
 * Dynamic Voltage and Frequency Scaling (DVFS) API for MSP430/MSP432.
 *
 * FreqLevel: 1-based index into the 11-level frequency table defined in
 *   dvfs.c.  Set at compile time; can be changed at runtime with
 *   setFrequency().  uartinit() uses FreqLevel - 1 to pick the matching
 *   baud-rate register values from UartParams[] in myuart.c.
 *
 * setFrequency(level): reconfigures DCO (MSP430) or CS module (MSP432) to
 *   the frequency corresponding to the given level.  Does NOT reinitialise
 *   the UART — call uartinit() again after a frequency change if UART output
 *   is still needed.
 *
 * getFrequency(level): returns the clock frequency in Hz for the given level.
 *   Useful for computing loop iteration counts for software delay loops.
 */

#ifndef DVFS_H_
#define DVFS_H_

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int FreqLevel;

void setFrequency(int level);
unsigned long getFrequency(int level);

#ifdef __cplusplus
}
#endif

#endif /* DVFS_H_ */
