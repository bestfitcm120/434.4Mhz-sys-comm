/*
 * ATmega32 Manchester-coded packet transmitter
 *
 * IMPORTANT:
 *  - Compile with optimization (-Os or -O1 or higher), or _delay_us() is wrong.
 *  - F_CPU must match your real clock. A fresh ATmega32 runs from the
 *    internal 1 MHz RC oscillator. For 8 MHz, set the fuses (CKSEL) for your
 *    crystal or for the 8 MHz internal oscillator.
 *  - Best practice: pass F_CPU on the compiler command line instead of here
 *    (e.g. -DF_CPU=8000000UL). The #ifndef below allows that.
 */
#ifndef F_CPU
#define F_CPU 8000000UL   // 8 MHz
#endif

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define HALF_BIT_US   250   // 250 us per half-bit = 2 kbit/s
#define DEBOUNCE_MS    30

/* =========================================================================
   System Initialization
   - PB0: Manchester data output
   - PB1: TX status LED (use a current-limiting resistor)
   - PD2: push button input with internal pull-up (active low)
   - ADC on PA0, AVCC reference (needs 100 nF cap on AREF to GND)
   ========================================================================= */
void System_Init(void) {
    DDRB |= (1 << PB0) | (1 << PB1);
    PORTB &= ~((1 << PB0) | (1 << PB1));   // start with both outputs low

    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    DDRA &= ~(1 << PA0);
    ADMUX = (1 << REFS0);                                   // AVCC reference, channel ADC0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);     // prescaler 64 -> 125 kHz @ 8 MHz

    // Dummy conversion: the first one after enabling the ADC takes longer
    // and can be inaccurate, so discard it.
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    (void)ADC;
}

/* =========================================================================
   ADC Reading Function
   - Starts a conversion on PA0, waits, returns the 10-bit result.
   ========================================================================= */
uint16_t Read_ADC(void) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

/* =========================================================================
   Manchester Encoding
   - 2 kbit/s (250 us per half-bit)
   - Logic '1': High then Low
   - Logic '0': Low then High
   ========================================================================= */
void Send_Manchester_Bit(uint8_t bit) {
    if (bit) {
        PORTB |= (1 << PB0);
        _delay_us(HALF_BIT_US);
        PORTB &= ~(1 << PB0);
        _delay_us(HALF_BIT_US);
    } else {
        PORTB &= ~(1 << PB0);
        _delay_us(HALF_BIT_US);
        PORTB |= (1 << PB0);
        _delay_us(HALF_BIT_US);
    }
}

void Send_Manchester_Byte(uint8_t byte) {
    for (int8_t i = 7; i >= 0; i--) {
        Send_Manchester_Bit((byte >> i) & 1);
    }
}

/* =========================================================================
   CRC-8 (polynomial 0x07, init 0x00)
   ========================================================================= */
uint8_t Calculate_CRC8(const uint8_t *data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* =========================================================================
   Packet Transmission (9 bytes = 72 bits before Manchester encoding)
     Preamble : AA AA
     Sync     : 2D 34
     Payload  : Address, Length, Temp High, Temp Low
     Checksum : CRC-8 over the 4 payload bytes
   Note: the preamble 0xAA is applied BEFORE Manchester encoding. Check that
   your receiver expects this and not 0xAA on the encoded line.
   ========================================================================= */
void Send_Packet(uint16_t temperature_val) {
    PORTB |= (1 << PB1);   // TX LED on

    const uint8_t address   = 0x01;
    const uint8_t length    = 0x02;
    const uint8_t temp_high = (temperature_val >> 8) & 0xFF;
    const uint8_t temp_low  = temperature_val & 0xFF;

    const uint8_t payload[4] = { address, length, temp_high, temp_low };
    const uint8_t crc = Calculate_CRC8(payload, 4);

    // Preamble
    Send_Manchester_Byte(0xAA);
    Send_Manchester_Byte(0xAA);

    // Sync word
    Send_Manchester_Byte(0x2D);
    Send_Manchester_Byte(0x34);

    // Payload + CRC
    Send_Manchester_Byte(address);
    Send_Manchester_Byte(length);
    Send_Manchester_Byte(temp_high);
    Send_Manchester_Byte(temp_low);
    Send_Manchester_Byte(crc);

    PORTB &= ~(1 << PB0);  // leave data line idle low
    PORTB &= ~(1 << PB1);  // TX LED off
}

int main(void) {
    System_Init();

    while (1) {
        // Button pressed (active low)
        if (!(PIND & (1 << PD2))) {
            _delay_ms(DEBOUNCE_MS);                 // debounce press
            if (!(PIND & (1 << PD2))) {             // still pressed?
                uint16_t adc_reading = Read_ADC();
                Send_Packet(adc_reading);

                // One packet per press: wait for release, then debounce release
                while (!(PIND & (1 << PD2)));
                _delay_ms(DEBOUNCE_MS);
            }
        }
    }
}
