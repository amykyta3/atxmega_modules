
#ifndef INTEL_HEX_H
#define INTEL_HEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct ihex_packet{
    uint32_t addr;
    uint8_t len;
    uint8_t data[32];
};


#define IHEX_ERROR          0xFF
#define IHEX_DATA           0x00
#define IHEX_EOF            0x01
#define IHEX_EXT_SEGMENT    0x02
#define IHEX_EXT_LINEAR     0x04

/**
 * \brief Parses an intel-hex line
 * \details If extended addresses are used, they are automatically applied to the following data records
 * \param str Pointer to input string
 * \param dst Pointer to destination packet struct (only valid if IHEX_DATA)
 * \returns record type. Returns IHEX_ERROR on error.
 **/
uint8_t parse_intel_hex(char *str, struct ihex_packet *dst);

#ifdef __cplusplus
}
#endif

#endif
