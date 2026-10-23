/* led.h - status LED driver (PB1) */
#ifndef LED_H
#define LED_H

#include <stdbool.h>

void led_init(void);            /* pin becomes an output, LED off */
void led_on(void);
void led_off(void);
void led_toggle(void);
void led_set(bool on);

#endif /* LED_H */
