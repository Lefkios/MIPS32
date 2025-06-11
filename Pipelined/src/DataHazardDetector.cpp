#include "DataHazardDetector.h"
#include "InstructionMemory.h"

DataHazardDetector::DataHazardDetector() {
    // Constructor (initialize if needed)
}

// Returns true if the source register 'src' depends on the destination register 'dest'
// and the write enable signal is active.
bool DataHazardDetector::hasDependency(const std::string& src, const std::string& dest, bool regWrite) {
    if (!regWrite || dest.empty() || dest == "$zero")
        return false;
    return src == dest;
}

bool DataHazardDetector::detectHazard(const PipeRegisters& pipe, InstructionMemory& instrMem) {
    static int stallCounter = 0;

    if (stallCounter > 0) {
        stallCounter--;
        std::cout << "[HazardDetector] Continuing stall (" << stallCounter + 1 << " cycles remaining)\n";
        return true;
    }

    std::string opcode, rs, rt, rd;
    instrMem.parseInstruction(pipe.if_id.instruction, opcode, rs, rt, rd);

    // Case 1: Load-use hazard (LW instruction followed by an instruction that uses its result)
    const std::string& exDest = pipe.id_ex.controlSignals.RegDst ? pipe.id_ex.rd : pipe.id_ex.rt;
    if (pipe.id_ex.controlSignals.MemRead &&
        (exDest == rs || (opcode != "sw" && exDest == rt))) {
        stallCounter = 1;
        std::cout << "[HazardDetector] LOAD-USE hazard detected: DOUBLE STALL\n";
        return true;
    }

    // Case 2: Branch hazard (ONLY stall if real dependency detected)
    if (opcode == "beq" || opcode == "bne") {
        // Check previous instruction (ID/EX)
        const auto& prevSignals = pipe.id_ex.controlSignals;
        const std::string& prevDest = pipe.id_ex.controlSignals.RegDst ? pipe.id_ex.rd : pipe.id_ex.rt;

        if (prevSignals.RegWrite && !prevDest.empty() && prevDest != "$zero" &&
            (prevDest == rs || prevDest == rt)) {
            stallCounter = 1;  // Stall for 2 cycles total (this and next)
            std::cout << "[HazardDetector] BRANCH hazard with previous instruction: DOUBLE STALL\n";
            return true;
        }

        // Check pre-previous instruction (EX/MEM)
        const auto& prePrevSignals = pipe.ex_mem.controlSignals;
        const std::string& prePrevDest = pipe.ex_mem.dest;

        if (prePrevSignals.RegWrite && !prePrevDest.empty() && prePrevDest != "$zero" &&
            (prePrevDest == rs || prePrevDest == rt)) {
            std::cout << "[HazardDetector] BRANCH hazard with pre-previous instruction: SINGLE STALL\n";
            return true;
        }
    }

    // No hazard detected: No stalls required.
    return false;
}
