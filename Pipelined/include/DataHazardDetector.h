#ifndef DATA_HAZARD_DETECTOR_H
#define DATA_HAZARD_DETECTOR_H

#include <string>
#include "PipeRegisters.h" // Ensure this file defines your pipeline register structures
#include "InstructionMemory.h"


struct ForwardingSignals {
    int forwardA;
    int forwardB;
};

class DataHazardDetector {
public:
    DataHazardDetector();

    // Computes and returns the forwarding signals based on current pipeline state.
    ForwardingSignals getForwardingSignals(const PipeRegisters& pipe, InstructionMemory& instrMem);

    bool detectHazard(const PipeRegisters& pipe, InstructionMemory& instrMem);
    bool dependsOn(PipeRegisters& pipe, const std::string& reg);

private:
    // Utility: Checks if a source register depends on a destination register that is scheduled for a write.
    bool hasDependency(const std::string& src, const std::string& dest, bool regWrite);




};

#endif // DATA_HAZARD_DETECTOR_H

