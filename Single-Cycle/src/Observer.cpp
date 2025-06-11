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

void Observer::observeCycle(ostream& outFile, int cycleCount, int PC,int PCold, const string& current, const string& opcode, const string& operand1,
                            const string& operand2, const string& operand3, int value1, int value2, int aluResult, int memoryValue,
                            const string& dest, ControlUnit& CU, Registers& registers, Memory& memory) {

    outFile << "\n-----Cycle "<<dec<<cycleCount<< "-----\n";

    //print registers
    outFile<<"Registers:\n";

    outFile<<hex<<nouppercase<<PC<<dec<<"\t";

    registers.printRegisters(outFile);
    if (!(opcode=="beq"|| opcode=="bne"||opcode=="j"||opcode=="jal"||opcode=="jr"||opcode=="jalr"))
        PCold=PC-4;

    outFile<<"\nMonitors:\n"<<hex<<nouppercase<<(PCold)<<"\t"<<current<<"\t";

    if (opcode=="lw"||opcode=="sw")
        outFile<<operand3<<"\t"<<operand2<<"\t-";
    else{
        //Read Register 1
        if (operand2.size()>1 && operand2[0]=='$') {
            outFile <<hex<< operand2<<dec;
        }else{
            outFile << "-";
        }
        outFile << "\t";




        //Read Register 2
        if (operand3.size()>1&&operand3[0]=='$') {
            outFile <<hex<<operand3<<dec;
        }else{
            outFile << "-";
        }
    }
    outFile<<"\t";
     //Write Register
        if (operand1.size()>1 && operand1[0]=='$') {
            outFile<<hex<<operand1<<dec;
        }else
            outFile<<"-";

        outFile << "\t";

    //Write Data
    if(opcode=="sw"||opcode=="beq"||opcode=="bne")
        outFile<<"-\t";
    else if (opcode=="lw"){
        outFile<<hex<<memoryValue<<dec<<"\t";
    }else {
        outFile<<hex<<aluResult<<dec<<"\t";
    }

    //Read Data 1
    outFile<<hex<<value1<<dec<<"\t";
    //Read Data 2
    if(opcode == "addi" || opcode == "addiu" || opcode=="andi"||opcode=="ori"||opcode == "slti"|| opcode == "sltiu" ){
        outFile<<"-"<<"\t";
    }else
        outFile<<hex<<value2<<dec<<"\t";

    outFile<<hex<<aluResult<<dec<<"\t";

    //ALU Result for branch
    if (CU.getBranch())
        outFile << hex<<operand1<<dec<<"\t";  // Print operand1 if it's a branch instruction
    else
        outFile << "-\t";

    //Memory Address
    if (CU.getMemRead() || CU.getMemWrite()) {
        outFile <<hex<<aluResult;
    } else {
        outFile << "-";
    }
    outFile << "\t";

    //Memory Write Data
    if (CU.getMemWrite()) {
        outFile <<hex<<value2;
    }else {
        outFile << "-";
    }
    outFile << "\t";

    //Memory Read Data
    if (CU.getMemRead()) {
        outFile << to_string(registers.getRegister(dest));
    } else {
        outFile << "-";
    }
    outFile << "\t";

    //Control Unit Signals
    // RegDst (Determines destination register, only relevant for R-type)
    if (CU.getRegDst()==true) {
        outFile << "1";
    }else if (opcode=="sw"||opcode=="beq"||opcode=="bne"){
        outFile<<"-";
    }else{
        outFile<<"0";
    }
    outFile<<"\t";

    // Jump
    if (opcode == "j" || opcode == "jal") {
        outFile << "1\t";
    }else {
        outFile << "0\t";
    }

    //Branch (Relevant for branch instructions)
    if (opcode == "beq" || opcode == "bne") {
        outFile << "1";
    } else {
        outFile << "0";
    }
    outFile << "\t";

    // MemRead (Relevant for lw)
    if (opcode == "lw") {
        outFile << "1";
    }else {
        outFile << "0";
    }
    outFile << "\t";

    //MemToReg (Relevant for lw, determines if data comes from memory)
    if (opcode=="lw"){
        outFile<<"1";
    }else if (opcode=="sw"||opcode=="beq"||opcode=="bne"){
        outFile<<"-";
    } else {
        outFile<<"0";
    }
    outFile<<"\t";

    // ALUOp
if (opcode == "lw" || opcode == "sw") {
    outFile << "00\t"; // Add address calculation
}
else if (opcode == "beq") {
    outFile << "01\t"; // Subtract for comparison
}
else if (opcode == "add" || opcode == "sub" || opcode == "and" ||
         opcode == "or" || opcode == "slt") {
    outFile << "10\t"; // R-type: use funct field
}
else if (opcode == "j") {
    outFile << "--\t"; // Jump: ALU not used
}
else {
            outFile << bitset<2>(((CU.getALUOp1() << 1) | CU.getALUOp2())) << "\t";

}

    //MemWrite (Relevant for sw)
    if (opcode == "sw") {
        outFile << "1";
    }else{
        outFile << "0";
    }

    outFile << "\t";

    // ALUSrc(determines if ALU takes immediate or register value)
    if (opcode == "lw" || opcode == "sw" || opcode == "addi" || opcode == "andi" || opcode == "ori" || opcode == "slti" || opcode == "sltiu") {
        outFile << "1"; // Immediate instructions
    }else if (opcode == "j" || opcode == "jal") {
        outFile << "-"; // Don't care for jumps
    }else {
        outFile << "0"; // Register-based operations
    }
    outFile << "\t";

    // RegWrite (Determines if result is written to register)
    if (opcode == "sw" || opcode == "beq" || opcode == "bne" || opcode == "j") {
        outFile << "0"; //no write for these instructions
    } else if (opcode == "jal" || opcode == "lw" || opcode == "addi" || opcode == "andi" || opcode == "ori" || opcode == "slti" || opcode == "sltiu") {
        outFile<<"1"; //write for these instructions
    } else{
        outFile<<"1"; //default to write for R-type instructions
    }
    outFile<<"\n\nMemory State:\n";
    for (int addr=0x10008000; addr <= 0x1000900C; addr += 4) {  // Example address range, adjust as needed
        int memValue=memory.readMemory(addr);
        if (memValue!=0) {
            outFile<<hex<<memValue<<dec<<"\t";
        }
    }
    outFile<<"\n";
}

void Observer::finalCycle(ostream& outFile, int cycleCount, int PC, const string& current, const string& opcode, int aluResult, Registers& registers, Memory& memory) {
    if (!outFile) {
        cerr << "Error opening output file!" << endl;
        return;
    }
    outFile << "\n-----Final State-----\n";
    outFile << "Registers:\n";
    outFile << hex << PC << dec << "\t";
    registers.printRegisters(outFile);
    outFile << "\nMemory State:" << endl;
    for (int addr = 0x10008000; addr <= 0x1000900C; addr += 4) {
        int memValue = memory.readMemory(addr);
        if (memValue != 0) {
            outFile <<uppercase<<hex<< memValue << "\t";
        }
    }
    outFile<<"\n";
    outFile << "\nTotal Cycles:\n" <<dec<<cycleCount;
}
