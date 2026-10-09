#ifndef MIDICC_CPP
#define MIDICC_CPP

#include "master_controller/midicc.hpp"

const ModuleCommandDeclaration MIDICC::moduleCommands[] = {
    { "control", "cl", "number", "Get/Set the controller number", false, true, &s_control, eCommandType_data::ectSimpleUInt8 | eCommandType_function::ectName },
    { "command", "cm", "outputassignment", "Get/Set the commands to execute when the given controller is sent", false, true, &s_command,
        eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment | eCommandType_dataOptions::ectExpression, "channel,controller,value" }
};

getModuleCount(MIDICC)

MIDICC::MIDICC(uint8_t control, String *commands) {
    ccnum = control;
    outputS = *commands;
};

CREATE_MODULE_COMMAND_FUNCTION(control, MIDICC) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        ccnum = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(ccnum), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(command, MIDICC) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        outputS = stripQuotes(inCommandItem->argument[0]);
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(outputS, true), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

#endif // MIDICC_CPP
