#ifndef OBSERVER_H
#define OBSERVER_H
#include <iostream>
#include <string>
#include "Registers.h"
#include "Memory.h"
#include "ControlUnit.h"
#include "InstructionMemory.h"

using namespace std;

class Observer
{
    public:
        Observer();
        virtual ~Observer();
        void observe(const ControlUnit& controlUnit);
        void observeCycle(std::ostream& outFile,int state, int cycleCount, int PC,int PCold, const std::string& current,
                  const std::string& opcode, const std::string& rs,
                  const std::string& rt, const std::string& rd,
                  int value1, int value2,int immValue,int operand1,int operand2, int aluResult, int memoryValue,
                  const std::string& dest, ControlUnit& CU, Registers& registers, Memory& memory);

        //void observeCycle(int cycleCount, int PC, const string& current, const string& opcode, int aluResult, Registers& registers);
        void finalCycle(std::ostream& outFile,int cycleCount, int PC, const string& current, const string& opcode,
                        int aluResult, Registers& registers, Memory& memory,long long duration);


    protected:

    private:
         std::unordered_map<int,int> previousMemoryState;
         // One-cycle-behind variables
    int  prevPC       = 0;
    int  prevMDR      = 0;
    int  prevA        = 0;
    int  prevB        = 0;
    int  prevOp1      = 0;
    int  prevOp2      = 0;
    int  prevALURes   = 0;
    int  prevALUOut   = 0;

    bool firstCycle   = true;


};

#endif // OBSERVER_H
