#include "Observer.h"
#include "InstructionMemory.h"
#include "PipeRegisters.h"
#include <fstream>
#include <bitset>
#include <algorithm>


using namespace std;

Observer::Observer()
{
    //ctor
}

Observer::~Observer()
{
    //dtor
}

void Observer::observeCycle(ostream& outFile, int cycleCount, int nextPC, int PC,
    const string& ifInstruction, const string& idInstruction, const string& exInstruction,
    const string& memInstruction, const string& wbInstruction, const string& current, const string& opcode,
    const string& rs, const string& rt, const string& rd, int value1, int value2,
    int immValue, int operand1, int operand2, int aluResult,
    int memoryValue, const string& dest, ControlUnit& CU, string target,
    Registers& registers, Memory& memory, InstructionMemory& instrMem, PipeRegisters& pipe) {

    outFile<<"\n-----Cycle "<<dec<<cycleCount<<"-----\n";

    // --- Registers ---
    outFile << "Registers:\n";
    outFile << hex << nouppercase << PC <<dec<< "\t";
    registers.printRegisters(outFile);

    // --- Monitors ---
  outFile << "\n\nMonitors:\n";

    // FETCH STAGE
    // 1. Next PC
    if (pipe.if_id.instruction == "nop" ||
        pipe.if_id.instruction.empty() ||
        pipe.if_id.instruction == "bubble")
        outFile << "-\t";
    else
        outFile << hex << nextPC << dec << "\t";

    // 2. PC
    if (pipe.if_id.instruction == "nop" ||
        pipe.if_id.instruction.empty() ||
        pipe.if_id.instruction == "bubble")
        outFile << "-\t";
    else
        outFile << hex << pipe.if_id.PC << dec << "\t";

    // 3. IF Instruction
    if (pipe.if_id.instruction == "nop" ||
        pipe.if_id.instruction.empty() ||
        pipe.if_id.instruction == "bubble")
        outFile << "-\t";
    else
        outFile << pipe.if_id.instruction << "\t";

    outFile << "-" << "\t";

        // DECODE STAGE
    // 5–7: rs, rt, write register (all strings)
    if (pipe.id_ex.instruction == "nop" ||
        pipe.id_ex.instruction.empty() ||
        pipe.id_ex.instruction == "bubble") {
        outFile << "-\t"; // Monitor 5: Read Reg 1
        outFile << "-\t"; // Monitor 6: Read Reg 2
        outFile << "-\t"; // Monitor 7: Write Reg
    } else {
        // (Your existing logic here.)
        outFile << pipe.id_ex.rs << "\t";
        // For I-type that use only one register:
        if (pipe.id_ex.opcode == "addi"  || pipe.id_ex.opcode == "addiu" ||
            pipe.id_ex.opcode == "andi"  || pipe.id_ex.opcode == "ori"   ||
            pipe.id_ex.opcode == "slti"  || pipe.id_ex.opcode == "sltiu"  ||
            pipe.id_ex.opcode == "lw")
            outFile << "-\t";
        else
            outFile << pipe.id_ex.rt << "\t";

        if (pipe.id_ex.opcode == "addi" || pipe.id_ex.opcode == "addiu" ||
            pipe.id_ex.opcode == "andi" || pipe.id_ex.opcode == "ori"   ||
            pipe.id_ex.opcode == "slti" || pipe.id_ex.opcode == "sltiu")
            outFile << pipe.id_ex.rt << "\t";
        else if (pipe.id_ex.controlSignals.RegDst)
            outFile << pipe.id_ex.rd << "\t";
        else
            outFile << pipe.id_ex.rt << "\t";
    }


        // 8: Write Data (hex)
    // For a nop instruction, print "-".
    // Otherwise, print the value computed in the write-back stage.
    if (pipe.id_ex.instruction == "nop")
        outFile << "-\t";
    else
        outFile << hex << pipe.wb_write_value << dec << "\t";

        // 9–10: readData1, readData2
    if (pipe.id_ex.instruction == "nop") {
        outFile << "-\t-\t";
    } else {
        outFile << hex << pipe.id_ex.readData1 << dec << "\t";
        // If the instruction is an I-type arithmetic or lw, then read data 2 is not used.
        if (pipe.id_ex.opcode == "addi"  || pipe.id_ex.opcode == "addiu" ||
            pipe.id_ex.opcode == "andi"  || pipe.id_ex.opcode == "ori"   ||
            pipe.id_ex.opcode == "slti"  || pipe.id_ex.opcode == "sltiu"  ||
            pipe.id_ex.opcode == "lw")
            outFile << "-\t";
        else
            outFile << hex << pipe.id_ex.readData2 << dec << "\t";
    }


    // 11: Immediate Value (hex if it exists, "-" otherwise)
    if (pipe.id_ex.instruction == "nop") {
        outFile << "-\t"; // Monitor 11
    } else {
        std::string op = pipe.id_ex.opcode;
        if (op == "addi" || op == "addiu" || op == "ori" || op == "andi" ||op == "lw" || op == "sw" || op == "slti" || op == "sltiu"
            ||op == "beq" || op == "bne" || op == "lui") {
            outFile << hex << pipe.id_ex.signExtendedImm << dec << "\t";
        } else {
            outFile << "-\t";
        }
    }

    // 12–15: rs, rt, rd, dest
    if (pipe.id_ex.instruction == "nop") {
        outFile << "-\t-\t-\t-\t";
    } else {
        outFile << pipe.id_ex.rs << "\t";
        outFile << pipe.id_ex.rt << "\t";

        if (pipe.id_ex.rd.empty())
            outFile << "-\t";
        else
            outFile << pipe.id_ex.rd << "\t";

        if (pipe.id_ex.controlSignals.RegDst)
            outFile << pipe.id_ex.rd << "\t";
        else
            outFile << pipe.id_ex.rt << "\t";
    }

    // EX STAGE
    // 16–18: operand1, operand2, ALU result (hex)
    if (pipe.ex_mem.instruction == "nop" ||
        pipe.ex_mem.instruction.empty() ||
        pipe.ex_mem.instruction == "bubble" ||
        pipe.ex_mem.opcode == "beq" ||
        pipe.ex_mem.opcode == "bne") {
        outFile << "-\t-\t-\t";
    } else {
        outFile << hex << pipe.ex_mem.operand1 << dec << "\t";
        outFile << hex << pipe.ex_mem.operand2 << dec << "\t";
        outFile << hex << pipe.ex_mem.aluResult << dec << "\t";
    }


    // 19. ALUOp (binary)
    if (pipe.id_ex.instruction == "nop") {
        outFile << "-\t";
    } else {
        int ALUOp = (pipe.id_ex.controlSignals.ALUOp1 << 1) | pipe.id_ex.controlSignals.ALUOp2;
        outFile <<ALUOp<< "\t";
    }

    // MEM STAGE

    // 20–24: MEM stage
    if (pipe.ex_mem.instruction == "nop") {
        outFile << "-\t"; // 20: Memory address accessed
        outFile << "-\t"; // 21: Value written to memory
        outFile << "-\t"; // 22: Value read from memory
        outFile << "-\t"; // 23: Value produced
        outFile << "-\t"; // 24: Destination register
    } else {
        // 20: Memory Address Accessed (hex)
        // Only show a valid address for memory instructions (lw or sw).
        if (pipe.ex_mem.opcode == "lw" || pipe.ex_mem.opcode == "sw")
            outFile << hex << pipe.ex_mem.aluResult << dec << "\t";
        else
            outFile << "-\t";

    // 21: Value written to memory (for sw)
    if (pipe.ex_mem.opcode == "sw")
        outFile << hex << pipe.ex_mem.writeData << dec << "\t";
    else
        outFile << "-\t";

    // 22: Value read from memory (for lw)
    if (pipe.ex_mem.opcode == "lw")
        outFile << hex << pipe.mem_wb.memoryData << dec << "\t";
    else
        outFile << "-\t";

    // 23: Value produced
    // For lw, the value produced (to be written back) comes from memory,
    // otherwise it comes from the ALU result.
    if (pipe.mem_wb.opcode == "lw")
        outFile << hex << pipe.mem_wb.memoryData << dec << "\t";
    else
        outFile << hex << pipe.mem_wb.aluResult << dec << "\t";

    // 24: Destination register
    if (pipe.mem_wb.dest == "nop")
        outFile << "-\t";
    else
        outFile << pipe.mem_wb.dest << "\t";
}

    // WB STAGE

    // 25–26: WB Stage (from WB stage)
    if (pipe.mem_wb.instruction == "nop") {
        outFile << "-\t"; // 25: Write register
        outFile << "-\t"; // 26: Value to write
    } else {
        if (pipe.wb_write_register=="nop")
            outFile<<"-\t";
        else
            outFile << pipe.wb_write_register << "\t"; // 25
            outFile << hex << pipe.wb_write_value << dec << "\t"; // 26
    }

    // 27: Hazard Detection Unit sto DECODE STAGE
    if (cycleCount == 0)
        outFile << "1\t";
    else if (pipe.HDU_activated)
        outFile << "1\t";
    else
        outFile << "0\t";

    // 28: Forwarding Unit sto EXECUTE STAGE
    if (pipe.FU_activated)
        outFile << "1";
    else
        outFile << "0";


    // Memory State
    outFile << "\nMemory State:\n";
    const auto& memMap = memory.getDataMemory();
    for (const auto& entry : memMap) {
        if (entry.second != 0) {
            outFile << hex << entry.second << "\t";
        }
    }

    // Pipeline Stages

    outFile << "\n\nPipeline Stages:\n";
    if ((ifInstruction == "nop")|| (ifInstruction == "\t")||(ifInstruction == ""))outFile << "bubble" << "\t";
else outFile <<ifInstruction<< "\t";

    if ((idInstruction == "nop")|| (idInstruction == "\t")||(idInstruction == ""))outFile << "bubble" << "\t";
       else outFile << idInstruction << "\t";
    if ((exInstruction == "nop")|| (exInstruction == "\t")||(exInstruction == ""))outFile << "bubble" << "\t";
       else outFile << exInstruction << "\t";
    if ((memInstruction == "nop")|| (memInstruction == "\t")||(memInstruction == ""))outFile << "bubble" << "\t";
       else outFile << memInstruction << "\t";
    if ((wbInstruction == "nop")|| (wbInstruction == "\t")||(wbInstruction == ""))outFile << "bubble" << "\t";
        else outFile << wbInstruction << "\t";
    outFile << "\n";
}




void Observer::finalCycle(ostream& outFile, int cycleCount,int PC, Registers& registers,Memory& memory, long long duration) {
    if (!outFile) {
        cerr << "Error opening output file!" << endl;
        return;
    }
    outFile << "\n-----Final State-----\n";
    outFile << "Registers:\n";
    outFile << hex << PC-4 << dec << "\t";
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
   outFile << "\nTotal Execution Time:\n" <<duration;
}
