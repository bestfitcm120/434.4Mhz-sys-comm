/* led.c - status LED driver (PB1, high = on) */
#include "config.h"
#include "led.h"

void led_init(void)
{
    LED_DDR  |= (1 << LED_BIT);
    LED_PORT &= (uint8_t)~(1 << LED_BIT);
}

void led_on(void)
{
    LED_PORT |= (1 << LED_BIT);
}

void led_off(void)
{
    LED_PORT &= (uint8_t)~(1 << LED_BIT);
}

void led_toggle(void)
{
    LED_PORT ^= (1 << LED_BIT);
}

void led_set(bool on)
{
    if (on) {
        led_on();
    } else {
        led_off();
    }
}
