#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
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

    outFile<<"MIPS32 Multi-Cycle\n";

    InstructionMemory instrMem;
    ALU alu;
    Registers registers;
    ControlUnit CU;
    Observer observer;
    Memory memory;
    int32_t PC=0,PCold=0,targetPC;
    int nextPC=0,linkAddress ;
    int value1,value2,offset;
    string current;
    string opcode,rs,rt,rd;
    string dest;
    int cycleCount=0;
    int aluResult=0,memoryValue=0;
    bool branchTaken=0;
    int immValue,branchOffset;
    int currentState=0;  // Track which stage of execution we're in
    bool InstrComplete=true;  // Track when to fetch a new instruction
    unordered_map<string, int> labelMap;  // Declare label map globall
    int operand1 = 0, operand2 = 0;


    //load instructions from file
    if (!instrMem.loadInstructions("input.txt")) {
        return 1; //exit if file couldn't be opened
    }

    if (!outFile) {
        cerr << "Error opening output file!" <<endl;
        return 1;
    }

    using namespace std::chrono;
    auto start = high_resolution_clock::now();  // Start time measurement


    //fetch-decode-execute Loop
    while (PC<instrMem.getInstructionCount()*4) {
        if (InstrComplete) {
            PCold=PC;
            // Fetch new instruction
            current = instrMem.getInstruction(PC);
            instrMem.parseInstruction(current, opcode, rs, rt, rd);
            cout << "Fetched instruction: " << current << endl;

            // Reset control unit and prepare for execution
            CU.resetSignals();
            InstrComplete = false;
        }
        // Fetch Control Signals and Track Current State
        currentState=CU.getState();
        CU.nextState(opcode);

        cout << "Executing state: " << currentState << " for instruction: " << opcode << endl;

        //state 0,1 for all the instructions
        // Instruction Fetch
        if (currentState == 0) {
            registers.setA(0);
            registers.setB(0);
            registers.setALUOut(0);
            registers.setMDR(0);

            registers.setIR(instrMem.getInstruction(PC));  // Store in IR
            operand1=PC;
            operand2=4;
            registers.setALUOut(PC+4);  //Save PC+4 for later
            nextPC=registers.getALUOut();
        }
        //Decode/Register Fetch
        else if (currentState == 1) {

            //Jump Instruction
            if (opcode == "j"|| opcode == "jal") {
                cout << "Jump Instruction Detected in Decode: '" << rs << "'" << endl;

                //check if label exists
                if (instrMem.labelMap.find(rs) != instrMem.labelMap.end()) {
                    int targetAddress = instrMem.getLabelAddress(rs); // Retrieve the label's address
                     if (opcode == "jal") {
                        linkAddress = PCold + 4;  //PCold is the PC before the jump
                    }
                    cout << "Jump Target PC Address: " << targetAddress << endl;
                    operand2=targetAddress;
                    registers.setALUOut(targetAddress);  //store jump target in ALUOut
                    nextPC=targetAddress;
                }else {
                    cerr << "ERROR: Label '" << rs << "' not found in labelMap!" << endl;
                    exit(1);  // Exit if label not found
                }
            }else if (opcode == "lui") {
                int immValue = 0;
                if (rs.find("0x") == 0 || rs.find("0X") == 0) {
                    immValue = stoi(rs, nullptr, 16);
                }else {
                    immValue = stoi(rs);
                }
                registers.setB(immValue);
            }else if (opcode == "srl" || opcode == "sll") {
                cout << "RD: " << rd << endl;

                // Convert shift amount directly (rd stores the shift amount)
                int shamt = stoi(rs, nullptr, 0);

                cout << "SHAMT: " << shamt << endl;
                registers.setB(shamt);  // Store shift amount in B

                // Load value to be shifted from rt (source register)
                registers.setA(registers.getRegister(rt));
            }else{
                registers.setA(registers.getRegister(rs));

                //for I-Type we use immediate value instead of rt
                if (opcode == "addi" || opcode == "addiu" || opcode == "andi" || opcode == "ori" ||
                    opcode == "slti" || opcode == "sltiu" || opcode == "lui") {
                    immValue = stoi(rd, nullptr, 0);

                    registers.setB(immValue);  //Immediate stored in B

                }else if (opcode == "beq" || opcode == "bne") {
                    registers.setB(registers.getRegister(rt));  // Load rt value
                    cout << rs << " and " << rt << endl;
                    branchOffset = stoi(rd, nullptr, 0);
                    cout << "Branch Offset: " << branchOffset << endl;
                    // Store the branch offset (for later use in state 8)
                    operand2=branchOffset;
                    registers.setALUOut(branchOffset);

                // Handle Load (`lw`) and Store (`sw`) instructions
                }else if (opcode == "lw" || opcode == "sw") {
                    immValue = stoi(rd, nullptr, 0);  // Convert offset to integer
                    registers.setB(immValue);  // Store offset (immediate value) in B


                //registers.setALUOut(computedAddress);  // Store computed memory address in ALUOut

                }else if (opcode == "add" || opcode == "addu" || opcode == "sub" || opcode == "subu" ||
                    opcode == "and" || opcode == "or" || opcode == "nor" || opcode == "slt" || opcode == "sltu") {
                    registers.setB(registers.getRegister(rt));  // Load rt into B
                }

            }
        // Memory Address Calculation
        }else if (currentState == 2) {
            operand1=registers.getA();
            operand2=stoi(rd);
            registers.setALUOut(registers.getA() + stoi(rd));  // Compute Address

        // Memory Read or Mmeory write
        }else if (currentState == 3) {
            if (CU.getMemRead()) {  // Load Word
                registers.setMDR(memory.readMemory(registers.getALUOut()));
                cout<<"LOADED WORD in mdr: "<<memory.readMemory(registers.getALUOut());
            }
            else if (CU.getMemWrite()) {  // Store Word
                memory.writeMemory(registers.getALUOut(), registers.getRegister(rt));
            }
        }

        // Memory Read Completion
        else if (currentState == 4) {
            if (CU.getMemToReg()==true) {
                registers.setRegister(rt, registers.getMDR());  // Write MDR value to Register
                cout<<"LOADED WORD:"<<registers.getMDR()<<" STO "<<rt;
            }
        }
        // Memory Write
        else if (currentState == 5) {  // Memory Write
            cout << registers.getRegister(rt) << " ALU OUT  :" << registers.getALUOut();
            memory.writeMemory(registers.getALUOut(), registers.getRegister(rt));  // Store value in memory
        }

        // R-Type Execution
       else if (currentState == 6) {
            cout << "Executing ALU operation for R-Type instruction: " << opcode << endl;
            operand1= registers.getA();
            operand2= registers.getB();
            // Perform ALU Operation
            registers.setALUOut(alu.compute(opcode, CU.getALUOp1(), CU.getALUOp2(), registers.getA(), registers.getB()));
        }

        // R-Type Completion
        else if (currentState == 7) {
            if (opcode == "jr") {
                // For jr, update the PC with the value in ALUOut (or register A)
                cout << "JR instruction: Updating PC with value from ALUOut." << endl;
                targetPC = registers.getALUOut();
                nextPC=targetPC;
                cout << "PC updated to: " << targetPC << endl;
            } else {
                cout << "RegWrite signal: " << CU.getRegWrite() << endl;
                if (CU.getRegWrite() == true) {  // Only write if RegWrite is enabled
                    registers.setRegister(rd, registers.getALUOut());  // Write ALUOut to rd
                    cout << "Register " << rd << " updated with value: " << registers.getALUOut() << endl;
                }
            }
        }

        // Branch Completion (beq, bne)
        else if (currentState == 8) {
            cout << "BRANCH EXECUTION" << endl;
            // Load register values for comparison
            value1 = registers.getA();
            value2 = registers.getB();

            targetPC = std::stoi(rd);  // Convert string to integer

            // Perform branch decision
            if ((opcode == "beq" && value1 == value2) || (opcode == "bne" && value1 != value2)) {
                cout << "Branch Taken! Jumping to PC: " << targetPC << endl;
                nextPC=targetPC;
            } else {
                cout << "Branch Not Taken" << endl;
            }

        }
        else if (currentState == 9) {  // Jump Completion
            cout << "Executing JUMP Instruction" << endl;
                targetPC=registers.getALUOut();  // Use the stored jump target PC
                cout << "Jump Taken! Jumping to address: " << hex << PC << endl;
        }
        else if (currentState == 10) {  // I-Type Execution
            if (opcode == "lui") {
                // Shift it left 16 bits.
                int shiftedValue = registers.getB() << 16;

                // Put that in ALUOut (the standard place to hold I-type results).
                registers.setALUOut(shiftedValue);
            }
            // Perform ALU Operation Using A(rs) and B(Immediate)
            else{
                operand1 = registers.getA();
                operand2 = registers.getB();
                registers.setALUOut(alu.compute(opcode, CU.getALUOp1(), CU.getALUOp2(), registers.getA(), registers.getB()));
                cout<<"ALU: "<< CU.getALUOp1()<< CU.getALUOp2()<<"  "<<registers.getA()<<registers.getB()<<"    ";
                cout<<alu.compute(opcode, CU.getALUOp1(), CU.getALUOp2(), registers.getA(), registers.getB());
            }
        }
        else if (currentState == 11) {  // I-Type Completion

            if (CU.getRegWrite()) {
            registers.setRegister(rt, registers.getALUOut());  //Write ALUOut to rt
            cout << "Register " << rt << " updated with value: " << registers.getALUOut() << endl;
        }
       }else if (currentState == 12) {  // jal Execution
            cout << "Executing JAL Instruction" << endl;
            registers.setRegister("$ra", linkAddress);
            cout << "Link register ($ra) updated with: " << linkAddress << endl;
            targetPC = registers.getALUOut();  // jump target address
            PC = targetPC;
            cout << "JAL: PC updated to address: " << targetPC << endl;
            }


        cout<<"\n\n\n";
        // Increment the cycle count per execution
        cycleCount++;
        cout << "Cycle: " << cycleCount
            << " | State: " << currentState
            << " | Instruction: " << opcode
            << " | IR: " << registers.getIR()
            << " | MDR: " << registers.getMDR()
            << " | A: " << dec << registers.getA()
            << " | B: " << dec << registers.getB()
            << " | ALUOut: " << dec << registers.getALUOut()
            << " | PC: " << dec << PC << endl;


        // Advance to the next instruction when complete
        if (CU.getState() == 0&& !InstrComplete) {
            InstrComplete = true;
            PC=nextPC;
            if (current == "sll $zero, $zero, 0"){
                break;
            }
        }
            cout<<"CURRENT PC:  "<<PC<<"NEXT PC: "<<nextPC;


       observer.observeCycle(outFile,currentState,cycleCount,nextPC, PC , current, opcode, rs, rt, rd,value1, value2,immValue,operand1,operand2, aluResult, memoryValue, dest, CU, registers, memory);
    }
    auto end = high_resolution_clock::now();  // End time measurement
    long long duration = duration_cast<nanoseconds>(end - start).count();  //convert to nanoseconds


    //Final Output
    observer.finalCycle(outFile,cycleCount, PC, current, opcode, aluResult, registers, memory,duration);

    outFile.close();
    return 0;
}
