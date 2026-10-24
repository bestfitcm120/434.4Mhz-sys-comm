/*
 * radio_rx.h - Manchester OOK receiver on ICP1 (PD6).
 *
 * Timer 1 captures every edge from the slicer. The interrupt turns edge gaps
 * into half-bit levels, finds the 2D D4 sync word, then reads ADDR, LEN, the
 * message and the CRC-8. Only packets with a good CRC reach the main loop.
 */
#ifndef RADIO_RX_H
#define RADIO_RX_H

#include <stdint.h>

void radio_rx_init(void);

/* If a good packet is waiting, copies its text to out (which needs
 * MSG_MAX_LEN + 1 bytes), adds a terminating 0, and returns the length.
 * Returns 0 when nothing is waiting. */
uint8_t radio_rx_get(char *out);

#endif /* RADIO_RX_H */
