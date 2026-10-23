# 434.4 MHz Link: transmitter firmware

ATmega32, 8 MHz crystal. Matches blocks T1 to T8 in the project doc.

## Files

| File | What it does |
|---|---|
| `config.h` | Every definition: pins, timings, baud rate, messages, packet bytes |
| `main.c` | Starts everything and runs the main loop |
| `serial_rx.c/.h` | Serial text input on PD0, plus the auto message every 2 s |
| `button.c/.h` | Debounced button on PD2, sends the button message |
| `led.c/.h` | Status LED driver on PB1 |
| `packet.c/.h` | Packet framing and CRC-8 |
| `radio_tx.c/.h` | Manchester OOK transmit on PB0 (Timer 1), 3 repeats |
| `timebase.c/.h` | 1 ms clock (Timer 0) |
| `Makefile` | `make`, `make flash`, `make clean` |
| `tx.hex` | Prebuilt for the ATmega32 |

## What gets sent

1. A line typed on the serial port (9600 baud, 8N1, press Enter).
2. `BUTTON PRESSED` when the button is pressed.
3. `HELLO FROM CAIRO` once 2 seconds have passed since the last transmission.

Change the two hard-coded messages and the 2 s period in `config.h`.

## Pins

| Pin | Use |
|---|---|
| PB0 | KEY, to the T4 OOK keyer (high = carrier on) |
| PB1 | Status LED, on while the radio is sending |
| PD0 | RXD, serial input from T2 or a USB-UART adapter |
| PD2 | Button to ground (internal pull-up) |

## Before you flash

The chip must run from the external 8 MHz crystal. A new ATmega32 uses its
1 MHz internal oscillator, which makes every time and the baud rate wrong.
Check the fuse values for your board with a fuse calculator before writing
them, because a wrong clock fuse can lock the chip out of ISP programming.

Edit `PROGRAMMER` in the Makefile (default `usbasp`), then run `make flash`.
