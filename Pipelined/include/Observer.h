#ifndef OBSERVER_H
#define OBSERVER_H
#include <iostream>
#include <string>
#include "Registers.h"
#include "Memory.h"
#include "ControlUnit.h"
#include "InstructionMemory.h"
#include "PipeRegisters.h"

using namespace std;

class Observer
{
    public:
        Observer();
        virtual ~Observer();
        void observe(const ControlUnit& controlUnit);
        void observeCycle(ostream& outFile, int cycleCount, int nextPC, int PC,
                            const string& ifInstruction,
                            const string& idInstruction,
                            const string& exInstruction,
                            const string& memInstruction,
                            const string& wbInstruction,
                            const string& current, const string& opcode, const string& rs,
                            const string& rt, const string& rd, int value1, int value2,
                            int immValue, int operand1, int operand2, int aluResult,
                            int memoryValue, const string& dest, ControlUnit& CU,string target,
                            Registers& registers, Memory& memory, InstructionMemory& instrMem,PipeRegisters& pipe);

        //void observeCycle(int cycleCount, int PC, const string& current, const string& opcode, int aluResult, Registers& registers);
        void finalCycle(std::ostream& outFile,int PC,int cycleCount,Registers& registers, Memory& memory, long long duration);


    protected:

    private:
         std::unordered_map<int,int> previousMemoryState;


};

#endif // OBSERVER_H
