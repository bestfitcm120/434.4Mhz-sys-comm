/*
 * packet.h - builds the over-the-air packet.
 *
 *   AA AA | 2D D4 | ADDR | LEN | MESSAGE (LEN bytes) | CRC-8
 *
 * The CRC-8 (polynomial 0x07, start value 0) covers ADDR, LEN and MESSAGE.
 */
#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

/* Returns the CRC-8 of len bytes. */
uint8_t packet_crc8(const uint8_t *data, uint8_t len);

/* Fills out[] (at least PKT_MAX_LEN bytes) and returns the packet length,
 * which is PKT_OVERHEAD + len. Returns 0 if len is 0 or above MSG_MAX_LEN. */
uint8_t packet_build(uint8_t *out, const char *msg, uint8_t len);

#endif /* PACKET_H */
