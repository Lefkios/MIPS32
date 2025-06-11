#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <string>
#include <bitset>

struct ControlSignals {
    bool RegDst, ALUSrc, MemToReg, RegWrite, MemRead, MemWrite, Branch, Jump, ALUOp1, ALUOp2;
    int PCSource;  // Ensure this is included
    int ALUSrcB;
    int ALUSrcA;
};

class ControlUnit {
private:
    ControlSignals signals;
    void resetSignals();

public:
    ControlUnit();
    ~ControlUnit();
    void decodeInstruction(const std::string &opcode);
    ControlSignals getSignals() const;

    // Getters for control signals
    bool getRegDst() const;
    bool getALUSrc() const;
    bool getMemToReg() const;
    bool getRegWrite() const;
    bool getMemRead() const;
    bool getMemWrite() const;
    bool getBranch() const;
    bool getJump() const;
    bool getALUOp1() const;
    bool getALUOp2() const;

    int getPCSource() const;
    int getALUSrcB() const;
    int getALUSrcA() const;
};

#endif

