#include "Memory.h"
#include <iostream>
#include <iomanip>

// Constructor: nothing special for map-based memory
Memory::Memory() {
}

// Destructor
Memory::~Memory() {
}

// Read a value from memory, or 0 if address not found or unaligned
int32_t Memory::readMemory(int32_t address) const {
    // Check alignment (must be multiple of 4)
    if (address % 4 != 0) {
        std::cerr << "ERROR: Memory read unaligned at address 0x"
                  << std::hex << address << std::endl;
        return 0;
    }

    // Look up 'address' in the map. If not found, return 0
    auto it = dataMemory.find(address);
    return (it != dataMemory.end()) ? it->second : 0;
}

// Write a value to memory, erase from map if value = 0
void Memory::writeMemory(int32_t address, int32_t value) {
    // Check alignment (must be multiple of 4)
    if (address % 4 != 0) {
        std::cerr << "ERROR: Memory write unaligned at address 0x"
                  << std::hex << address << std::endl;
        return;
    }

    if (value == 0) {
        // Optionally erase zero entries to keep memory sparse
        dataMemory.erase(address);
    } else {
        dataMemory[address] = value;
    }
}

// Print all non-empty entries in the map
void Memory::printMemory() const {
    for (auto &entry : dataMemory) {
        std::cout << "0x" << std::hex << entry.first
                  << " : 0x" << entry.second << std::endl;
    }
}

const std::map<int, int>& Memory::getDataMemory() const {
    return dataMemory;
}
