/*
 * button.c - debounced push button on PD2.
 *
 * The pin is read every BTN_SAMPLE_MS. A counter climbs while the pin is
 * low and falls while it is high. The press counts when the counter reaches
 * BTN_DEBOUNCE_SAMPLES, and the release counts when it falls back to zero,
 * so contact bounce never produces a second press.
 *
 * A press is remembered until button_pressed() collects it, so a press that
 * happens while the radio is busy is not lost.
 */
#include <stdint.h>

#include "config.h"
#include "timebase.h"
#include "button.h"

static uint8_t  integrator;     /* 0 .. BTN_DEBOUNCE_SAMPLES */
static bool     held;           /* debounced state: true while pressed */
static bool     press_event;    /* latched until collected */
static uint32_t last_sample_ms;

void button_init(void)
{
    BTN_DDR  &= (uint8_t)~(1 << BTN_BIT);   /* input */
    BTN_PORT |= (1 << BTN_BIT);             /* pull-up on */

    integrator     = 0;
    held           = false;
    press_event    = false;
    last_sample_ms = timebase_ms();
}

void button_update(void)
{
    uint32_t now = timebase_ms();
    bool     raw;

    if ((now - last_sample_ms) < BTN_SAMPLE_MS) {
        return;
    }
    last_sample_ms = now;

    raw = ((BTN_PINREG & (1 << BTN_BIT)) == 0);     /* low = pressed */

    if (raw) {
        if (integrator < BTN_DEBOUNCE_SAMPLES) {
            integrator++;
        }
    } else if (integrator > 0) {
        integrator--;
    }

    if (!held && integrator == BTN_DEBOUNCE_SAMPLES) {
        held        = true;
        press_event = true;
    } else if (held && integrator == 0) {
        held = false;
    }
}

bool button_pressed(void)
{
    if (press_event) {
        press_event = false;
        return true;
    }
    return false;
}
