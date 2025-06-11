#include "ForwardingUnit.h"

ForwardingUnit::ForwardingUnit() {
    // Constructor
}

ForwardingSignals ForwardingUnit::getForwardingSignals(const PipeRegisters& pipe, const std::string& rs, const std::string& rt) {
    ForwardingSignals signals = {0, 0};

    const std::string& ex_mem_dest = pipe.ex_mem.dest;
    const std::string& mem_wb_dest = pipe.mem_wb.dest;

    bool ex_mem_write = pipe.ex_mem.controlSignals.RegWrite;
    bool mem_wb_write = pipe.mem_wb.controlSignals.RegWrite;

    // Forwarding for RS (operand A)
    if (!rs.empty() && rs != "$zero") {
        if (ex_mem_write && rs == ex_mem_dest && ex_mem_dest != "nop") {
            signals.forwardA = 2; // Forward from EX/MEM
        } else if (mem_wb_write && rs == mem_wb_dest && mem_wb_dest != "nop") {
            signals.forwardA = 1; // Forward from MEM/WB
        }
    }

    // Forwarding for RT (operand B)
    if (!rt.empty() && rt != "$zero") {
        if (ex_mem_write && rt == ex_mem_dest && ex_mem_dest != "nop") {
            signals.forwardB = 2; // Forward from EX/MEM
        } else if (mem_wb_write && rt == mem_wb_dest && mem_wb_dest != "nop") {
            signals.forwardB = 1; // Forward from MEM/WB
        }
    }

    return signals;
}
bool ForwardingUnit::applyForwarding(const PipeRegisters& pipe, int& operandA, int& operandB, const std::string& rs, const std::string& rt) {
    bool forwarded = false;

    // ----- Forward RS (operand A) -----
    if (!rs.empty() && rs != "$zero") {
        if (pipe.ex_mem.controlSignals.RegWrite && pipe.ex_mem.dest == rs && pipe.ex_mem.dest != "nop") {
            operandA = pipe.ex_mem.aluResult;
            std::cout << "FORWARDING: EX/MEM → ID for " << rs << " (operandA = " << operandA << ")\n";
            forwarded = true;
        }
        else if (pipe.mem_wb.controlSignals.RegWrite && pipe.mem_wb.dest == rs && pipe.mem_wb.dest != "nop") {
            operandA = pipe.mem_wb.controlSignals.MemToReg ? pipe.mem_wb.memoryData : pipe.mem_wb.aluResult;
            std::cout << "FORWARDING: MEM/WB → ID for " << rs << " (operandA = " << operandA << ")\n";
            forwarded = true;
        }
    }

    // ----- Forward RT (operand B) -----
    if (!rt.empty() && rt != "$zero") {
        if (pipe.ex_mem.controlSignals.RegWrite && pipe.ex_mem.dest == rt && pipe.ex_mem.dest != "nop") {
            operandB = pipe.ex_mem.aluResult;
            std::cout << "FORWARDING: EX/MEM → ID for " << rt << " (operandB = " << operandB << ")\n";
            forwarded = true;
        }
        else if (pipe.mem_wb.controlSignals.RegWrite && pipe.mem_wb.dest == rt && pipe.mem_wb.dest != "nop") {
            operandB = pipe.mem_wb.controlSignals.MemToReg ? pipe.mem_wb.memoryData : pipe.mem_wb.aluResult;
            std::cout << "FORWARDING: MEM/WB → ID for " << rt << " (operandB = " << operandB << ")\n";
            forwarded = true;
        }
    }

    return forwarded;
}

bool ForwardingUnit::forwardSingle(const PipeRegisters& pipeRegs, int& value, const std::string& reg) {
    if (reg.empty() || reg == "$zero") return false;

    // From EX/MEM
    if (pipeRegs.ex_mem.controlSignals.RegWrite && pipeRegs.ex_mem.dest == reg) {
        value = pipeRegs.ex_mem.aluResult;
        std::cout << "FORWARDING (SINGLE): EX/MEM → ID for " << reg << " (value = " << value << ")\n";
        return true;
    }

    // From MEM/WB
    if (pipeRegs.mem_wb.controlSignals.RegWrite && pipeRegs.mem_wb.dest == reg) {
        value = pipeRegs.mem_wb.controlSignals.MemToReg
            ? pipeRegs.mem_wb.memoryData
            : pipeRegs.mem_wb.aluResult;
        std::cout << "FORWARDING (SINGLE): MEM/WB → ID for " << reg << " (value = " << value << ")\n";
        return true;
    }

    return false;
}

bool ForwardingUnit::detectForward(const PipeRegisters& pipe) {
    const std::string& rs = pipe.id_ex.rs;
    const std::string& rt = pipe.id_ex.rt;

    const std::string& ex_mem_dest = pipe.ex_mem.dest;
    const std::string& mem_wb_dest = pipe.mem_wb.dest;

    bool ex_mem_write = pipe.ex_mem.controlSignals.RegWrite;
    bool mem_wb_write = pipe.mem_wb.controlSignals.RegWrite;

    bool forwardA = (!rs.empty() && rs != "$zero") &&
                    ((ex_mem_write && rs == ex_mem_dest && ex_mem_dest != "nop") ||
                     (mem_wb_write && rs == mem_wb_dest && mem_wb_dest != "nop"));

    bool forwardB = (!rt.empty() && rt != "$zero") &&
                    ((ex_mem_write && rt == ex_mem_dest && ex_mem_dest != "nop") ||
                     (mem_wb_write && rt == mem_wb_dest && mem_wb_dest != "nop"));

    return forwardA || forwardB;
}

