/* packet.c - CRC-8 for the receiver. Declared in the shared packet.h. */
#include "rx_config.h"
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
