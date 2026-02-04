#ifndef BOWACTUATOR_CPP
#define BOWACTUATOR_CPP

#include "generic_functions/bowactuator.hpp"

const ModuleCommandDeclaration BowActuator::moduleCommands[] = {
    { "", "", "int", "Sets the current actuator", false, false, nullptr, eCommandType::Index },
    { "name", "na", "string", "Get / set the name of the current actuator", false, false, &s_name, eCommandType::Name  },
    { "data", "da", "name:rest:engage:stall", "Set actuator data", false, true, &s_data, eCommandType::Data }
};

getModuleCount(BowActuator)

BowActuator::BowActuator() {};

CREATE_MODULE_COMMAND_FUNCTION(name, BowActuator) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        id = inCommandItem->argument[0];
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + id, debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(data, BowActuator) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 4)) { return eProcessResult::WrongArgumentCount; }
        id = inCommandItem->argument[0];
        restPosition = inCommandItem->argument[1].toInt();
        firstTouchPressure = inCommandItem->argument[2].toInt();
        stallPressure = inCommandItem->argument[3].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + id + ":" + String(restPosition) + ":" + String(firstTouchPressure) + ":" + String(stallPressure), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

#endif // BOWACTUATOR_CPP
