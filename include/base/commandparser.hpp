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
#include "master_controller/expressionparser.h"

extern const uint8_t quoteStyle[];

#define SELECTION std::vector<uint8_t>

struct commandItemPart {
    String name;
    SELECTION selection;
};

/// Class containing command items consisting of one command and any number of arguments
class commandItem {
public:
    std::vector<String> argument;
    String command = "";
    std::vector<commandItemPart> hierarchy;
    int hierarchyIndex = 0;
    String originalCommand = "";
    SELECTION selection;

    commandItem();
    commandItem (String commandString);
    commandItem& operator = (const commandItem& other);

private:
    void buildHierarchy();
};

/// Class containing an array of commandItem objects and ways of processing them
class commandList {
public:
    std::vector<commandItem> item;
    bool processingCommands = false;

    void waitIfProcessing();

    int parseQuote(String commandString, char quote, int start);

    int parseBracket(String commandString, int start);

  /// Parses a string of commands into several commandItem objects
    void addCommands(String commandItems);

    void parseCommandExpressions(ExpressionParser expressionParser);

    commandList(String commandItems);

    commandList& operator = (const commandList& other);

    commandList();
};

/// Structure for containing help information as long as short-to-long name command name parsing
struct serialCommandItem {
    String longCommand;
    String shortCommand;
    String arguments;
    String Help;
};

/// Preprocesses a commandItem, replacing any short command names with long ones and turns them all into lowercase
bool processCommandItems(commandItem *_commandParser, const serialCommandItem serialCommandArray[], int size);
/// Prints all help data for commands given in the serialCommandArray
String printCommandHelp(serialCommandItem serialCommandArray[], int size);
/// Prints all help data for commands given in the serialCommandArray
String addCommandHelp(const serialCommandItem serialCommandArray[], int size, std::vector<commandResponse> *commandResponses, String prefix = "");

bool checkArguments(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t arguments, bool t_supressError = false);
bool checkArgumentsMin(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t argumentsMin, bool t_supressError = false);
bool validateNumber(int16_t number, int16_t min, int16_t max, bool t_supressError = false);

String delimitExpression(String expression, bool force = false);
String stripQuotes(String expression);
String removePrefix(String input, String prefix);

extern commandList globalCommands;
extern commandList globalResponseCommands;

#endif
