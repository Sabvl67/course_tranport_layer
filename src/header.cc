#include "include.h"

using namespace std;

Header::Header(){
    buffer.assign(12,0);
}

// 1st byte - 1st 8 bits, type: bit0&1
void Header::setType(uint16_t t) {
    uint8_t mask = 0b00000011;
    t = t & mask;
    buffer[0] = buffer[0] & (~mask);
    buffer[0] = buffer[0] | t;
}

uint16_t Header::getType() const {
    uint16_t type = buffer[0] & 0b00000011;
    return type;
}

void Header::setW(uint16_t w) {
    uint8_t mask = 0b11111000;
    w = w & mask;
    buffer[0] = buffer[0] & (~mask);

    // shift left to position 3
    uint16_t shifted_bit = w << 3;
    
    buffer[0] = buffer[0] | shifted_bit;
}

uint16_t Header::getW() const {
    uint16_t window = buffer[0] & 0b11111000;
    window = window >> 3;
    return window;
}

void Header::setSeq(uint16_t seq) {
    uint16_t mask = 0b11111111;
    seq = seq & mask;
    buffer[1] = seq;
}

uint16_t Header::getSeq() const {
    return buffer[1];
}


void Header::setLength(uint16_t l) {
    buffer[2] = l & 0xFF;
    buffer[3] = (l >> 8) & 0xFF;
}

uint16_t Header::getLength() const {
    return buffer[2] | (buffer[3] << 8);
}


void Header::setCRC(uint32_t c) {
    buffer[8] = c & 0xFF;
    buffer[9] = (c >> 8) & 0xFF;
    buffer[10] = (c >> 16) & 0xFF;
    buffer[11] = (c >> 24) & 0xFF;
}

uint32_t Header::getCRC() const {
    return static_cast<uint32_t> (buffer[8])|
            (static_cast<uint32_t>(buffer[9]) << 8)|
            (static_cast<uint32_t>(buffer[10]) << 16)|
            (static_cast<uint32_t>(buffer[11]) << 24);

}