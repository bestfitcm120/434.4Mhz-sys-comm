/* timebase.h - free-running millisecond counter on Timer 0 */
#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdint.h>

void     timebase_init(void);
uint32_t timebase_ms(void);     /* milliseconds since timebase_init() */

#endif /* TIMEBASE_H */
