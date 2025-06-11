#include "ControlUnit.h"
#include <iostream>

using namespace std;


ControlUnit::ControlUnit() {
    // Initialize all control signals to false.
    signals.RegDst    = false;
    signals.ALUSrc    = false;
    signals.MemToReg  = false;
    signals.RegWrite  = false;
    signals.MemRead   = false;
    signals.MemWrite  = false;
    signals.Branch    = false;
    signals.Jump      = false;
    signals.ALUOp1    = false;
    signals.ALUOp2    = false;
}

ControlUnit::~ControlUnit() { }

void ControlUnit::decodeInstruction(const std::string &opcode) {
    // Reset all control signals
    signals.RegDst    = false;
    signals.ALUSrc    = false;
    signals.MemToReg  = false;
    signals.RegWrite  = false;
    signals.MemRead   = false;
    signals.MemWrite  = false;
    signals.Branch    = false;
    signals.Jump      = false;
    signals.ALUOp1    = false;
    signals.ALUOp2    = false;

    // **R-type Instructions (ALUOp1 = true, ALUOp2 = false)**
    if (opcode == "add" || opcode == "addu" || opcode == "sub" || opcode == "subu" ||
        opcode == "and" || opcode == "or" || opcode == "nor" || opcode == "slt" || opcode == "sltu" ||
        opcode == "sll" || opcode == "srl") {

        signals.RegDst   = true;    // Destination is rd.
        signals.RegWrite = true;
        signals.ALUSrc   = false;   // Both operands from registers.
        signals.ALUOp1   = true;    // R-type operation
        signals.ALUOp2   = false;
    }

    // **I-type Immediate Arithmetic & Logical Instructions**
    else if (opcode == "addi" || opcode == "addiu" || opcode == "andi" || opcode == "ori" ||
             opcode == "slti" || opcode == "sltiu") {

        signals.ALUSrc   = true;    // Use immediate.
        signals.RegWrite = true;    // Write result to register.

        // Set ALU operation control signals
        signals.ALUOp1 = true;      // ALU operation (same for signed and unsigned versions)
        signals.ALUOp2 = (opcode == "sltiu"); // Differentiate signed vs unsigned (sltiu needs ALUOp2)
    }

    // **Branch Instructions (ALUOp1 = true, ALUOp2 = false)**
    else if (opcode == "beq" || opcode == "bne") {
        signals.Branch   = true;
        signals.ALUSrc   = false;   // Compare two registers.
        signals.ALUOp1   = true;    // ALU will perform subtraction.
        signals.ALUOp2   = false;
    }

    // **Load Word (lw)**
    else if (opcode == "lw") {
        signals.ALUSrc   = true;    // Use immediate for address calculation.
        signals.MemToReg = true;    // Data comes from memory.
        signals.RegWrite = true;    // Write loaded data into register.
        signals.MemRead  = true;    // Perform a memory read.
        signals.ALUOp1   = true;    // ALU performs addition.
        signals.ALUOp2   = false;
    }

    // **Store Word (sw)**
    else if (opcode == "sw") {
        signals.ALUSrc   = true;    // Use immediate for address calculation.
        signals.MemWrite = true;    // Write data from register to memory.
        signals.ALUOp1   = true;    // ALU performs addition.
        signals.ALUOp2   = false;
    }

    // **Jump Instruction**
    else if (opcode == "j") {
        signals.Jump = true;
    }

    // **Invalid Opcode Handling**
    else {
        std::cout << "Invalid operation: " << opcode << "\n";
    }
}

ControlSignals ControlUnit::getSignals() const {
    return signals;
}


// Getter functions
bool ControlUnit::getRegDst() const    { return signals.RegDst; }
bool ControlUnit::getALUSrc() const    { return signals.ALUSrc; }
bool ControlUnit::getMemToReg() const  { return signals.MemToReg; }
bool ControlUnit::getRegWrite() const  { return signals.RegWrite; }
bool ControlUnit::getMemRead() const   { return signals.MemRead; }
bool ControlUnit::getMemWrite() const  { return signals.MemWrite; }
bool ControlUnit::getBranch() const    { return signals.Branch; }
bool ControlUnit::getJump() const      { return signals.Jump; }
bool ControlUnit::getALUOp1() const    { return signals.ALUOp1; }
bool ControlUnit::getALUOp2() const    { return signals.ALUOp2; }
