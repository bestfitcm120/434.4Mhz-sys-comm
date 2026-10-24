/* button.h - debounced push button on PD2 */
#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

void button_init(void);         /* input with internal pull-up */
void button_update(void);       /* call often from the main loop */
bool button_pressed(void);      /* true once per press, then clears */

#endif /* BUTTON_H */
