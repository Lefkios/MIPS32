#include "ControlUnit.h"
#include <iostream>
#include <cstring>

using namespace std;

ControlUnit::ControlUnit()
    : RegDst(false), ALUSrcA(false), ALUSrcB(0), MemToReg(false),
      RegWrite(false), MemRead(false), MemWrite(false),
      Branch(false), ALUOp1(false), ALUOp2(false), Jump(false), IorD(false), IRWrite(false),
      PCWrite(false), PCWriteCond(false), PCSource(0)
{
    resetSignals();
    currentState = 0;  //start at instruction fetch
}

ControlUnit::~ControlUnit() {}

void ControlUnit::resetSignals() {
    RegDst = false;
    ALUSrcA = false;
    ALUSrcB = 0;
    Branch = false;
    MemRead = false;
    MemToReg = false;
    ALUOp1 = false;
    ALUOp2 = false;
    MemWrite = false;
    RegWrite = false;
    Jump = false;
    IorD = false;
    IRWrite = false;
    PCWrite = false;
    PCWriteCond = false;
    PCSource = 0;
}

void ControlUnit::nextState(string opcode) {
    resetSignals();

    // Instruction Fetch
    if (currentState == 0) {
        MemRead = true;
        IorD = false;
        IRWrite = true;
        ALUSrcA = false;
        ALUSrcB = 1;  //PC + 4
        ALUOp1 = false;
        ALUOp2 = false;
        PCWrite = true;
        PCSource = 0;
        currentState = 1;
    }
    // Instruction Decode/Register Fetch
    else if (currentState == 1) {
        ALUSrcA = false;
        ALUSrcB = 3;
        ALUOp1 = false;
        ALUOp2 = false;

        if (opcode == "lw" || opcode == "sw") {
            currentState = 2;  // lw/sw -> Address computation
        }
        else if (opcode == "add" || opcode == "addu" || opcode == "sub" || opcode == "subu" ||
                 opcode == "and" || opcode == "or" || opcode == "nor" || opcode == "slt" || opcode == "sltu" ||
                 opcode == "sll" || opcode == "srl"|| opcode == "jr") {
            currentState = 6;  // R-type -> Execute
        }

        else if (opcode == "beq" || opcode == "bne") {
            cout<<"BEQ and BEQ"<<endl;
            currentState =8;  // Branch -> Completion
        }
        else if (opcode == "j") {
            currentState = 9;  // Jump -> Completion
        }
        else if (opcode == "jal") {
            currentState = 12;  // jal Execution
        }
        else if (opcode == "addi" || opcode == "addiu" || opcode == "andi" || opcode == "ori" ||
                 opcode == "slti" || opcode == "sltiu" || opcode == "lui") {
            currentState = 10;  // I-Type Execution
        }
    }
    // Memory Address Computation
    else if (currentState == 2) {
        ALUSrcA = true;
        ALUSrcB = 2;  // Offset
        ALUOp1 = false;
        ALUOp2 = false;

        if (opcode == "lw") {
            currentState = 3;  // Move to memory read state
        } else {
            currentState = 5;  // sw (store word), move to memory write state
        }
    }
    // Memory Read (lw)
    else if (currentState == 3) {
        MemRead = true;
        IorD = true;
        currentState = 4;
    }
    // Memory Read Completion
    else if (currentState == 4) {
        RegDst = false;   // Write to `rt`, not `rd`
        RegWrite = true;
        MemToReg = true;
        currentState = 0;  // Return to Fetch
    }
    // Memory Write (sw)
    else if (currentState == 5) {
        MemWrite = true;
        IorD = true;
        currentState = 0;  // Return to Fetch
    }
    // R-Type Execution
    else if (currentState == 6) {
        ALUSrcA = true;
        ALUSrcB = 0;
        ALUOp1 = true;
        ALUOp2 = false;
        currentState = 7;
    }
    // R-Type Completion
    else if (currentState == 7) {
        RegDst = true;
        RegWrite = true;
        MemToReg = false;
        currentState = 0;  // Return to Fetch
    }
    // Branch Completion (beq, bne)
    else if (currentState == 8) {
        ALUSrcA = true;
        ALUSrcB = 0;
        ALUOp1 = false;
        ALUOp2 = true;
        PCWriteCond = true;
        PCSource = 1;
        currentState = 0; // Return to Fetch
    }
    // Jump Completion
    else if (currentState == 9) {
        PCWrite = true;
        PCSource = 2;
        currentState = 0;  // Return to Fetch
    }
    // I-Type Execution
    else if (currentState == 10) {
        ALUSrcA = true;
        ALUSrcB = 2;  // Immediate
        ALUOp1 = true;
        ALUOp2 = false;
        currentState = 11;
    }
    // I-Type Completion
    else if (currentState == 11) {
        RegDst = false;  // Write to `rt`
        RegWrite = true;
        MemToReg = false;
        currentState = 0;  // Return to Fetch
    }
    // jal Execution
    else if (currentState == 12) {
        RegWrite = true;
        PCWrite = true;
        PCSource = 2;
        currentState = 0;  // Return to Fetch
    }
}

int ControlUnit::getState() const {
    return currentState;
}

// Getters for control signals
bool ControlUnit::getRegDst() const { return RegDst; }
bool ControlUnit::getALUSrcA() const { return ALUSrcA; }
int ControlUnit::getALUSrcB() const { return ALUSrcB; }
bool ControlUnit::getMemToReg() const { return MemToReg; }
bool ControlUnit::getRegWrite() const { return RegWrite; }
bool ControlUnit::getMemRead() const { return MemRead; }
bool ControlUnit::getMemWrite() const { return MemWrite; }
bool ControlUnit::getJump() const { return Jump; }
bool ControlUnit::getBranch() const { return Branch; }
bool ControlUnit::getALUOp1() const { return ALUOp1; }
bool ControlUnit::getALUOp2() const { return ALUOp2; }
bool ControlUnit::getIorD() const { return IorD; }
bool ControlUnit::getIRWrite() const { return IRWrite; }
bool ControlUnit::getPCWrite() const { return PCWrite; }
bool ControlUnit::getPCWriteCond() const { return PCWriteCond; }
int ControlUnit::getPCSource() const { return PCSource; }
