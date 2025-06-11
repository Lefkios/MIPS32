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
        void observeCycle(std::ostream& outFile,int cycleCount, int PC,int PCold, const std::string& current,
                  const std::string& opcode, const std::string& operand1,
                  const std::string& operand2, const std::string& operand3,
                  int value1, int value2, int aluResult, int memoryValue,
                  const std::string& dest, ControlUnit& CU, Registers& registers, Memory& memory);

        //void observeCycle(int cycleCount, int PC, const string& current, const string& opcode, int aluResult, Registers& registers);
        void finalCycle(std::ostream& outFile,int cycleCount, int PC, const string& current, const string& opcode, int aluResult, Registers& registers, Memory& memory);


    protected:

    private:
};

#endif // OBSERVER_H
