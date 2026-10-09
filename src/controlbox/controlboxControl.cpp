/*
 * This file is part of The Ekdahl FAR firmware.
 *
 * The Ekdahl FAR firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Ekdahl FAR firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Ekdahl FAR firmware. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2024 Karl Ekdahl
 */
#ifndef BOWACTUATORS_C
#define BOWACTUATORS_C

/*
    Conforming to STD C++ naming conventions with the exception of where external non-conforming items are referenced
*/

#include <vector>
#include "controlbox/controlboxControl.hpp"


const ModuleCommandDeclaration ControlboxControl::moduleCommands[] = {
    { "name", "na", "string", "Gets the name of the control", false, false, &s_name, eCommandType_data::ectSimpleString | eCommandType_function::ectName | eCommandType_access::ectRequest },
    { "commands", "cm", "string", "Sets the output commands of the control", false, true, &s_commands, eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment |
        eCommandType_dataOptions::ectExpression, "value" },
    { "value", "va", "int", "Returns the last value read", false, false, &s_value, eCommandType_data::ectSimpleInt16 | eCommandType_function::ectLiveStatistics | eCommandType_access::ectRequest },
    { "averages", "av", "int", "Sets the number of averages used while sampling", false, true, &s_averages, eCommandType_data::ectSimpleInt8 | eCommandType_function::ectAdminSetting },
    { "interruptthreshold", "it", "int", "Sets the sample difference needed to register as a change and start continuous sampling", false, true, &s_interruptthreshold,
        eCommandType_data::ectSimpleInt16 | eCommandType_function::ectAdminSetting },
    { "continuousthreshold", "ct", "int", "Sets the sample difference needed to register as a changed *during* continuous sampling", false, true, &s_continuousthreshold,
        eCommandType_data::ectSimpleInt16 | eCommandType_function::ectAdminSetting },
    { "continuoustimeout", "co", "int", "Sets the amount of milliseconds that needs to lapse without any new data before exiting continuous sampling mode", false, true, &s_continuoustimeout,
        eCommandType_data::ectMilliseconds | eCommandType_function::ectAdminSetting },
};

getModuleCount(ControlboxControl)

ControlboxControl::ControlboxControl(String inName, String inDefaultCommands, bool inTrigger)
{
    pName = inName;
    defaultCommands = inDefaultCommands;
    outputCommands = inDefaultCommands;
    averager.trigger = inTrigger;
}

CREATE_MODULE_COMMAND_FUNCTION(name, ControlboxControl) {
    if (request) {

        inCommandResponses->push_back({ thisItem.shortCommand + ":" + pName, debugPrintType::InfoRequest });
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(commands, ControlboxControl) {
    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 1, true)) { return eProcessResult::WrongArgumentCount; }
        outputCommands = stripQuotes(inCommandItem->argument[0]);
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(outputCommands, true), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(value, ControlboxControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(lastValue), debugPrintType::InfoRequest });
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(averages, ControlboxControl)  {
    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 1, true)) { return eProcessResult::WrongArgumentCount; }
        averager.dataAverageLength = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(averager.dataAverageLength), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(interruptthreshold, ControlboxControl)  {
    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 1, true)) { return eProcessResult::WrongArgumentCount; }
        averager.interruptedErrorThreshold = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(averager.interruptedErrorThreshold), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(continuousthreshold, ControlboxControl)  {
    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 1, true)) { return eProcessResult::WrongArgumentCount; }
        averager.continuousErrorThreshold = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(averager.continuousErrorThreshold), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(continuoustimeout, ControlboxControl)  {
    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 1, true)) { return eProcessResult::WrongArgumentCount; }
        averager.continuousTimeout = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(averager.continuousTimeout), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

void ControlboxControl::setDefaults() {
    averager.setDefaults();
    outputCommands = defaultCommands;
}

#endif
