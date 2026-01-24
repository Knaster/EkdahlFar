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
#include "bowActuators.hpp"


const ModuleCommandDeclaration BowActuators::moduleCommands[] = {
    { "add", "a", "(name):(rest:engage:stall)", "Add new actuator with the given name and parameters", false, false, &s_add},
    { "remove", "rm", "actuator", "Remove bow actuator", false, false, &s_remove},
    { "count", "c", "-", "Returns the amount of saved bow actuators", false, false, &s_count},
};

getModuleCount(BowActuators)

BowActuators::BowActuators(BowPressure *inBowPressure)
{
    moduleID = new ModuleID("actuatorhandler", "ah", "Bow actuator handler", ModuleID::software);

    bowPressure = inBowPressure;

    BowActuator ba;
    ModuleGroup *group = addGroup(ba.moduleID);
    addBowActuator();
    group->mustHaveSelection = true;
    group->singleSelection = true;
    group->setIndexCallback(this, &s_actuatorIndexChanged);
    group->setSelection(0);
}

CREATE_INDEX_CALLBACK(actuatorIndexChanged, BowActuators) {
//    debugPrintln("Actuator index changed", debugPrintType::Debug);
    return true;
}

CREATE_MODULE_COMMAND_FUNCTION(add, BowActuators) {
    BowActuator *actuator;

    switch(inCommandItem->argument.size()) {
    case 0:
        actuator = addBowActuator();
        break;
    case 1:
        actuator = addBowActuator(inCommandItem->argument[0]);
        break;
    case 4:
        actuator = addBowActuator(inCommandItem->argument[0], inCommandItem->argument[1].toInt(), inCommandItem->argument[2].toInt(), inCommandItem->argument[3].toInt());
        break;
    default:
        return eProcessResult::WrongArgumentCount;
    }

    inCommandResponses->push_back({ thisItem.shortCommand + ":" + actuator->id + ":" + actuator->restPosition+ ":" + actuator->firstTouchPressure  + ":" + actuator->stallPressure,
                                  debugPrintType::InfoRequest });

    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(remove, BowActuators) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }

        ModuleGroup *group = getGroup("actuator");
        if (group == nullptr) { return eProcessResult::CommandFailed; }

        if (!request) {
            if (!group->removeModule(inCommandItem->argument[0].toInt())) {
                return eProcessResult::CommandFailed;
            }
        }
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(count, BowActuators) {
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + getGroup("actuator")->modules.size(), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

BowActuator* BowActuators::addBowActuator(String name, uint16_t rest, uint16_t engage, uint16_t stall) {
    BowActuator *lBowActuator = new BowActuator();
    ModuleGroup *group = addModule(lBowActuator);
    BowActuator *actuator = group->modules.back();
    actuator->id = name;
    actuator->firstTouchPressure = engage;
    actuator->restPosition = rest;
    actuator->stallPressure = stall;

    debugPrint("adding actuator with values " + actuator->id + "," + String(actuator->restPosition) + ":" + String(actuator->firstTouchPressure) + ":" + String(actuator->stallPressure), debugPrintType::Debug);
    return actuator;
}

bool BowActuators::removeBowActuator(uint8_t t_actuator) {
    ModuleGroup *group = getGroup("bowactuators");
    if (group == nullptr) { return eProcessResult::CommandFailed; }
    return group->removeModule(t_actuator);
}

void BowActuators::dumpData(std::vector<commandResponse> *dataDump) {
/*    std::vector<commandResponse> commandResponses;
    commandResponses = ModuleHandler::dumpData();
    commandResponses.push_back({ "ac[" + String(moduleGroups[0].selection[0]) + "]", debugPrintType::InfoRequest });
    return commandResponses;*/
    ModuleHandler::dumpData(dataDump);
}

#endif
