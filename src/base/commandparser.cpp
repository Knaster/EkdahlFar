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
#ifndef COMMANDPARSER_CPP
#define COMMANDPARSER_CPP

#include "base/commandparser.hpp"

bool checkArguments(CommandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t arguments, bool t_supressError) {
    if (_commandItem->argument.size() != arguments) {
        if (!t_supressError) {
            commandResponses->push_back({"Wrong number of arguments, got " + String(_commandItem->argument.size()) + " expected " + String(arguments), Error});
        }
        return false;
    }
    return true;
}

bool checkArgumentsMin(CommandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t argumentsMin, bool t_supressError) {
    if (_commandItem->argument.size() < argumentsMin) {
        if (!t_supressError) { commandResponses->push_back({"Wrong number of arguments", Error}); };
        return false;
    }
    return true;
}

String buildCommandString(CommandItem *inCommandItem) {
    String commandString = "";
    uint8_t i, j;
    for (i = inCommandItem->hierarchyIndex; i < inCommandItem->hierarchy.size(); i++) {
        if (i > inCommandItem->hierarchyIndex) { commandString += "."; }
        commandString += inCommandItem->hierarchy[i].name;
        if (inCommandItem->hierarchy[i].selection.size() > 0) {
            commandString += "[";
            for (j = 0; j < inCommandItem->hierarchy[i].selection.size(); j++) {
                if (j > 0) { commandString += ","; }
                commandString += String(inCommandItem->hierarchy[i].selection[j]);
            }
            commandString += "]";
        }
    }
    for (j = 0; j < inCommandItem->argument.size(); j++) {
        commandString += ":" + delimitExpression(inCommandItem->argument[j], true);
    }
    return commandString;
}

#endif

