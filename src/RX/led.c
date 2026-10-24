/* led.c - lock LED driver for the receiver (PD7, high = on). Uses the shared led.h. */
#include "rx_config.h"
#include "led.h"

void led_init(void)
{
    RXLED_DDR  |= (1 << RXLED_BIT);
    RXLED_PORT &= (uint8_t)~(1 << RXLED_BIT);
}

void led_on(void)
{
    RXLED_PORT |= (1 << RXLED_BIT);
}

void led_off(void)
{
    RXLED_PORT &= (uint8_t)~(1 << RXLED_BIT);
}

void led_toggle(void)
{
    RXLED_PORT ^= (1 << RXLED_BIT);
}

void led_set(bool on)
{
    if (on) {
        led_on();
    } else {
        led_off();
    }
}
