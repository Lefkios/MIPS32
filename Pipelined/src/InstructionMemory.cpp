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

string InstructionMemory::getLabelName(int address) const {
    for (const auto& entry : labelMap) {
        if (entry.second == address) {
            return entry.first; // Return the label name if found
        }
    }
    return "-"; // If no label is found for the given addsress, return "-"
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
    int pc = 0;
    while (getline(inputFile, line)) {
        line = removeComment(line);
        line = trim(line);

        // Wait until we get to "main:" to start processing instructions
        if (!foundMain) {
            if (line == "main:")
                foundMain = true;
            continue;
        }

        // Process labels (allows for multiple labels on one line)
        size_t colonPos = line.find(':');
        while (colonPos != string::npos) {
            string label = trim(line.substr(0, colonPos));
            labelMap[label] = pc;
            cout << "DEBUG: Label '" << label << "' mapped to PC = " << pc << endl;
            // Remove the label and the colon from the line
            line = trim(line.substr(colonPos + 1));
            colonPos = line.find(':');
        }

        // If nothing remains after removing labels, skip this line.
        if (line.empty())
            continue;

        // Check for the special stop instruction
        if (line == "sll $zero, $zero, 0") {
            instructions.push_back(line);
            cout << "**Stored 'sll $zero, $zero, 0', will let simulator stop on execution.**\n";
            pc += 4;
            continue;
        }

        // Store the non-label instruction and update the PC.
        instructions.push_back(line);
        pc += 4;
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

void InstructionMemory::parseInstruction(const string& instruction, string& operation, string& rs,
                                         string& rt, string& rd) const {
    string instr = removeComment(instruction);
    instr = trim(instr);

    if (instr.empty()) return;

    istringstream iss(instr);
    iss >> operation;

    // Detect `sll $zero, $zero, 0`, execute it, and THEN stop fetching
    if (instr == "sll $zero, $zero, 0") {
        operation = "sll";
        rs = "$zero";
        rt = "$zero";
        rd = "0";
        cout << "**Executing 'sll $zero, $zero, 0' - Fetching stops AFTER this!**\n";
        return;
    }

    if (operation == "j" || operation == "jal" || operation == "jr") {
        iss >> rs;
        rs = trim(rs);
        rt.clear();
        rd.clear();
    }
else if (operation == "lw" || operation == "sw") {
    string rawRT, offsetBase;

    // 1. Extract rt (value to be loaded/stored)
    getline(iss, rawRT, ',');
    rt = trim(rawRT);

    // 2. Extract "offset(base)" part
    getline(iss, offsetBase);
    offsetBase = trim(offsetBase);

    size_t openParen = offsetBase.find('(');
    size_t closeParen = offsetBase.find(')');
    if (openParen != string::npos && closeParen != string::npos) {
        rd = offsetBase.substr(0, openParen);  // immediate (e.g., "0")
        rs = offsetBase.substr(openParen + 1, closeParen - openParen - 1);  // base (e.g., "$t4")

        // Clean/trim
        rd = trim(rd);
        rs = trim(rs);
    } else {
        cerr << "[ERROR] Malformed lw/sw: " << instruction << "\n";
        exit(1);
    }

    // Final check (optional but helpful)
    if (rt.empty() || rs.empty() || rd.empty()) {
        cerr << "[ERROR] lw/sw parse failed: " << instruction << "\n";
        exit(1);
    }
}

    else if (operation == "beq" || operation == "bne") {
    getline(iss, rs, ',');
    getline(iss, rt, ',');
    getline(iss, rd);
    rs = trim(rs);
    rt = trim(rt);
    rd = trim(rd);  // 'rd' holds the label name here

}


    else if (operation == "addi" || operation == "addiu" || operation == "andi" ||
             operation == "ori" || operation == "slti" || operation == "sltiu" || operation == "lui") {
        getline(iss, rt, ',');
        getline(iss, rs, ',');
        getline(iss, rd);
        rs = trim(rs);
        rt = trim(rt);
        rd = trim(rd);
        if (!rd.empty()) {
            if (rd.compare(0, 2, "0x") == 0 || rd.compare(0, 2, "0X") == 0) {
                rd = to_string(stoi(rd, nullptr, 16));
                cout<<rd<<"\n";
            } else {
                rd = to_string(stoi(rd));
            }
        }
    }
    else if (operation == "srl" || operation == "sll") {
    // sll $t0, $t1, 10
    //      rd    rt   shamt
    getline(iss, rd, ',');  // Destination register
    getline(iss, rt, ',');  // Source register
    getline(iss, rs);       // Shift amount (immediate)

    rd = trim(rd);
    rt = trim(rt);
    rs = trim(rs);  // rs is used to temporarily store shift amount

    // Overwrite `rs` with the immediate as a decimal string (not a register!)
    if (!rs.empty()) {
        try {
            rs = to_string(stoi(rs));  // e.g., "10"
        } catch (...) {
            cerr << "[ERROR] Invalid shift amount in: " << instruction << endl;
            rs = "0";
        }
    }
}


    else {
        getline(iss, rd, ',');
        getline(iss, rs, ',');
        getline(iss, rt);
        rs = trim(rs);
        rt = trim(rt);
        rd = trim(rd);
    }

}
