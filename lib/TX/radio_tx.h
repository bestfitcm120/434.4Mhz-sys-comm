/*
 * radio_tx.h - Manchester OOK transmitter on the KEY pin (PB0).
 *
 * Sending runs from Timer 1 interrupts, so main() is free while a packet
 * goes out. Each packet is sent TX_REPEATS times with a TX_GAP_MS pause.
 */
#ifndef RADIO_TX_H
#define RADIO_TX_H

#include <stdbool.h>
#include <stdint.h>

void radio_tx_init(void);

/* Copies the packet and starts sending it. Returns false if a transmission
 * is still running or the length is not valid. */
bool radio_tx_send(const uint8_t *pkt, uint8_t len);

/* True from radio_tx_send() until the last repeat has finished. */
bool radio_tx_busy(void);

#endif /* RADIO_TX_H */
