#include "header_driver_t.h"
#include <cstddef>   // for size_t
#include <cstdint>   // for uint32_t, uint16_t

// ------------------------------------------------------------
// TYPE FIELD (2 bits: bits 7–6 of buffer[0])
// ------------------------------------------------------------
void setType_t(unsigned char *buffer, int buf_size, unsigned int value) {
    if (buf_size < 1) return;
    value &= 0x03; // only keep 2 bits
    buffer[0] = (buffer[0] & ~0xC0) | (value << 6);
}

unsigned int getType_t(unsigned char *buffer, int buf_size) {
    if (buf_size < 1) return 0;
    return (buffer[0] & 0xC0) >> 6;
}

// ------------------------------------------------------------
// WINDOW FIELD (5 bits: bits 5–1 of buffer[0])
// ------------------------------------------------------------
void setWin_t(unsigned char *buffer, int buf_size, unsigned int value) {
    if (buf_size < 1) return;
    value &= 0x1F; // only 5 bits
    buffer[0] = (buffer[0] & ~0x3E) | ((value << 1) & 0x3E);
}

unsigned int getWin_t(unsigned char *buffer, int buf_size) {
    if (buf_size < 1) return 0;
    return (buffer[0] & 0x3E) >> 1;
}

// ------------------------------------------------------------
// SEQUENCE FIELD (8 bits: buffer[1])
// ------------------------------------------------------------
void setSeq_t(unsigned char *buffer, int buf_size, unsigned int value) {
    if (buf_size < 2) return;
    buffer[1] = static_cast<unsigned char>(value & 0xFF);
}

unsigned int getSeq_t(unsigned char *buffer, int buf_size) {
    if (buf_size < 2) return 0;
    return static_cast<unsigned int>(buffer[1]);
}

// ------------------------------------------------------------
// LENGTH FIELD (16 bits: buffer[2] = MSB, buffer[3] = LSB)
// ------------------------------------------------------------
void setLen_t(unsigned char *buffer, int buf_size, unsigned int value) {
    if (buf_size < 4) return;
    buffer[2] = static_cast<unsigned char>((value >> 8) & 0xFF); // high byte
    buffer[3] = static_cast<unsigned char>(value & 0xFF);        // low byte
}

unsigned int getLen_t(unsigned char *buffer, int buf_size) {
    if (buf_size < 4) return 0;
    return (static_cast<unsigned int>(buffer[2]) << 8) |
           (static_cast<unsigned int>(buffer[3]));
}

// ------------------------------------------------------------
// CRC1 FIELD (32 bits: buffer[8]..buffer[11], big-endian)
// ------------------------------------------------------------
void setCRC1_t(unsigned char *buffer, int buf_size, unsigned long int value) {
    if (buf_size < 12) return;
    buffer[8]  = static_cast<unsigned char>((value >> 24) & 0xFF);
    buffer[9]  = static_cast<unsigned char>((value >> 16) & 0xFF);
    buffer[10] = static_cast<unsigned char>((value >> 8)  & 0xFF);
    buffer[11] = static_cast<unsigned char>(value & 0xFF);
}

unsigned long int getCRC1_t(unsigned char *buffer, int buf_size) {
    if (buf_size < 12) return 0;
    return (static_cast<unsigned long int>(buffer[8])  << 24) |
           (static_cast<unsigned long int>(buffer[9])  << 16) |
           (static_cast<unsigned long int>(buffer[10]) << 8)  |
           (static_cast<unsigned long int>(buffer[11]));
}
