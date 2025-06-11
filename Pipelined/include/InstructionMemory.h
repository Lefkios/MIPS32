#ifndef INSTRUCTION_MEMORY_H
#define INSTRUCTION_MEMORY_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

using namespace std;

class InstructionMemory {
private:
    vector<string> instructions;
    bool foundMain;

public:
    InstructionMemory();  // Constructor
    ~InstructionMemory(); // Destructor

    string getLabelName(int address) const;
    bool loadInstructions(const string& filename);
    string getInstruction(int32_t PC) const;
    size_t getInstructionCount() const;
    void parseInstruction(const string& instruction, string& operation, string& rs, string& rt, string& rd) const;
    int getLabelAddress(const std::string& label) const;
    std::unordered_map<std::string, int> labelMap;  // Maps label to instruction address
};

string trim(const string& str);

#endif // INSTRUCTION_MEMORY_H
