#ifndef ALU_H
#define ALU_H
#include <string>

class ALU {
private:
    int op1;       //operand 1
    int op2;       //operand 2
    //int result;    //result of operation
union {
        int32_t signedResult;
        uint32_t unsignedResult;
    } result = {0};
    int operation; //operation code (0 = ADD, 1 = SUB, 2 = AND, 3 = OR)

public:
    ALU();           // Constructor
    virtual ~ALU();  // Destructor

    void setOperands(int operand1, int operand2); // Set operands
    int setOperation(std::string operation);               // Set operation code
    int getResult() const;                       // Get result
    int compute(std::string op,int ALUSrc, int op1, int op2);                            // Perform computation
};

#endif // ALU_H
