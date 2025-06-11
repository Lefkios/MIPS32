#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H
#include<string>


class ControlUnit
{
    public:
        ControlUnit();
        virtual ~ControlUnit();
        void resetSignals();
        void setSignals(const std::string operation);

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

    protected:

    private:
    // Control signals
    bool RegDst;
    bool ALUSrc;
    bool MemToReg;
    bool RegWrite;
    bool MemRead;
    bool MemWrite;
    bool Branch;
    bool Jump;
    bool ALUOp1, ALUOp2;
};

#endif // CONTROLUNIT_H
