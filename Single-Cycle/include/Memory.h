#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <cstdint>

class Memory {
public:
    Memory();  // Constructor
    ~Memory();  // Destructor

    int32_t readMemory(int32_t address) const;   // Read function for lw
    void writeMemory(int32_t address, int32_t value);  // Write function for sw
    void printMemory() const;  // Debug print function

private:
    std::vector<int32_t> dataMemory;  // Vector to simulate memory
};

#endif // MEMORY_H
