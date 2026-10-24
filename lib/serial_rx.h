/*
 * serial_rx.h - serial text input (UART, PD0) and the 2 second auto message.
 *
 * Every buffer passed in must hold MSG_MAX_LEN + 1 characters.
 */
#ifndef SERIAL_RX_H
#define SERIAL_RX_H

#include <stdint.h>

void    serial_rx_init(void);

/* A complete line typed on the serial port, without the newline.
 * Returns its length, or 0 if no line is waiting. */
uint8_t serial_rx_get_line(char *dst);

/* The hard-coded AUTO_MESSAGE, once every AUTO_PERIOD_MS.
 * Returns its length, or 0 if it is not due yet. */
uint8_t serial_rx_get_auto(char *dst);

/* Start the AUTO_PERIOD_MS wait again (call after any other transmission). */
void    serial_rx_restart_auto(void);

#endif /* SERIAL_RX_H */
