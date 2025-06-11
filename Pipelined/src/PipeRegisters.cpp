#include "PipeRegisters.h"
#include <iostream>

// ----- IF_ID Struct Constructor -----
PipeRegisters::IF_ID::IF_ID()
    : instruction("nop"), PC(0), rs(""), rt("") {}

// ----- ID_EX Struct Constructor -----
PipeRegisters::ID_EX::ID_EX()
    : instruction("nop"), opcode("nop"), rs(""), rt(""), rd(""), PC(0),
      readData1(0), readData2(0), signExtendedImm(0),
      branchLabel("-"), branchTarget(0), controlSignals() {}

// ----- EX_MEM Struct Constructor -----
PipeRegisters::EX_MEM::EX_MEM()
    : instruction("nop"), opcode("nop"), aluResult(0), writeData(0),
      dest(""), operand1(0), operand2(0), controlSignals() {}

// ----- MEM_WB Struct Constructor -----
PipeRegisters::MEM_WB::MEM_WB()
    : instruction("nop"), opcode("nop"), aluResult(0), memoryData(0),
      dest(""), controlSignals() {}

// ----- PipeRegisters Class Constructor -----
PipeRegisters::PipeRegisters() {
    // Pipeline registers are default constructed
}

// Commits the computed next-state pipeline registers
void PipeRegisters::update() {
    prev_mem_wb_instruction = mem_wb.instruction;
    prev_mem_wb_opcode = mem_wb.opcode;
    prev_mem_wb_dest = mem_wb.dest;
    prev_mem_wb_value = (mem_wb.opcode == "lw") ? mem_wb.memoryData : mem_wb.aluResult;
    wb_write_register = mem_wb.dest;
    wb_write_value = (mem_wb.opcode == "lw") ? mem_wb.memoryData : mem_wb.aluResult;

    if_id = if_id_next;
    id_ex = id_ex_next;
    ex_mem = ex_mem_next;
    mem_wb = mem_wb_next;
}

// Clears next-stage pipeline registers
void PipeRegisters::clearNext() {
    if_id_next = IF_ID();
    id_ex_next = ID_EX();
    ex_mem_next = EX_MEM();
    mem_wb_next = MEM_WB();
}

// Debug print of all pipeline stages
void PipeRegisters::printState() const {
    std::cout << "\nPipeline Stages:\n";
    std::cout << "  IF  : " << if_id.instruction << "\n";
    std::cout << "  ID  : " << id_ex.instruction << "\n";
    std::cout << "  EX  : " << ex_mem.instruction << "\n";
    std::cout << "  MEM : " << mem_wb.instruction << "\n";
    std::cout << "  WB  : " << prev_mem_wb_instruction << "\n\n";
}

void PipeRegisters::flushPipeline() {
    std::cout << "Pipeline Flush: Removing instruction in ID stage (delay slot)\n";
    id_ex_next = ID_EX();               // Clear all fields
    id_ex_next.instruction = "nop";     // Explicitly set NOP
}


