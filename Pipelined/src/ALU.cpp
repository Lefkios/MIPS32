#include "ALU.h"
#include<iostream>
using namespace std;

// Constructor
ALU::ALU():op1(0), op2(0), operation(0){
    result.signedResult=0;
    //operation(0) {}
}
// Destructor
ALU::~ALU() {}

int ALU::compute(std::string op, int ALUOp1, int ALUOp2, int op1, int op2) {
    int result = 0;

    if (ALUOp1 == 0 && ALUOp2 == 0) {  // ALUOp = 00 (Addition, used for lw/sw)
        result = op1 + op2;
    }
    else if (ALUOp1 == 0 && ALUOp2 == 1) {  // ALUOp = 01 (Subtraction, used for branches)
        result = op1 - op2;
    }
    else if (ALUOp1 == 1 && ALUOp2 == 0) {  // ALUOp = 10 (R-Type Instructions)

        if (op == "add" || op == "addi" || op == "jr") {
            result = op1 + op2;
        }
        else if (op == "addu") {
            result = static_cast<uint32_t>(op1) + static_cast<uint32_t>(op2);  // Unsigned addition
        }
        else if (op == "addiu") {
            uint32_t imm = static_cast<uint32_t>(op2) & 0xFFFF;  // Zero-extend immediate
            result = static_cast<uint32_t>(op1) + imm;
        }
        else if (op == "sub") {
            result = op1 - op2;
        }
        else if (op == "subu") {
            result = static_cast<uint32_t>(op1) - static_cast<uint32_t>(op2);  // Unsigned subtraction
        }
        else if (op == "and" || op == "andi") {
            result = op1 & (op == "andi" ? (op2 & 0xFFFF) : op2); // Immediate is zero-extended
        }
        else if (op == "or" || op == "ori") {
            result = op1 | (op == "ori" ? (op2 & 0xFFFF) : op2); // Immediate is zero-extended
            cout<<"\n\n"<<result<<"\n\n";
        }
        else if (op == "nor") {
            result = ~(op1 | op2);  // Bitwise NOR
        }
        else if (op == "slt") {
            result = (op1 < op2) ? 1 : 0;  // Signed comparison
        }
        else if (op == "slti") {
            result = (op1 < op2) ? 1 : 0;  // Signed comparison for immediate
        }
        else if (op == "sltiu") {
            uint32_t imm = static_cast<uint32_t>(op2) & 0xFFFF; // Zero-extend
            result = (static_cast<uint32_t>(op1) < imm) ? 1 : 0; // Unsigned comparison
        }
        else if (op == "sltu") {
            result = (static_cast<uint32_t>(op1) < static_cast<uint32_t>(op2)) ? 1 : 0; // Unsigned comparison
        }
        else if (op == "sll") {
            result = static_cast<uint32_t>(op1) << (op2 & 0x1F);  // Ensure shift amount is within range (5 bits)
        }
        else if (op == "srl") {
            result = static_cast<uint32_t>(op1) >> (op2 & 0x1F);  // Ensure shift amount is within range (5 bits)
        }
    }
    else {
        result = 0;  // Invalid ALUOp
    }

    return result;
}
