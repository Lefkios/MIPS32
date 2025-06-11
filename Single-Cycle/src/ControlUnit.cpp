#include "ControlUnit.h"
#include <iostream>
#include <cstring>

using namespace std;

ControlUnit::ControlUnit()
    : RegDst(false), ALUSrc(false), MemToReg(false),
      RegWrite(false), MemRead(false), MemWrite(false),
      Branch(false), ALUOp1(false), ALUOp2(false),Jump(false)
{
    resetSignals();
}

ControlUnit::~ControlUnit(){
}

    void ControlUnit::resetSignals() {
        RegDst = false;
        Branch = false;
        MemRead = false;
        MemToReg = false;
        ALUOp1=0;
        ALUOp2=0;
        MemWrite = false;
        ALUSrc = false;
        RegWrite = false;
        Jump = false;
    }

void ControlUnit::setSignals(const string operation) {
    resetSignals();

    if(operation=="add"||operation=="addi"||operation=="addu"||operation=="addiu"){
        //set destination register for R-type instructions
        if(operation =="add"||operation=="addu"){
            RegDst=true;
        }
        //ALUSrc is enabled for immediate operations
        if(operation=="addi"||operation=="addiu"){
            ALUSrc=true;
        }
        RegWrite=true;
        //ALU operation:ADD
        ALUOp1=1;
        ALUOp2=0;
    }
    //we also subtract for branch instr
    else if (operation=="sub"||operation=="subu"||operation == "beq" || operation == "bne") {
        ALUSrc = false;
        //register write only for subtraction (not for branch)
        if (operation == "sub" || operation == "subu") {
            RegWrite = true;
        }
        // Branch signal enabled for branch instructions
        if (operation == "beq" || operation == "bne") {
            Branch = true;
        }
        // ALU operation:ADD
        ALUOp1=1;
        ALUOp2=0;
    }
    else if (operation == "and" || operation == "andi") {
        //ALUSrc is enabled for immediate operations
        if (operation=="andi") {
            ALUSrc=true;
        }
        // These instructions always write to a register
        RegWrite=true;

        // ALU operation: AND
        ALUOp1 = 0;
        ALUOp2 = 0;
    }

    else if (operation == "or"||operation=="ori") {
        if (operation=="ori") {
            ALUSrc=true;
        }
        if (operation == "nor") {
        RegWrite = true;
        ALUOp1 = 0;
        ALUOp2 = 0;
    }
    else{
        RegWrite=true;
        ALUOp1=0;
        ALUOp2=1;
    }

    }
    else if (operation == "slt" || operation == "slti" || operation == "sltiu" || operation == "sltu") {
        if (operation == "slt" || operation == "sltu") {
            RegDst = true;
        }
        if (operation == "slti" || operation == "sltiu") {
            ALUSrc = true;
        }
        RegWrite = true;
        ALUOp1 =1;
        ALUOp2 =0;
    }

    else if (operation == "lw") {
        ALUSrc = true;     // Immediate used for address computation
        MemToReg = true;   // Load from memory to register
        RegWrite = true;   // Store result in register
        MemRead = true;    // Read from memory
        MemWrite=false;

        // ALU operation:ADD
        ALUOp1=1;
        ALUOp2=0;
    }
    else if (operation == "sw") {
        ALUSrc = true;     // Immediate used for address computation
        MemWrite = true;   //store value in memory

        // ALU operation: ADD
        ALUOp1=1;
        ALUOp2=0;
    }
    else if (operation == "sll") {
        RegDst = true;  // Destination is rd
        ALUSrc = true;  // Uses immediate shift amount
        RegWrite = true; // Writes to rd
        MemToReg = false;
        MemRead = false;
        MemWrite = false;
        Branch = false;
        ALUOp1=1;
        ALUOp2=1;
    }
    else if (operation == "srl") {
        RegDst = true;
        ALUSrc = true;
        RegWrite = true;
        ALUOp1 = 1;
        ALUOp2 = 0;  // ALU control for SRL (shift right)
    }
    else if (operation == "j") {
    Jump = true;        // Enable the jump control signal
    RegDst = false;     // No register destination
    ALUSrc = false;     // No ALU source needed
    MemRead = false;    // No memory read
    MemWrite = false;   // No memory write
    Branch = false;     // Not a conditional branch
    MemToReg = false;   // No memory to register operation
    RegWrite = false;   // No register write
    ALUOp1 = 0;         // ALU operation not needed
    ALUOp2 = 0;
}


    else {
        std::cout << "Invalid operation: " << operation << "\n";
        ALUOp1 = 0;
        ALUOp2 = 0;
    }
}


// Getters for control signals
bool ControlUnit::getRegDst() const { return RegDst; }
bool ControlUnit::getALUSrc() const { return ALUSrc; }
bool ControlUnit::getMemToReg() const { return MemToReg; }
bool ControlUnit::getRegWrite() const { return RegWrite; }
bool ControlUnit::getMemRead() const { return MemRead; }
bool ControlUnit::getMemWrite() const { return MemWrite; }
bool ControlUnit::getJump() const { return Jump; }
bool ControlUnit::getBranch() const { return Branch; }
bool ControlUnit::getALUOp1() const { return ALUOp1; }
bool ControlUnit::getALUOp2() const { return ALUOp2; }

