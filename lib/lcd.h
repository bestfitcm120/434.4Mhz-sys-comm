/* lcd.h - HD44780 16x2 LCD, 4-bit mode, write only */
#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void lcd_init(void);
void lcd_clear(void);
void lcd_goto(uint8_t col, uint8_t row);
void lcd_puts(const char *s);

/* Shows len characters: the first 16 on line 1, the rest on line 2.
 * Both lines are padded with spaces, so old text never lingers. */
void lcd_show(const char *msg, uint8_t len);

#endif /* LCD_H */
