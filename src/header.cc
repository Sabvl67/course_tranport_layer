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

}

uint16_t Header::getSeq() const {

    return 0;
}


void Header::setLength(uint16_t l) {

}

uint16_t Header::getLength() const {

    return 0;
}


void Header::setCRC(uint16_t c) {

}

uint16_t Header::getCRC() const {

    return 0;
}