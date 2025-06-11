#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <string>

class ControlUnit
{
public:
    ControlUnit();
    virtual ~ControlUnit();
    int getState() const;
    void resetSignals();
    void nextState(const std::string opcode);  // Updates state based on opcode

    // Getters for control signals
    bool getRegDst() const;
    bool getALUSrcA() const;
    int getALUSrcB() const;
    bool getMemToReg() const;
    bool getRegWrite() const;
    bool getMemRead() const;
    bool getMemWrite() const;
    bool getBranch() const;
    bool getJump() const;
    bool getALUOp1() const;
    bool getALUOp2() const;
    bool getIorD() const;
    bool getIRWrite() const;
    bool getPCWrite() const;
    bool getPCWriteCond() const;
    int getPCSource() const;

protected:

private:
    int currentState; // FSM current stat

    // Control signals
    bool RegDst;
    bool ALUSrcA;
    int ALUSrcB;
    bool MemToReg;
    bool RegWrite;
    bool MemRead;
    bool MemWrite;
    bool Branch;
    bool Jump;
    bool ALUOp1;
    bool ALUOp2;
    bool IorD;
    bool IRWrite;
    bool PCWrite;
    bool PCWriteCond;
    int PCSource;
};

#endif // CONTROLUNIT_H

