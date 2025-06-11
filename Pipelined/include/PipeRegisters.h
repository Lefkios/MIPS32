#ifndef PIPE_REGISTERS_H
#define PIPE_REGISTERS_H

#include <string>
#include <iostream>
#include "ControlUnit.h"

class PipeRegisters {
public:
    bool HDU_activated = false; // Monitor 27: Hazard Detection Unit
    bool FU_activated = false;  // Monitor 28: Forwarding Unit

    // ------------------ Structures for Each Pipeline Stage ------------------ //
    // 1) IF/ID
    struct IF_ID {
        std::string instruction;  // The fetched instruction string
        int         PC;           // The PC at which this instruction was fetched

        std::string rs;           // Source register 1 (for hazard detection)
        std::string rt;           // Source register 2 (for hazard detection)
        IF_ID();                  // Default constructor
    };

    // 2) ID/EX
    struct ID_EX {
        std::string instruction;
        std::string opcode;
        std::string rs;
        std::string rt;
        std::string rd;
        int PC;

        int readData1;
        int readData2;
        int signExtendedImm;

        std::string branchLabel;
        int branchTarget;

        ControlSignals controlSignals;

        ID_EX(); // Default constructor
    };

    // 3) EX/MEM
    struct EX_MEM {
        std::string instruction;
        std::string opcode;
        int aluResult;
        int writeData;
        std::string dest;
        int operand1;   //first ALU input (from the EX stage)
        int operand2;   //second ALU input (from the EX stage)
        ControlSignals controlSignals;
        EX_MEM();
    };

    // 4) MEM/WB
    struct MEM_WB {
        std::string instruction;
        std::string opcode;
        int aluResult;
        int memoryData;
        std::string dest;
        ControlSignals controlSignals;

        MEM_WB(); // Default constructor
    };

    //Main PipeRegisters Class

    PipeRegisters();

    // Current pipeline registers
    IF_ID   if_id;
    ID_EX   id_ex;
    EX_MEM  ex_mem;
    MEM_WB  mem_wb;

    // Next-state pipeline registers
    IF_ID   if_id_next;
    ID_EX   id_ex_next;
    EX_MEM  ex_mem_next;
    MEM_WB  mem_wb_next;

    // Previous WB stage tracking (for monitor 25 & 26)
    std::string prev_mem_wb_instruction;
    std::string prev_mem_wb_opcode;
    std::string prev_mem_wb_dest;
    int         prev_mem_wb_value;

    // Current WB stage output
    std::string wb_write_register; // Write register name in WB stage
    int wb_write_value;            // Value to write in WB stage


    // Methods
    void update();           // Commit next-state to current
    void clearNext();        // Clear next-state pipeline registers
    void flushPipeline();    // Reset pipeline after branch
    void printState() const; // Debug print
};

#endif // PIPE_REGISTERS_H

