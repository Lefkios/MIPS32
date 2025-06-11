#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "InstructionMemory.h"
#include "ALU.h"
#include "Registers.h"
#include "ControlUnit.h"
#include "Observer.h"
#include "Memory.h"

using namespace std;

int main(){
    ofstream outFile("output.txt",std::ios::trunc);

    if (!outFile) {
        cerr<<"Error opening output file!" <<endl;
        return 1;
    }

    outFile<<"MIPS32 Single-Cycle\n";

    InstructionMemory instrMem;
    ALU alu;
    Registers registers;
    ControlUnit CU;
    Observer observer;
    Memory memory;
    int32_t PC=0,PCold=0;
    int value1,value2,offset;
    string current;
    string opcode,operand1,operand2,operand3;
    string dest;
    int cycleCount=0,cycle1,cycle2,cycle3;
    int aluResult=0,memoryValue=0;
    bool branchTaken=0;


    //load instructions from file
    if (!instrMem.loadInstructions("input.s")) {
        return 1; //exit if file couldn't be opened
    }

    if (!outFile) {
        cerr << "Error opening output file!" <<endl;
        return 1;
    }

    //fetch-decode-execute Loop
    while(PC<instrMem.getInstructionCount()*4) {
        current=instrMem.getInstruction(PC);
        instrMem.parseInstruction(current,opcode,operand1,operand2,operand3);
        cout<<current<<endl;

        //set control signals based on opcode
        CU.setSignals(opcode);

        int ALUOp=(CU.getALUOp1()<<1)|CU.getALUOp2();

        if (CU.getBranch()) {
            value1=registers.getRegister(operand1); //first register
            value2=registers.getRegister(operand2); //second register

            // ALU comparison (subtract values)
            int aluBranchResult = alu.compute("sub", 0b10, value1, value2);

            branchTaken=0;

            //determine if branch should be taken
            if ((aluBranchResult == 0 && opcode == "beq")||(aluBranchResult != 0 && opcode == "bne"))
                branchTaken=1;

            if (branchTaken==1) {
                if (instrMem.labelMap.find(operand3)!=instrMem.labelMap.end()) {
                    PCold=PC;
                    PC=instrMem.labelMap[operand3]; //jump to the label's address

                    cout << "Branch Taken to Label '" << operand3 << "' at PC = " << PC << endl;
                } else {
                    cerr << "Label '" << operand3 << "' not found" << endl;
                    exit(1); //exit if the label doesn't exist
                }
            } else {
                PC+=4;  //move to the next instruction if branch not taken
                cout << "Branch NOT Taken. Moving to PC = "<<PC<<endl;
            }

            string temp1=operand3;
            string temp2=operand2;

            operand2=operand1;
            operand3=temp2;
            operand1=temp1;

            cycleCount++;

            observer.observeCycle(outFile,cycleCount, PC, PCold, current, opcode, operand1, operand2, operand3,value1, value2, aluResult, memoryValue, dest, CU, registers, memory);

            continue;  //skip the default PC increment outside this block

        }else if (CU.getJump()==true) {
            if (instrMem.labelMap.find(operand1) != instrMem.labelMap.end()) {
                PCold=PC;
                PC=instrMem.labelMap[operand1];  // Jump directly to the label's address
                cout << "Jumping to Label '" << operand1 << "' at PC = " << PC << endl;
            } else {
                cerr << "ERROR: Jump target label '" << operand1 << "' not found!" << endl;
                exit(1);  // Exit if the label doesn't exist
            }
            cycleCount++;

            observer.observeCycle(outFile,cycleCount, PC, PCold, current, opcode, operand1, operand2, operand3,value1, value2, aluResult, memoryValue, dest, CU, registers, memory);

            continue;  //skip PC increment after jump

        }else if  (opcode == "sw" || opcode == "lw") {
            size_t openBracket=operand2.find('(');
            size_t closeBracket=operand2.find(')');

            string offsetStr = operand2.substr(0,openBracket);  //get offset part
            string baseReg = operand2.substr(openBracket + 1, closeBracket - openBracket - 1);  // Get base register

            int offset = 0;
            try {
                offset=stoi(offsetStr);  //convert offset to integer
            } catch (const std::exception& e) {
                cerr << "ERROR: Invalid immediate value in memory instruction: " << offsetStr << endl;
                exit(1);
            }

            operand3 = baseReg;
            operand2 = operand1;


            value1 = registers.getRegister(baseReg);  // Get base register value
            aluResult = alu.compute(opcode, ALUOp, value1, offset);  // Compute memory address

            if (aluResult % 4 != 0) {
                cerr << "ERROR:Unaligned memory access at address " << aluResult << endl;
                exit(1);
            }

            if (opcode == "lw") {  // LOAD WORD
                memoryValue = memory.readMemory(aluResult);
                if (CU.getMemToReg()) {
                    registers.setRegister(operand1, memoryValue);  // Store into destination register
                }
            }

            if (opcode == "sw") {  // STORE WORD
                memory.writeMemory(aluResult, registers.getRegister(operand1));
            }
            operand1.clear();
        }else if (opcode == "sll" || opcode == "srl") {
            value1 = registers.getRegister(operand2);         // value to shift
            value2 = stoi(operand3);                          // shift amount (shamt)
            dest = operand1;

        if (opcode == "sll") {
            aluResult = value1 << value2;
        } else { // srl
            aluResult = static_cast<uint32_t>(value1) >> value2;  // logical shift
        }

        registers.setRegister(dest, aluResult);

        PC += 4;
        cycleCount++;
        observer.observeCycle(outFile, cycleCount, PC, PCold, current, opcode,
                              operand1, operand2, operand3, value1, value2,
                              aluResult, memoryValue, dest, CU, registers, memory);

        continue;
    }
    else if (opcode == "addi" || opcode == "addiu") {
        value1 = registers.getRegister(operand2);        // source register
        value2 = stoi(operand3, nullptr, 0);             // immediate value
        dest = operand1;

        if (opcode == "addi") {
            aluResult = value1 + value2;                 // signed addition
        } else {
            aluResult = static_cast<uint32_t>(value1) + static_cast<uint32_t>(value2);  // unsigned addition
        }

        registers.setRegister(dest, aluResult);

        PC += 4;
        cycleCount++;

    if (cycleCount == cycle1 || cycleCount == cycle2 || cycleCount == cycle3)
        observer.observeCycle(outFile, cycleCount, PC, PCold, current, opcode,
                              operand1, operand2, operand3, value1, value2,
                              aluResult, memoryValue, dest, CU, registers, memory);

        continue;
    }

    else{
            //get the first operand value from registers
            value1 = registers.getRegister(operand2);  //operand2 is the first register
            if (CU.getRegDst() == true)
                dest =operand1;  //R-type
            else{
                dest=operand1;  //I-type(no difference practically)
            }
            //get the second operand value (register or immediate)
            if (CU.getALUSrc()==true){
                value2 = stoi(operand3, nullptr, 0);  //Immediate value (handles hex and decimal)
            }else{
                value2 = registers.getRegister(operand3);  //Register value
            }
            //perform ALU computation with the selected values
            aluResult = alu.compute(opcode, ALUOp, value1, value2);

            if (CU.getRegWrite() == true) {
                registers.setRegister(dest, aluResult);  //store the result
            }



            // Perform ALU computation with the selected values
            aluResult = alu.compute(opcode, ALUOp, value1, value2);

            // Store result in destination register if RegWrite is enabled
            if (CU.getRegWrite() == true) {
                registers.setRegister(dest, aluResult);
            }
        }

        PC+=4;
        cycleCount++;

        observer.observeCycle(outFile,cycleCount, PC, PCold, current, opcode, operand1, operand2, operand3,value1, value2, aluResult, memoryValue, dest, CU, registers, memory);

    }
    //Final Output
    PC+=4;
    cycleCount++;
    observer.finalCycle(outFile,cycleCount, PC, current, opcode, aluResult, registers, memory);

    outFile.close();
    return 0;
}


