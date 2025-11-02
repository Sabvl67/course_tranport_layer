// test/header_driver_t.cc

#include "header_driver_t.h"

// TYPE: bits 7..6 in buffer[0]
void setType_t(unsigned char *buffer, int /*buf_size*/, unsigned int value) {
    buffer[0] = (buffer[0] & 0x3F) | ((value & 0x3) << 6);
}

unsigned int getType_t(unsigned char *buffer, int /*buf_size*/) {
    return (buffer[0] >> 6) & 0x3;
}

// WINDOW: bits 4..0 in buffer[0]
void setWin_t(unsigned char *buffer, int /*buf_size*/, unsigned int value) {
    buffer[0] = (buffer[0] & ~0x1F) | (value & 0x1F);
}

unsigned int getWin_t(unsigned char *buffer, int /*buf_size*/) {
    return buffer[0] & 0x1F;
}

// SEQ: byte 1
void setSeq_t(unsigned char *buffer, int /*buf_size*/, unsigned int value) {
    buffer[1] = static_cast<unsigned char>(value & 0xFF);
}

unsigned int getSeq_t(unsigned char *buffer, int /*buf_size*/) {
    return buffer[1];
}

// LENGTH: bytes 2..3, big-endian
void setLen_t(unsigned char *buffer, int /*buf_size*/, unsigned int value) {
    buffer[2] = static_cast<unsigned char>((value >> 8) & 0xFF);
    buffer[3] = static_cast<unsigned char>( value       & 0xFF);
}

unsigned int getLen_t(unsigned char *buffer, int /*buf_size*/) {
    return (static_cast<unsigned int>(buffer[2]) << 8) |
            static_cast<unsigned int>(buffer[3]);
}

// CRC1: bytes 8..11, big-endian
void setCRC1_t(unsigned char *buffer, int /*buf_size*/, unsigned long int value) {
    buffer[8]  = static_cast<unsigned char>((value >> 24) & 0xFF);
    buffer[9]  = static_cast<unsigned char>((value >> 16) & 0xFF);
    buffer[10] = static_cast<unsigned char>((value >> 8)  & 0xFF);
    buffer[11] = static_cast<unsigned char>( value        & 0xFF);
}

unsigned long int getCRC1_t(unsigned char *buffer, int /*buf_size*/) {
    return  (static_cast<unsigned long int>(buffer[8])  << 24) |
            (static_cast<unsigned long int>(buffer[9])  << 16) |
            (static_cast<unsigned long int>(buffer[10]) << 8)  |
             static_cast<unsigned long int>(buffer[11]);
}
