#include <iostream>
#include <bitset>
#include "include.h"

int main() {
    Header h;

    std::cout << "=== Testing Header Functions ===" << std::endl << std::endl;

    // Test Type (2 bits, values 0-3)
    std::cout << "Testing Type (2 bits):" << std::endl;
    h.setType(0);
    std::cout << "  setType(0) -> getType() = " << h.getType() << " (expected: 0)" << std::endl;
    h.setType(1);
    std::cout << "  setType(1) -> getType() = " << h.getType() << " (expected: 1)" << std::endl;
    h.setType(2);
    std::cout << "  setType(2) -> getType() = " << h.getType() << " (expected: 2)" << std::endl;
    h.setType(3);
    std::cout << "  setType(3) -> getType() = " << h.getType() << " (expected: 3)" << std::endl;
    h.setType(5);
    std::cout << "  setType(5) -> getType() = " << h.getType() << " (expected: 1, only 2 bits)" << std::endl;
    std::cout << std::endl;

    // Test Window (5 bits, values 0-31)
    std::cout << "Testing Window (5 bits):" << std::endl;
    h.setW(0);
    std::cout << "  setW(0) -> getW() = " << h.getW() << " (expected: 0)" << std::endl;
    h.setW(5);
    std::cout << "  setW(5) -> getW() = " << h.getW() << " (expected: 5)" << std::endl;
    h.setW(15);
    std::cout << "  setW(15) -> getW() = " << h.getW() << " (expected: 15)" << std::endl;
    h.setW(31);
    std::cout << "  setW(31) -> getW() = " << h.getW() << " (expected: 31)" << std::endl;
    h.setW(63);
    std::cout << "  setW(63) -> getW() = " << h.getW() << " (expected: 31, only 5 bits)" << std::endl;
    std::cout << std::endl;

    // Test Sequence (8 bits, values 0-255)
    std::cout << "Testing Sequence (8 bits):" << std::endl;
    h.setSeq(0);
    std::cout << "  setSeq(0) -> getSeq() = " << h.getSeq() << " (expected: 0)" << std::endl;
    h.setSeq(42);
    std::cout << "  setSeq(42) -> getSeq() = " << h.getSeq() << " (expected: 42)" << std::endl;
    h.setSeq(255);
    std::cout << "  setSeq(255) -> getSeq() = " << h.getSeq() << " (expected: 255)" << std::endl;
    h.setSeq(256);
    std::cout << "  setSeq(256) -> getSeq() = " << h.getSeq() << " (expected: 0, wraps around)" << std::endl;
    std::cout << std::endl;

    // Test Length (16 bits, values 0-65535)
    std::cout << "Testing Length (16 bits):" << std::endl;
    h.setLength(0);
    std::cout << "  setLength(0) -> getLength() = " << h.getLength() << " (expected: 0)" << std::endl;
    h.setLength(512);
    std::cout << "  setLength(512) -> getLength() = " << h.getLength() << " (expected: 512)" << std::endl;
    h.setLength(1234);
    std::cout << "  setLength(1234) -> getLength() = " << h.getLength() << " (expected: 1234)" << std::endl;
    h.setLength(65535);
    std::cout << "  setLength(65535) -> getLength() = " << h.getLength() << " (expected: 65535)" << std::endl;
    std::cout << std::endl;

    // Test CRC (32 bits)
    std::cout << "Testing CRC (32 bits):" << std::endl;
    h.setCRC(0);
    std::cout << "  setCRC(0) -> getCRC() = " << h.getCRC() << " (expected: 0)" << std::endl;
    h.setCRC(12345);
    std::cout << "  setCRC(12345) -> getCRC() = " << h.getCRC() << " (expected: 12345)" << std::endl;
    h.setCRC(0xDEADBEEF);
    std::cout << "  setCRC(0xDEADBEEF) -> getCRC() = 0x" << std::hex << h.getCRC() << std::dec << " (expected: 0xdeadbeef)" << std::endl;
    h.setCRC(4294967295);
    std::cout << "  setCRC(4294967295) -> getCRC() = " << h.getCRC() << " (expected: 4294967295)" << std::endl;
    std::cout << std::endl;

    // Test that fields don't interfere with each other
    std::cout << "Testing field independence:" << std::endl;
    Header h2;
    h2.setType(3);
    h2.setW(17);
    h2.setSeq(123);
    h2.setLength(456);
    h2.setCRC(789);
    std::cout << "  Set all fields -> Type=" << h2.getType()
              << ", W=" << h2.getW()
              << ", Seq=" << h2.getSeq()
              << ", Length=" << h2.getLength()
              << ", CRC=" << h2.getCRC() << std::endl;
    std::cout << "  Expected: Type=3, W=17, Seq=123, Length=456, CRC=789" << std::endl;

    return 0;
}
