#ifndef HEADER_H
#define HEADER_H

#include <vector>
#include <cstdint>
#include <array>

using namespace std;
class Header {
    private:
        vector<uint16_t> buffer;

    public:
    Header();

    // the first 2 bit 0 & 1 decide the type of msg. Here we use mask = ...11
    //  bc that's the bit 0&1 we care about for type.
    void setType(uint16_t t);
    uint16_t getType() const;

    // w is b3-b7, simialr to type but we need to shift after bitwise as it start at b3
    void setW(uint16_t w);
    uint16_t getW() const;

    void setSeq(uint16_t seq);
    uint16_t getSeq() const;

    void setLength(uint16_t l);
    uint16_t getLength() const;

    void setCRC(uint32_t c);
    uint32_t getCRC() const;

};

#endif