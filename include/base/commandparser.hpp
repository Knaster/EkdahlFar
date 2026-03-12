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
#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include "base/arduinorequired.hpp"
#include "commanditem.hpp"

#define commandResponseType debugPrintType

struct commandResponse {
    String response;
    commandResponseType responseType;
};

bool checkArguments(CommandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t arguments, bool t_supressError = false);

bool checkArgumentsMin(CommandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t argumentsMin, bool t_supressError = false);

void printResponses(std::vector<commandResponse> *commandResponses, bool isInternal = false);

String buildCommandString(CommandItem *inCommandItem);

#endif
