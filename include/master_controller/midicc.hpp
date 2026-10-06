#ifndef MIDICC_HPP
#define MIDICC_HPP

#include "base/module.hpp"

class MIDICC : public Module {
public:
    SETMODULEID("continuouscontroller", "cc", "MIDI Continuous Controller data", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(control, MIDICC)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(command, MIDICC)

    MIDICC(uint8_t, String*);

    uint8_t ccnum;
    String outputS;
};

#endif // BOWACTUATOR_HPP

