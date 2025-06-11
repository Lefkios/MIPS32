#include "InstructionMemory.h"
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

// Constructor: Initialize instruction memory
InstructionMemory::InstructionMemory() {
    instructions.clear(); //we ensure the vector is empty at start
    foundMain = false;
}

InstructionMemory::~InstructionMemory() {}

//trim function to remove leading and trailing spaces
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos) return ""; // Empty or only whitespace
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

// Remove comments (anything after #)
string removeComment(string line) {
    size_t comment_pos = line.find('#');
    if (comment_pos != string::npos) {
        return line.substr(0, comment_pos);
    }
    return line;
}

int InstructionMemory::getLabelAddress(const string& label) const {
    auto it = labelMap.find(label);
    if (it != labelMap.end()) {
        return it->second;
    } else {
        cerr << "ERROR: Label '" << label << "' not found." <<endl;
        exit(1);  // Exit if label not found
    }
}
bool InstructionMemory::loadInstructions(const string& filename) {
    ifstream inputFile(filename);
    if (!inputFile) {
        cerr << "Error: Could not open file: " << filename << endl;
        return false;
    }

    string line;
    int pc = 0;  // Program Counter to track instruction addresses

    while (getline(inputFile, line)) {
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
        if (colonPos != string::npos) {
            string label = line.substr(0, colonPos);  // Extract label name
            labelMap[label] = pc;  // Store label with current PC

            // Debugging label mapping
            cout << "DEBUG: Label '" << label << "' mapped to PC = " << pc << endl;

            line = line.substr(colonPos + 1);  // Remove label from line
            line = trim(line);  // Trim again in case of trailing instruction
        }

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
string InstructionMemory::getInstruction(int32_t PC) const {
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
void InstructionMemory::parseInstruction(const string& instruction, string& operation, string& rs,
                                           string& rt, string& rd) const {
    string instr = removeComment(instruction);  // Remove comments
    instr = trim(instr);  // Trim spaces

    if (instr.empty()) return;  // Ignore empty lines

    istringstream iss(instr);
    iss >> operation;  // Extract operation (opcode)

    // J-type instruction (jump)
    if (operation == "j" || operation == "jal"|| operation == "jr") {
        iss >> rs;  // Jump address (label)
        rs = trim(rs);
        cout << rs << endl;
        rt.clear();
        rd.clear();
    }

    // Load/store instructions (I-type) - lw and sw
    else if (operation == "lw" || operation == "sw") {
        string offsetBase;
        iss >> rt;  // Destination/source register (e.g., $t0)
        rt = trim(rt);
        if (!rt.empty() && rt.back() == ',') {
            rt.pop_back();
        }

        getline(iss >> ws, offsetBase);  // Read remaining part (offset(base)), ignore leading whitespace
        offsetBase = trim(offsetBase);  // Ensure it's properly trimmed

        // Extract offset and base register
        size_t openParen = offsetBase.find('(');
        size_t closeParen = offsetBase.find(')');
        if (openParen != string::npos && closeParen != string::npos) {
            rd = trim(offsetBase.substr(0, openParen));  // Extract Offset
            rs = trim(offsetBase.substr(openParen + 1, closeParen - openParen - 1));  // Extract Base Register
        } else {
            cerr << "Error: Malformed load/store instruction: " << instruction << std::endl;
            exit(1);
        }
    }
    // Branch Instructions (beq, bne) - Convert Label to Address
    else if (operation == "beq" || operation == "bne") {
        getline(iss, rs, ',');
        getline(iss, rt, ',');
        getline(iss, rd);

        // Trim spaces
        rs = trim(rs);
        rt = trim(rt);
        rd = trim(rd);

        // Convert label to instruction index
        if (!rd.empty()) {
            if (labelMap.find(rd) != labelMap.end()) {
                int targetAddress = labelMap.at(rd);  // Get absolute address
                rd =to_string(targetAddress);  // Store target address as string
            } else {
                cerr << "Error: Label '" << rd << "' not found!" << std::endl;
                exit(1);
            }
        }

        cout << "Parsed Branch Instruction: " << operation
             << " rs=" << rs
             << " rt=" << rt
             << " target address=" << rd << endl;
    }
    // Immediate-type Instructions (addi, addiu, andi, ori, slti, sltiu, lui)
    else if (operation == "addi" || operation == "addiu" || operation == "andi" ||
             operation == "ori" || operation == "slti" || operation == "sltiu" || operation == "lui") {
        getline(iss, rt, ',');
        getline(iss, rs, ',');
        getline(iss, rd);

        // Trim spaces
        rs = trim(rs);
        rt = trim(rt);
        rd = trim(rd);

        // Convert immediate (rd) to an integer.
        // If it begins with "0x" or "0X", interpret it as hexadecimal.
        int immValue = 0;
        if (!rd.empty()) {
            if (rd.compare(0, 2, "0x") == 0 || rd.compare(0, 2, "0X") == 0) {
                immValue =stoi(rd, nullptr, 16);
            } else {
                immValue =stoi(rd);
            }
            rd =to_string(immValue);
        }
    }
    else if (operation == "srl" || operation == "sll") {
        getline(iss, rd, ',');  // Destination register (rd)
        getline(iss, rt, ',');  // Source register (rt) (holds the value to be shifted)
        getline(iss, rs);       // Shift amount (shamt), but stored in `rs` during parsing

        // Trim spaces
        rt = trim(rt);  // Register to be shifted
        rd = trim(rd);  // Destination register
        rs = trim(rs);  // This actually holds the shift amount (shamt)

        // Convert shift amount to an integer
        if (!rs.empty()) {
            rs = to_string(stoi(rs));  // Store shift amount as a numeric string
        }
    }

    // R-type Instructions (add, sub, and, or, slt, etc.)
    else {
        getline(iss, rd, ',');  // Destination register
        getline(iss, rs, ',');  // First source register
        getline(iss, rt);       // Second source register

        // Trim spaces
        rs =trim(rs);
        rt =trim(rt);
        rd =trim(rd);
    }

    cout << "\n\nParsed Instruction: " << operation
         << " rs: " <<rs
         << " rt: "<<rt
         << " rd: "<<rd << "\n\n";
}

