#ifndef MEMORY_H
#define MEMORY_H

#include <map>

class Memory {
private:
    // Private map for addresses -> values
    std::map<int, int> dataMemory;

public:
    // Constructor, destructor
    Memory();
    ~Memory();

    // Existing methods
    int readMemory(int address) const;
    void writeMemory(int address, int value);
    void printMemory() const;

    const std::map<int, int>& getDataMemory() const;

};

#endif

