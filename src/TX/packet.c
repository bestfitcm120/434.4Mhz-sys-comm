/* packet.c - packet framing and CRC-8 */
#include <string.h>

#include "config.h"
#include "packet.h"

uint8_t packet_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;

    while (len--) {
        crc ^= *data++;
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80U) {
                crc = (uint8_t)((crc << 1) ^ PKT_CRC_POLY);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

uint8_t packet_build(uint8_t *out, const char *msg, uint8_t len)
{
    uint8_t n = 0;
    uint8_t crc_start;

    if (len == 0 || len > MSG_MAX_LEN) {
        return 0;
    }

    out[n++] = PKT_PREAMBLE_BYTE;
    out[n++] = PKT_PREAMBLE_BYTE;
    out[n++] = PKT_SYNC_HI;
    out[n++] = PKT_SYNC_LO;

    crc_start = n;
    out[n++]  = PKT_ADDRESS;
    out[n++]  = len;
    memcpy(&out[n], msg, len);
    n = (uint8_t)(n + len);

    out[n] = packet_crc8(&out[crc_start], (uint8_t)(2U + len));
    n++;

    return n;
}
