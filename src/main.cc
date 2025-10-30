#include <iostream>
#include <bitset>
#include "include.h"

int main() {
    Header h;

    // Test Type
    h.setType(2);
    std::cout << "Type set to 2, getType() = " << h.getType() << std::endl;

    // Test Window
    h.setW(5);
    std::cout << "Window set to 5, getWindow() = " << h.getW() << std::endl;

    return 0;
}
