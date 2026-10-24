/*
 * main.c - 434.4 MHz Link receiver (ATmega32, 8 MHz).
 *
 * The radio interrupt decodes packets. This loop takes each good one, shows it
 * on the 16x2 LCD and blinks the lock LED once. The transmitter sends every
 * message three times, so an identical copy that follows within
 * RX_DUP_WINDOW_MS is ignored.
 */
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "rx_config.h"
#include "timebase.h"
#include "led.h"
#include "lcd.h"
#include "radio_rx.h"

#define STATIC_CHECK(name, cond) typedef char check_##name[(cond) ? 1 : -1]

STATIC_CHECK(one_us_ticks,  F_CPU / RX_TIMER_PRESCALER == 1000000UL);   /* edge times assume 1 us ticks */
STATIC_CHECK(lcd_fits_msg,  LCD_COLS * LCD_ROWS >= MSG_MAX_LEN);

int main(void)
{
    char     msg[MSG_MAX_LEN + 1];
    char     prev[MSG_MAX_LEN + 1];
    uint8_t  prev_len    = 0;
    uint32_t prev_ms     = 0;
    uint32_t led_off_ms  = 0;
    bool     led_is_on   = false;
    uint8_t  len;

    timebase_init();
    led_init();
    lcd_init();
    radio_rx_init();

    lcd_show(LCD_WAITING_TEXT, (uint8_t)(sizeof(LCD_WAITING_TEXT) - 1U));

    sei();

    for (;;) {
        uint32_t now = timebase_ms();

        len = radio_rx_get(msg);
        if (len > 0) {
            bool repeat = prev_len == len &&
                          memcmp(prev, msg, len) == 0 &&
                          (now - prev_ms) < RX_DUP_WINDOW_MS;

            prev_ms = now;                  /* every copy extends the window */
            if (!repeat) {
                memcpy(prev, msg, len);
                prev_len = len;

                lcd_show(msg, len);
                led_on();
                led_is_on  = true;
                led_off_ms = now + RXLED_BLINK_MS;
            }
        }

        if (led_is_on && (int32_t)(now - led_off_ms) >= 0) {
            led_off();
            led_is_on = false;
        }
    }
}
