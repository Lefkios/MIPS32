#include "ALU.h"
#include<iostream>

// Constructor
ALU::ALU():op1(0), op2(0), operation(0){
    result.signedResult=0;
    //operation(0) {}
}
// Destructor
ALU::~ALU() {}

int ALU::compute(std::string op,int ALUSrc, int op1, int op2) {
    int result;
    if(ALUSrc == 0b10){  //R-Type
        if((op=="addu")||(op=="addiu"))
            result=static_cast<uint32_t>(op1)+static_cast<uint32_t>(op2);
        else if (op=="subu")
            result=static_cast<uint32_t>(op1)-static_cast<uint32_t>(op2);
        else if (op=="sub"||op == "beq" || op== "bne")
            result=op1-op2;
        else if (op == "slt" || op == "slti") {
            if (op1 < op2) {  // Signed comparison
                return 1;
            } else {
                return 0;
            }
        }
        else if (op == "sltu" || op == "sltiu") {
            if ((unsigned)op1 < (unsigned)op2) {  // Unsigned comparison
                return 1;
            } else {
                return 0;
            }
        }
        else
            result=op1+op2;
    }
    else if (ALUSrc == 0b01) {  //or operation
            result=op1|op2;
    }
    else if (ALUSrc==0b00){  //AND operation
        result=op1&op2;
    }
    else if (ALUSrc==0b11) {  //sll or srl operation
        if (op == "sll") {
            return op2 << op1;  // Logical shift left
        }
        else if (op == "srl") {
            return (unsigned)op2 >> op1;  // Logical shift right
        }
    }
    else {
        result=0;  //invalid ALUSrc
    }
    return result;
}


