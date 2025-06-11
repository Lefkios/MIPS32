#include "InstructionMemory.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// Constructor: Initialize instruction memory
InstructionMemory::InstructionMemory() {
    instructions.clear(); //we ensure the vector is empty at start
    foundMain = false;
}

InstructionMemory::~InstructionMemory() {}

//trim function to remove leading and trailing spaces
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return ""; // Empty or only whitespace
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

// Remove comments (anything after #)
std::string removeComment(std::string line) {
    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos) {
        return line.substr(0, comment_pos);
    }
    return line;
}

int InstructionMemory::getLabelAddress(const std::string& label) const {
    auto it = labelMap.find(label);
    if (it != labelMap.end()) {
        return it->second;
    } else {
        std::cerr << "ERROR: Label '" << label << "' not found." << std::endl;
        exit(1);  // Exit if label not found
    }
}

bool InstructionMemory::loadInstructions(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (!inputFile) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int pc = 0;  // Program Counter to track instruction addresses

    while (std::getline(inputFile, line)) {
        line = removeComment(line);  // Remove comments
        line = trim(line);           // Trim whitespace

        // Start processing only after encountering 'main:'
        if (!foundMain) {
            if (line == "main:") {
                foundMain = true;
            }
            continue;  // Skip lines until 'main:' is found
        }

        // Skip empty lines
        if (line.empty()) continue;

        // Handle labels
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string label = line.substr(0, colonPos);  // Extract label name
            labelMap[label] = pc;  // Store label with current PC

            // Debugging label mapping
            std::cout << "DEBUG: Label '" << label << "' mapped to PC = " << pc << std::endl;

            line = line.substr(colonPos + 1);  // Remove label from line
            line = trim(line);  // Trim again in case of trailing instruction
        }

        // Stop parsing at the termination instruction
        if (line == "sll $zero, $zero, 0") break;

        // Store instructions after 'main:'
        if (!line.empty()) {
            instructions.push_back(line);
            pc += 4;  // Increment PC for each instruction
        }
    }

    inputFile.close();
    return true;
}


// Fetch an instruction at a specific PC
std::string InstructionMemory::getInstruction(int32_t PC) const {
    int index = PC / 4;
    if (index < instructions.size()) {
        return instructions[index];
    } else {
        return "";
    }
}

// Get total instruction count
size_t InstructionMemory::getInstructionCount() const {
    return instructions.size();
}

// Parse an instruction into operation and operands
void InstructionMemory::parseInstruction(const std::string& instruction, std::string& operation, std::string& operand1, std::string& operand2, std::string& operand3) const {
    std::string instr = removeComment(instruction);  // Remove comments
    instr = trim(instr);  // Trim spaces

    if (instr.empty()) return;  // Ignore empty lines

    std::istringstream iss(instr);
    iss >> operation;  // Extract operation

    // Handle jump instruction (single operand: label)
    if (operation == "j") {
        iss >> operand1;  // Only one operand for jump
        operand2.clear();
        operand3.clear();
    } else {
        // Extract operands for non-jump instructions (comma-separated)
        std::getline(iss, operand1, ',');
        std::getline(iss, operand2, ',');
        std::getline(iss, operand3);  // Get third operand if exists

        // Trim extracted operands
        operand1 = trim(operand1);
        operand2 = trim(operand2);
        operand3 = trim(operand3);

       // if ((operation == "sll" || operation == "srl") && operand3== "$zero") {
         //   operand3 = "0";
        //}
    }
}


