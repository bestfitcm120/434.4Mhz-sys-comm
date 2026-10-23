/*
 * main.c - 434.4 MHz Link transmitter (ATmega32, 8 MHz).
 *
 * What goes out, in priority order:
 *   1. a line typed on the serial port (9600 baud, 8N1, ends with Enter)
 *   2. BUTTON_MESSAGE, when the button on PD2 is pressed
 *   3. AUTO_MESSAGE, once AUTO_PERIOD_MS has passed since the last transmission
 *
 * Each message becomes one packet, goes out as Manchester OOK on PB0, and
 * the LED on PB1 stays on while the radio is sending.
 */
#include <avr/interrupt.h>
#include <stdbool.h>

#include "config.h"
#include "timebase.h"
#include "led.h"
#include "button.h"
#include "serial_rx.h"
#include "packet.h"
#include "radio_tx.h"

/* Compile-time checks (plain C99, so old avr-gcc versions work too) */
#define STATIC_CHECK(name, cond) typedef char check_##name[(cond) ? 1 : -1]

STATIC_CHECK(button_msg_len, sizeof(BUTTON_MESSAGE) - 1U >= 1U && sizeof(BUTTON_MESSAGE) - 1U <= MSG_MAX_LEN);
STATIC_CHECK(auto_msg_len,   sizeof(AUTO_MESSAGE) - 1U >= 1U && sizeof(AUTO_MESSAGE) - 1U <= MSG_MAX_LEN);
STATIC_CHECK(timer1_fits,    RADIO_TIMER_TOP <= 0xFFFFUL);

static void transmit(const char *msg, uint8_t len)
{
    uint8_t pkt[PKT_MAX_LEN];
    uint8_t n = packet_build(pkt, msg, len);

    if (n > 0) {
        radio_tx_send(pkt, n);
    }
}

int main(void)
{
    char    msg[MSG_MAX_LEN + 1];
    uint8_t len;
    bool    was_busy = false;

    timebase_init();
    led_init();
    button_init();
    serial_rx_init();
    radio_tx_init();

    sei();

    for (;;) {
        bool busy = radio_tx_busy();

        button_update();
        led_set(busy);

        if (busy) {
            was_busy = true;
            continue;               /* wait for the current message to finish */
        }
        if (was_busy) {             /* a transmission just ended */
            was_busy = false;
            serial_rx_restart_auto();
        }

        len = serial_rx_get_line(msg);
        if (len > 0) {
            transmit(msg, len);
            continue;
        }

        if (button_pressed()) {
            transmit(BUTTON_MESSAGE, (uint8_t)(sizeof(BUTTON_MESSAGE) - 1U));
            continue;
        }

        len = serial_rx_get_auto(msg);
        if (len > 0) {
            transmit(msg, len);
        }
    }
}