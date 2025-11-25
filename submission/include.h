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

    // Load header data from an external byte buffer
    void loadFromBuffer(unsigned char *buf, int buf_size);

    // Write header data to an external byte buffer
    void writeToBuffer(unsigned char *buf, int buf_size) const;

    // Type field: bits 6-7 of byte 0 (2 bits, values 0-3)
    void setType_t(uint16_t t);
    uint16_t getType_t() const;

    // Window field: bits 0-4 of byte 0 (5 bits, values 0-31)
    void setWin_t(uint16_t w);
    uint16_t getWin_t() const;

    // Sequence field: byte 1 (8 bits, values 0-255)
    void setSeq_t(uint16_t seq);
    uint16_t getSeq_t() const;

    // Length field: bytes 2-3 big-endian (16 bits, values 0-65535)
    void setLen_t(uint16_t l);
    uint16_t getLen_t() const;

    // CRC1 field: bytes 8-11 big-endian (32 bits)
    void setCRC1_t(uint32_t c);
    uint32_t getCRC1_t() const;

};

#endif