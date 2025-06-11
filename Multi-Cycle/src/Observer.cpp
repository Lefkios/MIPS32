#include "Observer.h"
#include <fstream>
#include <bitset>

using namespace std;

Observer::Observer()
{
    //ctor
}

Observer::~Observer()
{
    //dtor
}

void Observer::observeCycle(ostream& outFile,int state,int cycleCount,int nextPC,int PC,const string& current,const string& opcode,const string& rs,
                            const string& rt,const string& rd,int value1,int value2,int immValue,int operand1,int operand2,int aluResult,int memoryValue,
                            const string& dest,ControlUnit& CU,Registers& registers,Memory& memory){

    outFile << "\n-----Cycle " << dec << cycleCount << "-----\n";

    // Registers Section
    outFile << "Registers:\n";
    outFile<< hex << nouppercase << nextPC << dec << "\t";
    registers.printRegisters(outFile);

    //Monitors Section
    outFile << "\n\nMonitors:\n";

    // 1. PC (one cycle behind)
    if(state==0)
        outFile << hex << nouppercase << PC << "\t";
    else
        outFile  << hex << nouppercase << nextPC << "\t";


    // 2. Memory Address being accessed (current cycle)
    //outFile  << hex << registers.getALUOut << "\t";
    outFile  << hex << PC << "\t";


    // 3. Memory Write Data (current cycle)
    if (CU.getMemWrite()) {
        outFile << hex << registers.getA() << "\t";
    } else {
        outFile << "-\t";
    }

    // 4. Memory Read Data (current cycle)
    if ((opcode == "lw"&& state==4 )||( opcode == "sw" && state==3)) {
        outFile << hex << memoryValue << "\t";
    } else {
        outFile << current << "\t";
    }

    // 5, 6, 7, 8: Instruction Register Fields (current cycle)
    outFile << opcode << "\t";
    if(opcode=="sll" || opcode=="srl")
            outFile <<"-\t";
    else
        outFile << rs << "\t";
    outFile << rt << "\t";
    if (opcode == "addi" || opcode == "addiu" || opcode == "andi" ||
        opcode == "ori"  || opcode == "slti"  || opcode == "sltiu" ||
        opcode == "lui"  || opcode == "beq"   || opcode == "bne"   ||
        opcode == "lw"   || opcode == "sw")
    {
        outFile << immValue << "\t";
    }else if(opcode=="sll"||opcode=="srl")
        outFile<<rs<<"\t";
     else {
        outFile << "-\t";
    }

    if (state==0){
            outFile<<"-\t-\t-\t-\t-\t-\t-\t-\t0\t"<<PC<<"\t4\t-\t-\t";
    }else{
        // 9. Memory Data Register (MDR) – one cycle behind
        if (opcode=="sll"||opcode=="srl")
            outFile<<"-"<<"\t";
        else
            outFile<< hex << prevMDR << dec << "\t";

        // 10. Read register 1 (current cycle)
        if (opcode=="sll"||opcode=="srl")
            outFile<<rt<<"\t";
        else
            outFile << rs << "\t";

        // 11. Register Read Data 2 (B)
        if (opcode=="sll"||opcode=="srl")
            outFile<<"-"<<"\t";
        else
            outFile<< rt << "\t";

        // 12. Write Register Destination (current cycle)
        if (opcode == "sw"  || opcode == "beq"  || opcode == "bne" ||
            opcode == "j"   || opcode == "jr"   || opcode == "jal")
        {
            outFile << "-\t";
        }
        else if (opcode == "addi" || opcode == "addiu" || opcode == "andi" ||
                opcode == "ori"  || opcode == "slti"  || opcode == "sltiu" ||
                opcode == "lui"  || opcode == "lw")
        {
            outFile << rt << "\t";
        }
        else {
            // R-type destination is rd
            outFile << rd << "\t";
        }

        // 13. Write Data (Only for instructions that write to a register)
        if (state == 1) {
            outFile << "-\t";
       } else if (opcode == "sw"  || opcode == "beq"  || opcode == "bne" ||
            opcode == "j"   || opcode == "jr")
        {
            outFile << "-\t";
        }
        else if (opcode == "lw") {
            // Value loaded from memory
            outFile << hex << registers.getMDR() << "\t";
        }
        else if (opcode=="sll"||opcode=="srl")
            outFile<<"-"<<"\t";
        else{
            // The actual value being written
            outFile << hex << registers.getB() << "\t";
        }

        // 14. Read Data 1 (A) (current cycle)
        if (state == 1)
            outFile << "-\t";
        else
            outFile << hex << registers.getA() << dec << "\t";

        // 15. Read Data 2 (B) (current cycle)
        if (state == 1)
            outFile << "-\t";
        else if (opcode=="sll"||opcode=="srl")
            outFile<<"-"<<"\t";
        else
            outFile << hex << registers.getB() << dec << "\t";

        // 16. Register Read Data 1 (A) – one cycle behind
        if (state == 1)
            outFile << "-\t";
        else
            outFile << hex << prevA << dec << "\t";

        // 17. Register Read Data 2 (B) – one cycle behind
        if (state == 1)
            outFile << "-\t";
        else if (opcode=="sll"||opcode=="srl")
            outFile<<"-"<<"\t";
        else
            outFile  << hex << prevB << dec << "\t";

        // 18. ALU operand 1 – one cycle behind
        if (state == 1)
            outFile << "-\t";
        else
            outFile << hex << prevOp1 << dec << "\t";

        // 19. ALU operand 2 – one cycle behind
        if (state == 1)
            outFile << "-\t";
        else
            outFile << hex << prevOp2 << dec << "\t";

        // 20. ALU result – one cycle behind
        if (state == 1)
            outFile << "-\t";
        else
            outFile<< hex << prevALURes << dec << "\t";

        // 21. ALUOut Register – one cycle behind
        if (state == 1)
            outFile << "-\t";
        else
            outFile << hex << prevALUOut << "\t";
    }

    // Control Signals (current cycle)
    outFile << (CU.getPCWriteCond() ? "1" : "0") << "\t";
    outFile << (CU.getPCWrite() ? "1" : "0") << "\t";
    outFile << (CU.getIorD() ? "1" : "0") << "\t";
    outFile << (CU.getMemRead() ? "1" : "0") << "\t";
    outFile  << (CU.getMemWrite() ? "1" : "0") << "\t";
    if (state==1)
        outFile << "-\t";
    else
        outFile << (CU.getMemToReg() ? "1" : "0") << "\t";
    outFile << (CU.getIRWrite() ? "1" : "0") << "\t";

    // ALUOpValue in binary (2 bits)
    int ALUOpValue = (CU.getALUOp1() << 1) | CU.getALUOp2();
    outFile << std::bitset<2>(ALUOpValue) << "\t";

    if(state==1)
        outFile<<"10\t";
    else
        // PCSource (multi-bit)
        outFile << std::bitset<2>(CU.getPCSource()) << "\t";

    // ALUSrcB (multi-bit)
    outFile << std::bitset<2>(CU.getALUSrcB()) << "\t";


    outFile << CU.getALUSrcA() << "\t";
    outFile<< (CU.getRegWrite() ? "1" : "0") << "\t";

    if (state==0||state==1||state==2||state==3||state==6||state== 10)
        outFile<<"-\n";
    else
        outFile << (CU.getRegDst() ? "1" : "0") << "\n";
    outFile << "\nMemory State:" << endl;
    const auto& memMap = memory.getDataMemory();

    for (auto it = memMap.begin(); it != memMap.end(); ++it) {
        int32_t addr = it->first;
        int32_t memValue = it->second;

        // Only print nonzero entries
        if (memValue != 0) {
            // Print to file
            outFile<<hex<<memValue << "\t";
        }
    }

    prevPC= PC;
    prevMDR= registers.getMDR();
    prevA= registers.getA();
    prevB= registers.getB();
    prevOp1= operand1;
    prevOp2= operand2;
    prevALURes= aluResult;
    prevALUOut= registers.getALUOut();

}


void Observer::finalCycle(ostream& outFile, int cycleCount, int PC, const string& current, const string& opcode,
                           int aluResult, Registers& registers, Memory& memory,long long duration) {
    if (!outFile) {
        cerr << "Error opening output file!" << endl;
        return;
    }
    outFile << "\n\n-----Final State-----\n";
    outFile << "Registers:\n";
    outFile << hex << PC << dec << "\t";
    registers.printRegisters(outFile);

    outFile << "\n\nMemory State:" << endl;
    const auto& memMap = memory.getDataMemory();

    for (auto it = memMap.begin(); it != memMap.end(); ++it) {
        int32_t addr = it->first;
        int32_t memValue = it->second;

        // Only print nonzero entries
        if (memValue != 0) {
            cout << hex << memValue<<dec << endl;
            // Print to file
            outFile<<hex<<memValue << "\t";
        }
    }

    outFile<<"\n";
    outFile << "\nTotal Cycles:\n" <<dec<<cycleCount;
    outFile<<"\n";
    outFile << "\nUser Time\n" <<duration;
}
