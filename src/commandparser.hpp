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
#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <WProgram.h>
#include <vector>
#include "expressionparser.cpp"

const uint8_t quoteStyle[] = { '\'', '\"' };

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
//    String prefix = "";
    String originalCommand = "";
    SELECTION selection;

    commandItem() {
        argument.clear();
        command = "";
        hierarchy.clear();
        hierarchyIndex = 0;
        originalCommand = "";
        selection.clear();
    }

    /// Parses commands of style command:argument1:argument2 [etc..]
    commandItem (String commandString) {
        int i = commandString.indexOf(":");
        if (i == -1) {
            command = commandString;
            debugPrintln("Command is " + command, debugPrintType::EParser);
        } else {
            command = commandString.substring(0, i).toLowerCase();
            debugPrintln("Command is " + command, debugPrintType::EParser);
            while (i < (int(commandString.length()) - 1)) {
                i++;
                int e = commandString.indexOf(":", i);
                int f = commandString.indexOf("'", i);
                int h = commandString.indexOf("\"", i); // Added 2023-10-25
                if (((f != -1) && (f < e)) || ((h != -1) && (h < e))) { // Added 2023-10-25
                    String searchFor = "'"; // Added 2023-10-25
                    if (((h < f) && (h != -1)) || (f == -1)) { searchFor = "\""; f = h; } else { searchFor = "'"; } // Added 2023-10-25
                    int g = commandString.indexOf(searchFor,f + 1);  // Added 2023-10-25
                    if (g != -1) {
                        e = g + 1;
                    }
                };

                if (e == -1) {
                    e = commandString.length();
                }
                //String* _argument = new String(commandString.substring(i, e).toLowerCase());
                //debugPrintln("Argument " + *_argument, debugPrintType::EParser);
                //argument.push_back(*_argument);
                //delete _argument;

                String arg = commandString.substring(i, e).toLowerCase();
                argument.push_back(arg);
                i = e;
            }
        }
        buildHierarchy();
    }

    commandItem& operator = (const commandItem& other) {
        // Guard self assignment
        if (this == &other)
            return *this;

        this->command = other.command;
        this->hierarchyIndex = other.hierarchyIndex;
        this->argument = other.argument;
        this->hierarchy = other.hierarchy;
        this->originalCommand = other.originalCommand;
        this->selection = other.selection;

        return *this;
    }

    ~commandItem() {
    }

private:
    void buildHierarchy() {
        hierarchyIndex = 0;
        hierarchy.clear();

        int start = 0, end = 0;
        String portion;
        do {
            end = command.indexOf('.', start);
            if (end == -1) { end = command.length(); }
            portion = command.substring(start, end);

            SELECTION thisSel;
            thisSel.clear();

            //debugPrintln("Portion " + portion, debugPrintType::Debug);

            int i = portion.indexOf('[');
            int j = portion.lastIndexOf(']');

            if ((i != -1) && (j != -1 )) {
                i += 1;

                String sel = portion.substring(i, j);
                //debugPrintln("Adding selection: " + sel, debugPrintType::Debug);
                int l = 0, m, n, k = 0;
                while ((k < j)) {
                    k = portion.indexOf(',', i + 1);
                    if (k == -1) { k = j; };

                    sel = portion.substring(i, k);
                    l = sel.indexOf('-');
                    if (l == -1) {
                        //debugPrintln("Adding " + sel, debugPrintType::Debug);
                        thisSel.push_back(sel.toInt());
                    } else {
                        m = sel.substring(0, l).toInt();
                        n = sel.substring(l + 1, sel.length()).toInt();
                        if ((n < m) || (m < 0) || (n < 0) || (m > 255) || (n > 255)) {
                            debugPrintln("Invalid selection range", Error);
                            return;
                        }
                        for (m=m; m<=n; m++) {
                            thisSel.push_back(m);
                            //debugPrintln("Adding " + String(m), debugPrintType::Debug);
                        }
                    }
                    i = k + 1;
                }

                portion = portion.substring(0, portion.indexOf('['));
                //debugPrintln("New portion is " + portion, debugPrintType::Debug);
            }

            hierarchy.push_back({ portion, thisSel });
            start = end + 1;
        } while (end < command.length());

    }
};


//TODO remove this forwards declaration once headers have been made
String stripQuotes(String expression);

/// Class containing an array of commandItem objects and ways of processing them
class commandList {
public:
    std::vector<commandItem> item;
    bool processingCommands = false;

    void waitIfProcessing() {
        if (processingCommands) {
            debugPrintln("Waited for processing", debugPrintType::Debug);
        }
        while (processingCommands);
    }

    int parseQuote(String commandString, char quote, int start) {
        int otherQuote = 0;
        if (quote == 0) { otherQuote = 1; }
        else if (quote == 1) { otherQuote = 0; }
        else { debugPrintln("MAJOR ERROR IN QUOTE PARSER", debugPrintType::EParser); }

        debugPrintln("Found quote start @ " + String(start), debugPrintType::EParser);
        do {
            start++;
            if (String(commandString[start]) == quoteStyle[otherQuote]) {
                start = parseQuote(commandString, otherQuote, start);
            }
            //debugPrintln("start: " + String(start) + " length: " + String(commandString.length()) + " is quote style:" + String(String(commandString[start]) == quoteStyle[quote]) + " aka char -" + commandString[start] + "-", debugPrintType::Debug);

//        } while ((String(commandString[start]) != quoteStyle[quote]) && (start < (int(commandString.length()) - 1)));
        } while ((commandString[start] != quoteStyle[quote]) && (start < (int(commandString.length()) - 1)));
        debugPrintln("Found quote end @ " + String(start), debugPrintType::EParser);
        return start;
    }

    int parseBracket(String commandString, int start) {
        debugPrintln("Found bracket start @ " + String(start), debugPrintType::EParser);
        do {
            start++;
            if (String(commandString[start]) == '[') {
                start = parseBracket(commandString, start);
            }
        } while ((String(commandString[start]) != ']') && (start < (int(commandString.length()) - 1)));
        debugPrintln("Found bracket end @ " + String(start), debugPrintType::EParser);
        return start;
    }

  /// Parses a string of commands into several commandItem objects
    void addCommands(String commandItems) {
        debugPrintln("Building command list", debugPrintType::EParser);
        waitIfProcessing();
        commandItems = stripQuotes(commandItems.toLowerCase());
        int i = 0;
        int e = 0;
        int f = 0;
        while (i < (int(commandItems.length()) - 1)) {
            debugPrintln("Looking for ',' @ " + String(i), debugPrintType::EParser);
            e = i;
            while ((e < (int(commandItems.length()) - 1)) && (String(commandItems[e]) != ",")) {

                if (char(commandItems[e]) == quoteStyle[0]) {
                    e = parseQuote(commandItems, 0, e);
                } else
                if (char(commandItems[e]) == quoteStyle[1]) {
                    e = parseQuote(commandItems, 1, e);
                } else
                if (char(commandItems[e]) == '[') {
                    e = parseBracket(commandItems, e);
                };
                e++;
            }
            debugPrintln("Found ',' @ " + String(e), debugPrintType::EParser);

            if (String(commandItems[e]) != ",") {
                e = commandItems.length();
            }
            f = e;

            //String* _item = new String(stripQuotes(commandItems.substring(i, f).toLowerCase()));
            //debugPrintln("New string from " + String(i) + " to " + String(f) + " is " + commandItems.substring(i, f).toLowerCase(), EParser);

            //debugPrintln("Creating command string " + *_item, EParser);
            //commandItem *_commandItem = new commandItem(*_item);

            //item.push_back(*_item);
            //delete _item;
            //delete _commandItem;

            debugPrintln("New string from " + String(i) + " to " + String(f) + " is " + commandItems.substring(i, f).toLowerCase(), EParser);
            debugPrintln("Creating command string " + stripQuotes(commandItems.substring(i, f).toLowerCase()), EParser);
            item.push_back(commandItem(stripQuotes(commandItems.substring(i, f).toLowerCase())));

            i = e;
            i++;
            debugPrintln("Command created", EParser);
        }
        debugPrintln("List created", EParser);
        return;
    }

    void parseCommandExpressions(ExpressionParser expressionParser) { //(te_variable vars[], int varCount) {
        waitIfProcessing();
        String debugOut;

        debugOut = "Command sequence ";
        for (int i = 0; i < int(item.size()); i++) {
            debugOut += item[i].command;
            for (int j = 0; j < int(item[i].argument.size()); j++) {
                item[i].argument[j] = stripQuotes(item[i].argument[j]);

                debugOut +=  ":";
                item[i].argument[j] = expressionParser.parseCommandExpressions(item[i].argument[j]);
                debugOut += item[i].argument[j];
            }
            debugOut += " ";
        }
        debugPrintln(debugOut, EParser);
        return;
    }

    commandList(String commandItems) {
        addCommands(commandItems);
    }

    commandList& operator = (const commandList& other) {
        //debugPrintln("in the operator", debugPrintType::Debug);
        // Guard self assignment
        if (this == &other)
            return *this;

        //debugPrintln("passed self assignment", debugPrintType::Debug);
        for (int i = 0; i<other.item.size(); i++) {
            //debugPrintln("Adding item ", debugPrintType::Debug);
            item.push_back(other.item[i]);
        }

        return *this;
    }

    commandList() {

    }
};

/// Structure for containing help information as long as short-to-long name command name parsing
struct serialCommandItem {
    String longCommand;
    String shortCommand;
    String arguments;
    String Help;
};

/// Preprocesses a commandItem, replacing any short command names with long ones and turns them all into lowercase
bool processCommandItems(commandItem *_commandParser, const serialCommandItem serialCommandArray[], int size) {
    for (int i = 0; i<size; i++) {
        if ((_commandParser->command == serialCommandArray[i].shortCommand)) {
            _commandParser->command = serialCommandArray[i].longCommand.toLowerCase();
        }
    }
    return false;
}

/// Prints all help data for commands given in the serialCommandArray
String printCommandHelp(serialCommandItem serialCommandArray[], int size) {
    String help = "";
    for (int i = 0; i<size; i++) {
        help += "[";
        if (serialCommandArray[i].longCommand != "") { help += serialCommandArray[i].longCommand; }
        if (serialCommandArray[i].shortCommand != "") { help += "|"; }
        if (serialCommandArray[i].shortCommand != "") { help += serialCommandArray[i].shortCommand; }
        help += "]";
        if (serialCommandArray[i].arguments != "") { help += ":[" + serialCommandArray[i].arguments + "]"; }
        help += " " + serialCommandArray[i].Help + "\n";
    }
    return help;
}

/// Prints all help data for commands given in the serialCommandArray
String addCommandHelp(const serialCommandItem serialCommandArray[], int size, std::vector<commandResponse> *commandResponses, String prefix = "") {
    String help = "";
    if (prefix == "") { prefix = "[unk]:"; }
    for (int i = 0; i<size; i++) {
        help = prefix;
        if (serialCommandArray[i].longCommand != "") { help += serialCommandArray[i].longCommand; }
        if (serialCommandArray[i].shortCommand != "") { help += "|"; }
        if (serialCommandArray[i].shortCommand != "") { help += serialCommandArray[i].shortCommand; }
        if (serialCommandArray[i].arguments != "") { help += ":" + serialCommandArray[i].arguments + ":"; }
        help += "\"" + serialCommandArray[i].Help + "\""; // + "\n";
        commandResponses->push_back({help, debugPrintType::Help});
    }
    return help;
}

bool checkArguments(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t arguments, bool t_supressError = false) {
    if (_commandItem->argument.size() != arguments) {
        if (!t_supressError) {
            commandResponses->push_back({"Wrong number of arguments, got " + String(_commandItem->argument.size()) + " expected " + String(arguments), Error});
        }
        return false;
    }
    return true;
}

bool checkArgumentsMin(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t argumentsMin, bool t_supressError = false) {
    if (_commandItem->argument.size() < argumentsMin) {
        if (!t_supressError) { commandResponses->push_back({"Wrong number of arguments", Error}); };
        return false;
    }
    return true;
}

bool validateNumber(int16_t number, int16_t min, int16_t max, bool t_supressError = false) {
    if ((number < min) || (number > max)) {
        if (!t_supressError) {debugPrintln("Number out of bounds", Error); }
        return false;
    }
    return true;
}

String delimitExpression(String expression, bool force) {
    String delimit = "";

    if (expression == "") { return expression; }

    if (((expression[0] == "'") or (expression[0] == "\"")) and (expression[0] == expression[expression.length() - 1])) {
//        debugPrintln("-" + expression + "- is already delimited!", debugPrintType::Debug);
        return expression;
    }

    int singleQ = expression.indexOf("'");
    int doubleQ = expression.indexOf("\"");

    if (singleQ < doubleQ) {
        delimit = "'";
    } else if (singleQ > doubleQ) {
        delimit = "\"";
    }

//    debugPrintln("exp -" + expression + "-", debugPrintType::Debug);
    if (!((singleQ == 0) || (doubleQ == 0))) {
        if (force) {
//            debugPrintln("force", debugPrintType::Debug);
            delimit = "'";
        } else {
            delimit = "";
        }
    } else {
//        debugPrintln("has quote; singleQ: " + String(singleQ) + ", doubleQ: " + String(doubleQ), debugPrintType::Debug);
    }

    expression = delimit + expression + delimit;
//    debugPrintln("new exp -" + expression + "-", debugPrintType::Debug);

    if (delimit != "") {
//        debugPrintln("Using quotestyle " + delimit + ", complete expression is " + expression, debugPrintType::Debug);
    }

    return expression;
}

String stripQuotes(String expression) {
    // 1. strip starting and trailing white spaces
    // 2. strip starting and trailing commas
    // 3. check if beginning starts with a quote
    //  3.1 make sure that the end contains the same quote, if not, signal an error
    //  3.2 strip quotes from start and end
    expression = expression.trim();

    if ((String(expression[0]) == "\"") || (String(expression[0]) == "'")) {
//        debugPrintln("Stripping start & end quotes from -" + expression + "-", debugPrintType::Debug);
        if (expression[0] != expression[expression.length()-1]) {
//            debugPrintln("ERROR Quote start and end doesn't match", debugPrintType::Error);
        }
        expression = expression.substring(1, expression.length() - 1);
//        debugPrintln("Stripped expression is -" + expression + "-", debugPrintType::Debug);
    } else {
//        debugPrintln("The first character is -" + String(expression[0]) + "-", debugPrintType::Debug);
//        debugPrintln("No quotes found in the string -" + expression + "-", debugPrintType::Debug);
    }

    return expression;
}

String removePrefix(String input, String prefix) {
    prefix = prefix.replace(".", "");

    if (input.substring(0, prefix.length()) == prefix) {
        input = input.substring(prefix.length());
    }

    return input;
}


#endif
