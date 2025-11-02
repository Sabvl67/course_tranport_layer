#include "include.h"

using namespace std;

Header::Header(){
    buffer.assign(12,0);
}

// Load header data from an external byte buffer
void Header::loadFromBuffer(unsigned char *buf, int buf_size) {
    // Copy byte buffer into our internal uint16_t vector
    // Each byte goes into one element of the vector
    for (int i = 0; i < buf_size && i < 12; i++) {
        buffer[i] = buf[i];
    }
}

// Write header data to an external byte buffer
void Header::writeToBuffer(unsigned char *buf, int buf_size) const {
    // Copy our internal vector back to the byte buffer
    for (int i = 0; i < buf_size && i < 12; i++) {
        buf[i] = buffer[i] & 0xFF;
    }
}

// 1st byte - bits 6-7 for type (2 bits)
void Header::setType_t(uint16_t t) {
    uint8_t mask = 0b11000000;  // bits 6-7
    t = (t & 0b00000011) << 6;  // mask to 2 bits and shift to position 6-7
    buffer[0] = buffer[0] & (~mask);  // clear bits 6-7
    buffer[0] = buffer[0] | t;  // set new type value
}

uint16_t Header::getType_t() const {
    uint16_t type = (buffer[0] & 0b11000000) >> 6;  // extract bits 6-7 and shift down
    return type;
}

// Window is bits 0-4 (5 bits)
void Header::setWin_t(uint16_t w) {
    uint8_t mask = 0b00011111;  // bits 0-4
    w = w & 0b00011111;  // mask to 5 bits (no shift needed)
    buffer[0] = buffer[0] & (~mask);  // clear bits 0-4
    buffer[0] = buffer[0] | w;  // set new window value
}

uint16_t Header::getWin_t() const {
    uint16_t window = buffer[0] & 0b00011111;  // extract bits 0-4 (no shift needed)
    return window;
}

void Header::setSeq_t(uint16_t seq) {
    uint16_t mask = 0b11111111;
    seq = seq & mask;
    buffer[1] = seq;
}

uint16_t Header::getSeq_t() const {
    return buffer[1];
}


// Length is 16-bit big-endian in bytes 2-3 (MSB first)
void Header::setLen_t(uint16_t l) {
    buffer[2] = (l >> 8) & 0xFF;  // high byte first (big-endian)
    buffer[3] = l & 0xFF;  // low byte second
}

uint16_t Header::getLen_t() const {
    return (buffer[2] << 8) | buffer[3];  // big-endian: high byte first
}


// CRC1 is 32-bit big-endian in bytes 8-11 (MSB first)
void Header::setCRC1_t(uint32_t c) {
    buffer[8] = (c >> 24) & 0xFF;  // most significant byte first
    buffer[9] = (c >> 16) & 0xFF;
    buffer[10] = (c >> 8) & 0xFF;
    buffer[11] = c & 0xFF;  // least significant byte last
}

uint32_t Header::getCRC1_t() const {
    return (static_cast<uint32_t>(buffer[8]) << 24) |
            (static_cast<uint32_t>(buffer[9]) << 16) |
            (static_cast<uint32_t>(buffer[10]) << 8) |
            static_cast<uint32_t>(buffer[11]);
}