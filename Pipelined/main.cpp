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
#include "PipeRegisters.h"
#include "DataHazardDetector.h"
#include "ForwardingUnit.h"


using namespace std;
using namespace std::chrono;

int32_t signExtend(int16_t imm) {
    return static_cast<int32_t>(imm);
}

uint32_t zeroExtend(uint16_t imm) {
    return static_cast<uint32_t>(imm);
}

bool stall=false;
bool branchTaken=false;
bool branchFlush=false;
bool branchTakenPending = false;
int branchTarget = 0;
int delaySlot = -1;
int delaySlotPC;
bool delaySlotFetched = false;
int branchStallCounter=0;


DataHazardDetector hazardDetector;
ForwardingUnit forwardingUnit;

// ------------------ IF Stage ------------------ //
void IF_stage(InstructionMemory &instrMem, int &PC, PipeRegisters::IF_ID &if_id_next, const PipeRegisters::IF_ID &if_id) {

    // If pipeline is stalled, hold IF/ID register and don't fetch new instruction
    if (stall) {
        if_id_next = if_id;
        return;
    }

    // --- Handle delay slot fetch ---
    if (branchTakenPending && !delaySlotFetched) {
    // Fetch the delay slot instruction and set it to the IF stage
    if_id_next.PC = delaySlotPC;  // Delay slot PC address (the address right after the branch)
    if_id_next.instruction = instrMem.getInstruction(delaySlotPC);  // Get the instruction at delay slot
    delaySlotFetched = true;  // Mark that we fetched the delay slot instruction
    PC = delaySlotPC + 4;  // Advance PC so the next fetch gets the branch target
    return;  // Do not fetch the usual instruction, just return after handling the delay slot
}


    // --- After delay slot, fetch from branch target ---
    if (branchTakenPending && delaySlotFetched) {
        // Now update the PC to the branch target after executing the delay slot
        PC = branchTarget;  // Update to branch target
        branchTakenPending = false;  // Reset the branch taken flag
        delaySlotFetched = false;  // Reset the delay slot fetched flag
    }

    // --- Normal instruction fetch ---
    if_id_next.PC = PC;  // Update the next IF/ID pipeline register with the current PC
    string instr = instrMem.getInstruction(PC);  // Fetch instruction from memory

    if (instr.empty() || instr == "nop") {
        // If no valid instruction (or "nop") at the PC, move to the next instruction
        int nextPC = PC ;
        while (nextPC < instrMem.getInstructionCount() * 4) {
            string nextInstr = instrMem.getInstruction(nextPC);
            if (!nextInstr.empty() && nextInstr != "nop") {
                if_id_next.instruction = nextInstr;
                if_id_next.PC = nextPC;
                PC = nextPC+4;  // Move to the next instruction
                return;
            }
            nextPC += 4;  // Move forward to the next possible address
        }
        if_id_next.instruction = "nop";  // If still no instruction found, set as nop
    } else {
        // Otherwise, proceed as normal
        if_id_next.instruction = instr;
        PC += 4;  // Move PC to the next instruction
    }
}


// ------------------ ID Stage Function ------------------ //
void ID_stage(const PipeRegisters::IF_ID &if_id,
              PipeRegisters::ID_EX &id_ex_next,
              Registers &registers,
              ControlUnit &CU,
              InstructionMemory &instrMem,
              PipeRegisters &pipeRegs,
              int &PC,
              ForwardingUnit &forwardingUnit) {

    if (stall) return;

    if (if_id.instruction.empty() || if_id.instruction == "nop") {
        id_ex_next.instruction = "nop";
        id_ex_next.PC = if_id.PC;
        CU.decodeInstruction("nop");
        id_ex_next.controlSignals = CU.getSignals();
        return;
    }

    id_ex_next.instruction = if_id.instruction;
    string opcode, rs, rt, rd;
    instrMem.parseInstruction(if_id.instruction, opcode, rs, rt, rd);
    id_ex_next.opcode = opcode;
    id_ex_next.rs = rs;
    id_ex_next.rt = rt;
    id_ex_next.rd = rd;
    id_ex_next.PC = if_id.PC;

    CU.decodeInstruction(opcode);
    id_ex_next.controlSignals = CU.getSignals();

    // For shift instructions (sll, srl), just read from register file.
    if (opcode == "sll" || opcode == "srl") {
        id_ex_next.rt = rt;
        id_ex_next.rd = rd;
        id_ex_next.rs = rs;  // Holds shift amount for consistency/logging

        int shiftAmount = 0;
        try {
            shiftAmount = stoi(rs);
        } catch (...) {
            shiftAmount = 0;
        }
        // Simply read the register value for rt.
        id_ex_next.readData1 = registers.getRegister(rt);
        id_ex_next.readData2 = 0;
        id_ex_next.signExtendedImm = shiftAmount;

        // Set hazard monitor flag
        pipeRegs.HDU_activated = hazardDetector.detectHazard(pipeRegs, instrMem);
        return;
    }

    // For all other instructions, simply read registers.
    int readData1 = registers.getRegister(rs);
    int readData2 = registers.getRegister(rt);
    id_ex_next.readData1 = readData1;
    id_ex_next.readData2 = readData2;

    // ---------- Branch Instructions without Forwarding in ID ----------
    if (opcode == "beq" || opcode == "bne") {
        // Use register file values directly
        int branchOp1 = readData1;
        int branchOp2 = readData2;

        int targetPC = instrMem.getLabelAddress(rd);
        id_ex_next.branchTarget = targetPC;
        branchTaken = (opcode == "beq") ? (branchOp1 == branchOp2) : (branchOp1 != branchOp2);
        if (branchTaken) {
            std::cout << "Branch Taken in ID stage! Branch target = 0x" << std::hex << id_ex_next.branchTarget << "\n";
            branchTarget = id_ex_next.branchTarget;
            delaySlotPC = PC;
            branchTakenPending = true;
        }
    }

    else if (opcode == "addi" || opcode == "addiu") {
        id_ex_next.readData1 = readData1;
        id_ex_next.readData2 = 0;
        try {
            id_ex_next.signExtendedImm = signExtend(stoi(rd));
        } catch (...) {
            id_ex_next.signExtendedImm = 0;
        }
    }
    else if (opcode == "ori" || opcode == "andi") {
        try {
            int imm = (rd.substr(0, 2) == "0x" || rd.substr(0, 2) == "0X")
                        ? stoi(rd, nullptr, 16)
                        : stoi(rd);
            id_ex_next.signExtendedImm = imm & 0xFFFF;
        } catch (...) {
            id_ex_next.signExtendedImm = 0;
        }
        id_ex_next.readData1 = readData1;
        id_ex_next.readData2 = 0;
    }
    else if (opcode == "lw") {
        id_ex_next.readData1 = readData1;
        id_ex_next.readData2 = 0;
        try {
            id_ex_next.signExtendedImm = signExtend(stoi(rd));
        } catch (...) {
            id_ex_next.signExtendedImm = 0;
        }
    }
        else if (opcode == "sw") {
            id_ex_next.readData1 = readData1;
            id_ex_next.readData2 = readData2;
        try {
            id_ex_next.signExtendedImm = signExtend(stoi(rd));
        } catch (...) {
            id_ex_next.signExtendedImm = 0;
        }
    }
    else {
        // R-type or unknown instruction format
        id_ex_next.readData1 = readData1;
        id_ex_next.readData2 = readData2;
        id_ex_next.signExtendedImm = 0;
    }
    // Set hazard monitor flag in ID stage
    pipeRegs.HDU_activated = hazardDetector.detectHazard(pipeRegs, instrMem);
}

void EX_stage(const PipeRegisters::ID_EX &id_ex,
              PipeRegisters::EX_MEM &ex_mem_next,
              ALU &alu,
              int &PC,
              PipeRegisters &pipeRegs,
              InstructionMemory &instrMem,
              ForwardingUnit &forwardingUnit) {

    // If branch was taken, mark for flush
    if (branchTaken) {
        branchFlush = true;
        branchTaken = false;
    }

    // Propagate common fields
    ex_mem_next.instruction    = id_ex.instruction;
    ex_mem_next.controlSignals = id_ex.controlSignals;
    ex_mem_next.opcode         = id_ex.opcode;

    if (id_ex.opcode == "nop") {
        ex_mem_next.aluResult = 0;
        ex_mem_next.dest = "nop";
        return;
    }

    // Extract operands from ID/EX (values read in ID stage)
    int operand1 = id_ex.readData1;
    int operand2 = id_ex.readData2;

    // ---------- Apply Forwarding in EX Stage ----------
    pipeRegs.FU_activated = forwardingUnit.detectForward(pipeRegs);

    // Use the forwarding unit to determine if the operands should be replaced.
    ForwardingSignals signals = forwardingUnit.getForwardingSignals(pipeRegs, id_ex.rs, id_ex.rt);
    if (signals.forwardA == 2) {
        operand1 = pipeRegs.ex_mem.aluResult;
        std::cout << "FORWARDING: EX/MEM -> EX for " << id_ex.rs
                  << " (operand1 = " << operand1 << ")\n";
    } else if (signals.forwardA == 1) {
        operand1 = (pipeRegs.mem_wb.controlSignals.MemToReg ?
                    pipeRegs.mem_wb.memoryData : pipeRegs.mem_wb.aluResult);
        std::cout << "FORWARDING: MEM/WB -> EX for " << id_ex.rs
                  << " (operand1 = " << operand1 << ")\n";
    }
    if (signals.forwardB == 2) {
        operand2 = pipeRegs.ex_mem.aluResult;
        std::cout << "FORWARDING: EX/MEM -> EX for " << id_ex.rt
                  << " (operand2 = " << operand2 << ")\n";
    } else if (signals.forwardB == 1) {
        operand2 = (pipeRegs.mem_wb.controlSignals.MemToReg ?
                    pipeRegs.mem_wb.memoryData : pipeRegs.mem_wb.aluResult);
        std::cout << "FORWARDING: MEM/WB -> EX for " << id_ex.rt
                  << " (operand2 = " << operand2 << ")\n";
    }

    // ---------- sw ----------
    if (id_ex.opcode == "sw") {
        ex_mem_next.operand1 = operand1;
        ex_mem_next.operand2 = id_ex.signExtendedImm; // Immediate used for address calc.
        ex_mem_next.aluResult = operand1 + id_ex.signExtendedImm;
        ex_mem_next.writeData = operand2;  // rt is the value to store.
        ex_mem_next.dest = "nop";
        return;
    }
    // ---------- lw ----------
    if (id_ex.opcode == "lw") {
        ex_mem_next.operand1 = operand1;
        ex_mem_next.operand2 = id_ex.signExtendedImm; // Use immediate for address calc.
        ex_mem_next.aluResult = operand1 + id_ex.signExtendedImm;
        ex_mem_next.dest = id_ex.rt; // Destination is rt.
        return;
    }

    // ---------- Immediate instructions ----------
    if (id_ex.controlSignals.ALUSrc) {
        operand2 = id_ex.signExtendedImm;
    }

    // ---------- Shift instructions ----------
    if (id_ex.opcode == "sll") {
        ex_mem_next.operand1 = operand2;              // For shift, operand is in rt.
        ex_mem_next.operand2 = id_ex.signExtendedImm;   // Shift amount.
        ex_mem_next.aluResult = operand2 << (id_ex.signExtendedImm & 0x1F);
        ex_mem_next.dest = id_ex.rd;
        return;
    }
    if (id_ex.opcode == "srl") {
        ex_mem_next.operand1 = operand2;
        ex_mem_next.operand2 = id_ex.signExtendedImm;
        ex_mem_next.aluResult = static_cast<unsigned int>(operand2) >> (id_ex.signExtendedImm & 0x1F);
        ex_mem_next.dest = id_ex.rd;
        return;
    }

    // ---------- Logical immediates ----------
    if (id_ex.opcode == "ori") {
        ex_mem_next.operand1 = operand1;
        ex_mem_next.operand2 = id_ex.signExtendedImm;
        ex_mem_next.aluResult = operand1 | id_ex.signExtendedImm;
        ex_mem_next.dest = id_ex.rt;
        return;
    }
    if (id_ex.opcode == "andi") {
        ex_mem_next.operand1 = operand1;
        ex_mem_next.operand2 = id_ex.signExtendedImm;
        ex_mem_next.aluResult = operand1 & id_ex.signExtendedImm;
        ex_mem_next.dest = id_ex.rt;
        return;
    }

    // ---------- Default ALU operation ----------
    // For any remaining instructions, if ALUSrc is true then operand2 is already set.
    ex_mem_next.operand1 = operand1;
    ex_mem_next.operand2 = operand2;
    ex_mem_next.aluResult = alu.compute(id_ex.opcode,
                                        id_ex.controlSignals.ALUOp1,
                                        id_ex.controlSignals.ALUOp2,
                                        operand1, operand2);
    ex_mem_next.dest = id_ex.controlSignals.RegDst ? id_ex.rd : id_ex.rt;
}

// ------------------ MEM Stage Function ------------------ //
void MEM_stage(const PipeRegisters::EX_MEM &ex_mem,
               PipeRegisters::MEM_WB &mem_wb_next,
               Memory &memory,
               Registers &registers)
{
    mem_wb_next.instruction = ex_mem.instruction;

    // Memory read
    if (ex_mem.controlSignals.MemRead) {
        mem_wb_next.memoryData = memory.readMemory(ex_mem.aluResult);
        if (ex_mem.controlSignals.RegWrite && !ex_mem.dest.empty() && ex_mem.dest != "$zero") {
            registers.setRegister(ex_mem.dest, mem_wb_next.memoryData);
        }
    }

    // Memory write
    if (ex_mem.controlSignals.MemWrite) {
        memory.writeMemory(ex_mem.aluResult, ex_mem.writeData);
    }

    // For shift instructions, pass the ALU result through
    if (ex_mem.opcode == "sll" || ex_mem.opcode == "srl") {
        mem_wb_next.aluResult = ex_mem.aluResult;
        mem_wb_next.dest = ex_mem.dest;
    }

    // Pass-through of remaining signals
    mem_wb_next.opcode         = ex_mem.opcode;
    mem_wb_next.aluResult      = ex_mem.aluResult;
    mem_wb_next.dest           = ex_mem.dest;
    mem_wb_next.controlSignals = ex_mem.controlSignals;
}

// ------------------ WB Stage Function ------------------ //
void WB_stage(const PipeRegisters::MEM_WB &mem_wb, Registers &registers) {
    if (mem_wb.dest.empty() || mem_wb.dest == "$zero") return;

    int writeValue = (mem_wb.controlSignals.MemToReg)
                     ? mem_wb.memoryData
                     : mem_wb.aluResult;

    if (mem_wb.controlSignals.RegWrite) {
        registers.setRegister(mem_wb.dest, writeValue);
    }

    // For shift instructions, perform a redundant write-back for consistency
    if ((mem_wb.opcode == "sll" || mem_wb.opcode == "srl")) {
        registers.setRegister(mem_wb.dest, mem_wb.aluResult);
    }
}

// ------------------ MAIN ------------------ //
int main() {
    ofstream outFile("output.txt", ios::trunc);
    if (!outFile) {
        cerr << "Error opening output file!" << endl;
        return 1;
    }
    outFile << "MIPS32 Pipelined\n";

    // Create components
    InstructionMemory instrMem;
    ALU alu;
    Registers registers;
    ControlUnit CU;
    Observer observer;
    Memory memory;
    ForwardingUnit forwardingUnit;

    int32_t PC = 0;
    int cycleCount = 0;

    // Pipeline registers
    PipeRegisters pipeRegs;

    // Load instructions from file
    if (!instrMem.loadInstructions("input.s")) {
        return 1;
    }

    // Start time measurement
    auto start = high_resolution_clock::now();


    // Main pipeline simulation loop

    // Main pipeline simulation loop
    while (PC < instrMem.getInstructionCount() * 4) {
        WB_stage(pipeRegs.mem_wb, registers);
        MEM_stage(pipeRegs.ex_mem, pipeRegs.mem_wb_next, memory, registers);
        EX_stage(pipeRegs.id_ex, pipeRegs.ex_mem_next, alu, PC, pipeRegs, instrMem, forwardingUnit);

        // ----------- Parse instruction from IF/ID for hazard detection -----------
        std::string id_op, id_rs, id_rt, id_rd;
        instrMem.parseInstruction(pipeRegs.if_id.instruction, id_op, id_rs, id_rt, id_rd);
        pipeRegs.if_id.rs = id_rs;
        pipeRegs.if_id.rt = id_rt;

        bool hazardExists = false;

        // Check explicitly for branch or lw-use hazard
        if (id_op == "beq" || id_op == "bne" || id_op == "lw") {
            hazardExists = hazardDetector.detectHazard(pipeRegs, instrMem);
        }

        // Explicit load-use hazard detection (additional safety)
        std::string ex_op, ex_rs, ex_rt, ex_rd;
        instrMem.parseInstruction(pipeRegs.id_ex.instruction, ex_op, ex_rs, ex_rt, ex_rd);

        if (ex_op == "lw") {
            std::string lw_dest = ex_rt;
            if ((!id_rs.empty() && id_rs == lw_dest) || (!id_rt.empty() && id_rt == lw_dest)) {
                hazardExists = true;
                std::cout << "[Hazard Detected] load-use hazard on " << lw_dest << "\n";
            }
        }

        // Apply stall if hazard detected
        if (hazardExists) {
            std::cout << "Hazard detected: Stalling pipeline\n";
            stall = true;
            pipeRegs.id_ex_next = PipeRegisters::ID_EX();
            pipeRegs.id_ex_next.instruction = "nop";
            pipeRegs.if_id_next = pipeRegs.if_id;
        } else {
            stall = false;
            // Execute ID stage normally when no hazard detected
            ID_stage(pipeRegs.if_id, pipeRegs.id_ex_next, registers, CU, instrMem, pipeRegs, PC, forwardingUnit);
            IF_stage(instrMem, PC, pipeRegs.if_id_next, pipeRegs.if_id);
        }

        if (branchFlush) {
            cout << "Flushing instruction in ID stage (delay slot)\n";
            pipeRegs.flushPipeline();
            branchFlush = false;
        }

        // Update pipeline registers for the next cycle.
        pipeRegs.update();
        pipeRegs.clearNext();

        cycleCount++;
        cout << "Cycle " << dec << cycleCount << ":\n";
        pipeRegs.printState();

        string current = pipeRegs.if_id.instruction;
        string opcode = pipeRegs.id_ex.opcode;
        string rs = pipeRegs.id_ex.rs;
        string rt = pipeRegs.id_ex.rt;
        string rd = pipeRegs.id_ex.rd;
        int value1 = pipeRegs.id_ex.readData1;
        int value2 = pipeRegs.id_ex.readData2;
        int immValue = pipeRegs.id_ex.signExtendedImm;
        int operand1 = value1;
        int operand2 = pipeRegs.id_ex.controlSignals.ALUSrc ? immValue : value2;
        int memoryValue = pipeRegs.mem_wb.memoryData;
        string dest = pipeRegs.ex_mem.dest;
        int nextPC = PC + 4;
        string branchLabel = (opcode == "beq" || opcode == "bne") ? pipeRegs.id_ex.branchLabel : "-";
        int aluResult = pipeRegs.ex_mem.aluResult;

        observer.observeCycle(outFile, cycleCount, nextPC, PC,
                              pipeRegs.if_id.instruction,
                              pipeRegs.id_ex.instruction,
                              pipeRegs.ex_mem.instruction,
                              pipeRegs.mem_wb.instruction,
                              pipeRegs.prev_mem_wb_instruction,
                              current, opcode, rs, rt, rd, value1, value2,
                              immValue, operand1, operand2, aluResult,
                              memoryValue, dest, CU, branchLabel, registers,
                              memory, instrMem,pipeRegs);


    }
    // Drain pipeline for 4 extra cycles
    int extraCycles = 4;
    for (int i = 0; i < extraCycles; i++) {
        WB_stage(pipeRegs.mem_wb, registers);
        MEM_stage(pipeRegs.ex_mem, pipeRegs.mem_wb_next, memory, registers);
        EX_stage(pipeRegs.id_ex, pipeRegs.ex_mem_next, alu, PC, pipeRegs, instrMem, forwardingUnit);
        ID_stage(pipeRegs.if_id, pipeRegs.id_ex_next, registers, CU, instrMem, pipeRegs, PC, forwardingUnit);

        // Insert NOP for IF stage
        pipeRegs.if_id_next.instruction = "nop";
        pipeRegs.if_id_next.PC = PC;

        pipeRegs.update();
        pipeRegs.clearNext();

        cycleCount++;
        cout << "Cycle " << cycleCount << " (draining):\n";

        string current = pipeRegs.if_id.instruction;
        string opcode = pipeRegs.id_ex.opcode;
        string rs = pipeRegs.id_ex.rs;
        string rt = pipeRegs.id_ex.rt;
        string rd = pipeRegs.id_ex.rd;
        int value1 = pipeRegs.id_ex.readData1;
        int value2 = pipeRegs.id_ex.readData2;
        int immValue = pipeRegs.id_ex.signExtendedImm;
        int operand1 = value1;
        int operand2 = CU.getALUSrc() ? immValue : value2;
        int aluResult = pipeRegs.ex_mem.aluResult;
        int memoryValue = pipeRegs.mem_wb.memoryData;
        string dest = pipeRegs.ex_mem.dest;
        int nextPC = PC + 4;
        string branchLabel = (opcode == "beq" || opcode == "bne") ? pipeRegs.id_ex.branchLabel : "-";
        aluResult = pipeRegs.ex_mem.aluResult;

        pipeRegs.printState();

        observer.observeCycle(outFile, cycleCount, nextPC, PC,
                              pipeRegs.if_id.instruction,
                              pipeRegs.id_ex.instruction,
                              pipeRegs.ex_mem.instruction,
                              pipeRegs.mem_wb.instruction,
                              pipeRegs.prev_mem_wb_instruction,
                              current, opcode, rs, rt, rd, value1, value2,
                              immValue, operand1, operand2, aluResult,
                              memoryValue, dest, CU, branchLabel, registers,
                              memory, instrMem,pipeRegs);



    }

    auto end_time = high_resolution_clock::now();
    long long duration = duration_cast<nanoseconds>(end_time - start).count();

    observer.finalCycle(outFile, cycleCount, PC, registers, memory, duration);
    outFile.close();
    return 0;
}
