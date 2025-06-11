#include "Memory.h"
#include <iostream>
#include <iomanip>

#define MEMORY_SIZE 268500000


using namespace std;

// Constructor initializes memory with zeros
Memory::Memory() : dataMemory(MEMORY_SIZE / 4, 0) {}

Memory::~Memory() {}

int32_t Memory::readMemory(int32_t address) const {
    if (address >= 0 && address < MEMORY_SIZE && address % 4 == 0) {
        return dataMemory[address / 4];
    } else {
        cerr << "ERROR: Memory read out of bounds or unaligned at address 0x" << hex << address << endl;
        return 0;
    }
}

void Memory::writeMemory(int32_t address, int32_t value) {
    if (address >= 0 && address < MEMORY_SIZE && address % 4 == 0) {
        dataMemory[address / 4] = value;
    } else {
        cerr << "ERROR: Memory write out of bounds or unaligned at address 0x" << hex << address << endl;
    }
}

void Memory::printMemory() const {
    for (size_t i = 0; i < dataMemory.size(); ++i) {
        if (dataMemory[i] != 0) {
            cout << "0x" << hex << setw(4) << setfill('0') << (i * 4)
                 << " : 0x" << setw(8) << dataMemory[i] << endl;
        }
    }
}


