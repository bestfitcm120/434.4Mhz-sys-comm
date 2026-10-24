/* lcd.c - HD44780 driver, 4-bit mode (D4..D7 = PA0..PA3, RS = PA4, E = PA5) */
#include <util/delay.h>

#include "rx_config.h"
#include "lcd.h"

#define CMD_CLEAR       0x01U
#define CMD_ENTRY_MODE  0x06U   /* cursor moves right, no display shift */
#define CMD_DISPLAY_ON  0x0CU   /* display on, cursor off, blink off */
#define CMD_FUNC_4BIT   0x28U   /* 4-bit bus, 2 lines, 5x8 font */
#define CMD_SET_DDRAM   0x80U

#define LCD_DATA_MASK   (0x0FU << LCD_D4_BIT)

static void write_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t v = LCD_PORT & (uint8_t)~(LCD_DATA_MASK | (1 << LCD_RS_BIT) | (1 << LCD_E_BIT));

    v |= (uint8_t)((nibble & 0x0FU) << LCD_D4_BIT);
    if (rs) {
        v |= (1 << LCD_RS_BIT);
    }
    LCD_PORT = v;
    _delay_us(1);
    LCD_PORT = v | (1 << LCD_E_BIT);
    _delay_us(1);
    LCD_PORT = v;
    _delay_us(1);
}

static void write_byte(uint8_t b, uint8_t rs)
{
    write_nibble((uint8_t)(b >> 4), rs);
    write_nibble(b, rs);
    _delay_us(50);                          /* most commands take 37 us */
}

void lcd_init(void)
{
    LCD_DDR |= (uint8_t)(LCD_DATA_MASK | (1 << LCD_RS_BIT) | (1 << LCD_E_BIT));
    LCD_PORT &= (uint8_t)~(LCD_DATA_MASK | (1 << LCD_RS_BIT) | (1 << LCD_E_BIT));

    _delay_ms(50);                          /* power-up */
    write_nibble(0x03, 0);  _delay_ms(5);   /* reset sequence from the datasheet */
    write_nibble(0x03, 0);  _delay_us(150);
    write_nibble(0x03, 0);  _delay_us(150);
    write_nibble(0x02, 0);  _delay_us(150); /* switch to 4-bit */

    write_byte(CMD_FUNC_4BIT, 0);
    write_byte(CMD_DISPLAY_ON, 0);
    lcd_clear();
    write_byte(CMD_ENTRY_MODE, 0);
}

void lcd_clear(void)
{
    write_byte(CMD_CLEAR, 0);
    _delay_ms(2);                           /* clear takes 1.6 ms */
}

void lcd_goto(uint8_t col, uint8_t row)
{
    write_byte((uint8_t)(CMD_SET_DDRAM | (row ? 0x40U : 0x00U) | col), 0);
}

void lcd_puts(const char *s)
{
    while (*s) {
        write_byte((uint8_t)*s++, 1);
    }
}

void lcd_show(const char *msg, uint8_t len)
{
    for (uint8_t row = 0; row < LCD_ROWS; row++) {
        lcd_goto(0, row);
        for (uint8_t col = 0; col < LCD_COLS; col++) {
            uint8_t i = (uint8_t)(row * LCD_COLS + col);
            write_byte(i < len ? (uint8_t)msg[i] : (uint8_t)' ', 1);
        }
    }
}
