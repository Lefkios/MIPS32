#ifndef FORWARDING_UNIT_H
#define FORWARDING_UNIT_H

#include "PipeRegisters.h"
#include "DataHazardDetector.h"

class ForwardingUnit {
public:
    ForwardingUnit();

    // Determines forwarding signals based on pipeline state
    ForwardingSignals getForwardingSignals(const PipeRegisters& pipe, const std::string& rs, const std::string& rt);

    // Applies forwarding based on those signals
bool applyForwarding(const PipeRegisters& pipe, int& operandA, int& operandB, const std::string& rs, const std::string& rt);

bool forwardSingle(const PipeRegisters& pipeRegs, int& value, const std::string& reg);

bool detectForward(const PipeRegisters& pipe);


};

#endif // FORWARDING_UNIT_H
