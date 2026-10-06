#ifndef BOWACTUATOR_CPP
#define BOWACTUATOR_CPP

#include "generic_functions/bowactuator.hpp"

const ModuleCommandDeclaration BowActuator::moduleCommands[] = {
    { "", "", "int", "Sets the current actuator", false, false, nullptr, eCommandType_function::ectIndex },
    { "name", "na", "string", "Get / set the name of the current actuator", false, true, &s_name, eCommandType_data::ectSimpleString | eCommandType_function::ectName },
    //{ "data", "da", "name:rest:engage:stall", "Set actuator data", false, true, &s_data, eCommandType_data::ectData | eCommandType_function::ectParameter }
    { "restpressure", "rp", "0-65535", "Set the rest pressure of the current actuator", false, true, &s_restPressure, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectCalibration},
    { "stallpressure", "sp", "0-65535","Set the stall pressure of the current actuator", false, true, &s_stallPressureF, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectCalibration},
    { "engagepressure", "ep", "0-65535","Set the engage pressure of the current actuator", false, true, &s_engagePressure, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectCalibration}
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
/*
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
*/
CREATE_MODULE_COMMAND_FUNCTION(restPressure, BowActuator) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        restPosition = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(restPosition), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(stallPressureF, BowActuator) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        stallPressure = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(stallPressure), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(engagePressure, BowActuator) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        firstTouchPressure = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(firstTouchPressure), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

#endif // BOWACTUATOR_CPP
