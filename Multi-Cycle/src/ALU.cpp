#include "ALU.h"
#include<iostream>

// Constructor
ALU::ALU():op1(0), op2(0), operation(0){
    result.signedResult=0;
    //operation(0) {}
}
// Destructor
ALU::~ALU() {}

int ALU::compute(std::string op, int ALUOp1, int ALUOp2, int op1, int op2) {
    int result = 0;

    if (ALUOp1 == 0 && ALUOp2 == 0) { // ALUOp = 00 (Addition, used for lw/sw)
        result = op1 + op2;
    }
    else if (ALUOp1 == 0 && ALUOp2 == 1) {  // ALUOp = 01 (Subtraction, used for branches)
        result = op1 - op2;
    }
    else if (ALUOp1 == 1 && ALUOp2 == 0) {  // ALUOp = 10 (R-Type: Check function)
        if (op == "add"||op == "addi"||op=="jr")
            result = op1 + op2;
    else if (op == "addu") {
        result = static_cast<uint32_t>(op1) + static_cast<uint32_t>(op2);  // Unsigned addition
        std::cout << "ADDU: " << std::hex << op1 << " + " << std::hex << op2
              << " = " << std::hex << result << std::endl;
        }
    else if (op == "addiu") {
            result = static_cast<uint32_t>(op1) + static_cast<uint32_t>(op2);  // Unsigned immediate addition
            std::cout << "ADDIU: " << std::hex << op1 << " + " << std::hex << op2
                      << " = " << std::hex << result << std::endl;
        }

        else if (op == "sub"){
            result = op1 - op2;
            std::cout<<op1<<"    SUB: "<<op2<<" = "<<result;
        }
        else if (op == "subu")
            result = static_cast<uint32_t>(op1) - static_cast<uint32_t>(op2);  // Unsigned subtraction
        else if (op == "and")
            result = op1 & op2;
        else if (op == "andi")
            result = op1 & op2;
        else if (op == "or")
            result = op1 | op2;
        else if (op == "ori")
            result = op1 | op2;
        else if (op == "nor")
            result = ~(op1 | op2);  // Bitwise NOR: Inverts all bits of (op1 | op2)
        else if (op == "slt") {
            if (op1 < op2) {
                result = 1;  // Signed comparison
            } else {
                result = 0;
            }
        }
        else if (op == "slti") {
            if (op1 < op2) {
                result = 1;  // Signed comparison for immediate
            } else
                result = 0;
        }
        else if (op == "sltu")
            if (static_cast<uint32_t>(op1) < static_cast<uint32_t>(op2)) {
                result = 1;  //Unsigned comparison
            } else {
            result = 0;
        }
        else if (op == "sll"){
            result = (uint32_t)op1 << op2;  // Correct shift operation
        }

       else if (op == "srl"){
            result = (uint32_t)op1 >> op2;  // Correct shift operation
        }
    }
    else {
        result = 0;  //Invalid ALUOp
    }

    return result;
}



