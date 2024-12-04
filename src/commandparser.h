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
#include "tinyexpr.h"

/// Class containing command items consisting of one command and any number of arguments
class commandItem {
public:
  std::vector<String> argument;
  String command = "";

//  commandItem() {}

/// Parses commands of style command:argument1:argument2 [etc..]
  commandItem (String commandString) {
    int i = commandString.indexOf(":");
    if (i == -1) {
      command = commandString;
      return;
    }
    command = commandString.substring(0, i).toLowerCase();
//    debugPrintln("Adding command " + command, Debug);
    while (i < (int(commandString.length()) - 1)) {
      i++;
      int e = commandString.indexOf(":", i);
      // If we find quotes, and they come before the next command separator, process the quotes
      int f = commandString.indexOf("'", i);
      int h = commandString.indexOf("\"", i); // Added 2023-10-25
//      if ((f != -1) && (f < e)) { // Commented out 2023-10-25
      if (((f != -1) && (f < e)) || ((h != -1) && (h < e))) { // Added 2023-10-25
        String searchFor = "'"; // Added 2023-10-25
        if (((h < f) && (h != -1)) || (f == -1)) { searchFor = "\""; f = h; } else { searchFor = "'"; } // Added 2023-10-25
        int g = commandString.indexOf(searchFor,f + 1);  // Added 2023-10-25
//        debugPrintln("Found other  " + searchFor + " at " + String(g), Debug);  // Added 2023-10-25
//        int g = commandString.indexOf("'",f + 1); // Commented out 2023-10-25
        if (g != -1) {
          e = g + 1;
        }
      };

      if (e == -1) {
        e = commandString.length();
      }
      String* _argument = new String(commandString.substring(i, e).toLowerCase());
      argument.push_back(*_argument);
//      debugPrintln("Adding argument " + *_argument, Debug);
      delete _argument;
      i = e;
    }
  }

  ~commandItem() {
    //debugPrintln("Item destroyed");
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
        while (processingCommands);
    }

    int parseQuoteSingle(String commandItems, int i) {
        debugPrintln("Found single quote start @ " + String(i), debugPrintType::EParser);
        do {
            i++;
            if (String(commandItems[i]) == "\"") {
                i = parseQuoteSingle(commandItems, i);
            }
        } while ((String(commandItems[i]) != "'") && (i < (int(commandItems.length()) - 1)));
        debugPrintln("Found single quote end @ " + String(i), debugPrintType::EParser);
        return i;
    }

    int parseQuoteDouble(String commandItems, int i) {
        debugPrintln("Found double quote start @ " + String(i), debugPrintType::EParser);
        do {
            i++;
            if (String(commandItems[i]) == "'") {
                i = parseQuoteSingle(commandItems, i);
            }
        } while ((String(commandItems[i]) != "\"") && (i < (int(commandItems.length()) - 1)));
        debugPrintln("Found double quote end @ " + String(i), debugPrintType::EParser);
        return i;
    }

  /// Parses a string of commands into several commandItem objects
    void addCommands(String commandItems) {
        waitIfProcessing();
        commandItems = commandItems.toLowerCase();
        int i = 0;
        int e = 0;
        int f = 0;
        bool stripEnd = false;
        while (i < (int(commandItems.length()) - 1)) {
            debugPrintln("Looking for ',' @ " + String(i), debugPrintType::EParser);
            e = i;
            while ((e < (int(commandItems.length()) - 1)) && (String(commandItems[e]) != ",")) {
                //debugPrintln("Checking " + String(commandItems[e]), debugPrintType::EParser);
                if (String(commandItems[e]) == "\"") {
                    e = parseQuoteDouble(commandItems, e);
                    if (i == e) {
                        debugPrintln("Double quote found at beginning of command, advancing command start and setting flag", debugPrintType::Debug);
                        i++;
                        stripEnd = true;
                    }
                }
                else
                if (String(commandItems[e]) == "'") {
                    e = parseQuoteSingle(commandItems, e);
                    if (i == e) {
                        debugPrintln("Single quote found at beginning of command, advancing command start and setting flag", debugPrintType::Debug);
                        i++;
                        stripEnd = true;
                    }
                }
    //            else {
                e++;
    //            }
            }
            debugPrintln("Found ',' @ " + String(e), debugPrintType::EParser);

            if (String(commandItems[e]) != ",") {
                e = commandItems.length();
            }
            f = e;

            if (((String(commandItems[f - 1]) == "\"") || (String(commandItems[f - 1]) == "'")) && (stripEnd)) {
                debugPrintln("Quote found at end of command, reverting command end", debugPrintType::Debug);
                f--;
                stripEnd = false;
            }

    /*      int e = commandItems.indexOf(",", i);
          int f = commandItems.indexOf("'", i);
          int h = commandItems.indexOf("\"",i);   // Added 2023-10-25

          debugPrintln("Found , @ " + String(e) + " ' @ " + String(f) + " \" @ " + String(h), EParser); // Added 2023-10-25

          // If we find quotes, and they come before the next command separator, process the quotes
    //      if ((f != -1) && (f < e)) {   // commented out 2023-10-25
          if (((f != -1) && (f < e)) || ((h != -1) && (h < e))) { // Added 2023-10-25
            debugPrintln("Found delimiter", EParser);
            String searchFor = "'"; // Added 2023-10-25
            if (((h < f) && (h != -1)) || (f == -1)) { searchFor = "\""; f = h; } else { searchFor = "'"; } // Added 2023-10-25
            int g = commandItems.indexOf(searchFor,f + 1);  // Added 2023-10-25
            debugPrintln("Found other  " + searchFor + " at " + String(g), EParser);  // Added 2023-10-25
    //        int g = commandItems.indexOf("'",f + 1); // commented out 2023-10-25
            if (g != -1) {
              e = g + 1;
            }
          };

          if (e == -1) {
            e = commandItems.length();
          }
    */
    //      String* _item = new String(commandItems.substring(i, e).toLowerCase());
    //      debugPrintln("New string from " + String(i) + " to " + String(e) + " is " + commandItems.substring(i, e).toLowerCase(), EParser);
            String* _item = new String(commandItems.substring(i, f).toLowerCase());
            debugPrintln("New string from " + String(i) + " to " + String(f) + " is " + commandItems.substring(i, f).toLowerCase(), EParser);

            debugPrintln("Creating command string " + *_item, EParser);
            commandItem *_commandItem = new commandItem(*_item);

            item.push_back(*_item);
            delete _item;
            delete _commandItem;
            i = e;
            i++;
        }
        return;
    }

  /// Parses out a commandItem objects arguments with a expression parser using a list of variables
    void parseCommandExpressions(te_variable vars[], int varCount) {
        waitIfProcessing();
        String debugOut;
        //debugPrint("Command sequence ", EParser);

        debugOut = "Command sequence ";
        for (int i = 0; i < int(item.size()); i++) {
          //debugPrint(item[i].command, EParser);
            debugOut += item[i].command;
            for (int j = 0; j < int(item[i].argument.size()); j++) {
            //debugPrint(":", EParser);

        //        debugPrintln("index of \" " + String(item[i].argument[j].indexOf("\"")) + ", ' " + String(item[i].argument[j].indexOf("'")), debugPrintType::Debug);
        /*        if ((item[i].argument[j].indexOf("\"") != -1) || (item[i].argument[j].indexOf("'") != -1)) {
                item[i].argument[j] = item[i].argument[j].substring(1, item[i].argument[j].length() - 1);
                debugPrintln("Cleaned expression " + item[i].argument[j], debugPrintType::Debug);
            }*/
                item[i].argument[j] = stripQuotes(item[i].argument[j]);

                debugOut +=  ":";
                int err;
                te_expr *n = te_compile(item[i].argument[j].c_str(), vars, varCount, &err);
                double r = -1;
                if (n) {
                  r = te_eval(n);
                  item[i].argument[j] = String(r, 5);
                } else {
                  //debugPrintln("Error in expression at " + item[i].argument[j].substring(err, item[i].argument[j].length() - 1));
                }
                //debugPrint(item[i].argument[j], EParser);
                debugOut += item[i].argument[j];
                te_free(n);
            }
          //debugPrint(" ", EParser);
            debugOut += " ";
        }
        //    debugPrintln("", EParser);
        debugPrintln(debugOut, EParser);
        return;
    }

    commandList(String commandItems) {
        addCommands(commandItems);
    }

    commandList() {

    }

    ~commandList() {
    //debugPrintln("List destroyed");
    }

//  commandList() {}
};

/// Structure for containing help information as long as short-to-long name command name parsing
struct serialCommandItem {
    String longCommand;
    String shortCommand;
    String arguments;
    String Help;
};

/// Preprocesses a commandItem, replacing any short command names with long ones and turns them all into lowercase
bool processCommandItems(commandItem *_commandParser, serialCommandItem serialCommandArray[], int size) {
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
String addCommandHelp(serialCommandItem serialCommandArray[], int size, std::vector<commandResponse> *commandResponses, String prefix = "") {
    String help = "";
    for (int i = 0; i<size; i++) {
//        help += "[";
        help = prefix; // + "[";
        if (serialCommandArray[i].longCommand != "") { help += serialCommandArray[i].longCommand; }
        if (serialCommandArray[i].shortCommand != "") { help += "|"; }
        if (serialCommandArray[i].shortCommand != "") { help += serialCommandArray[i].shortCommand; }
        //help += "]";
        if (serialCommandArray[i].arguments != "") { help += ":" + serialCommandArray[i].arguments + ":"; }
        help += "\"" + serialCommandArray[i].Help + "\""; // + "\n";
        commandResponses->push_back({help, debugPrintType::Help});
    }
    return help;
}
/*
bool skipToCommand(commandList *commands, uint16_t *index, String command) {
  uint16_t j = *index;
  debugPrintln("Starting test at " + String(j), Debug);
  do {
    j++;
    if (j >= commands->item.size()) { break; }
  } while (commands->item[j].command != command);

  debugPrintln("Ending loop at " + String(j) + ", commands length " + String(commands->item.size()), Debug);

  if (j >= commands->item.size()) {
    debugPrintln("Termination not found, skipping", Error);
    return false;
  } else
  if (commands->item[j].command == command) {
    *index = j;
    debugPrintln("Skipping forward to command " + String(j), Debug);
    return true;
  }
  return false;
}
*/
bool checkArguments(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t arguments, bool t_supressError = false) {
    if (_commandItem->argument.size() != arguments) {
        if (!t_supressError) {
            commandResponses->push_back({"Wrong number of arguments " + String(_commandItem->argument.size()), Error});
        }
        return false;
    }
    return true;
}

bool checkArgumentsMin(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, uint8_t argumentsMin) {
    if (_commandItem->argument.size() < argumentsMin) {
        commandResponses->push_back({"Wrong number of arguments", Error});
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

String delimitExpression(String expression, bool force = false) {
    String delimit = "";

    int singleQ = expression.indexOf("'");
    int doubleQ = expression.indexOf("\"");

    if (singleQ < doubleQ) {
        delimit = "'";
    } else if (singleQ > doubleQ) {
        delimit = "\"";
    }

    //if ((force) && (delimit == "") && ((singleQ > -1) || (doubleQ > -1))) { delimit = "'"; }
    if (force) { delimit = "'"; }
    if ((singleQ == 0) || (doubleQ == 0)) { delimit = ""; }

    expression = delimit + expression + delimit;

    if (delimit != "") {
//        debugPrintln("Using quotestyle " + delimit + ", complete expression is " + expression, debugPrintType::Debug);
    }

    return expression;
}

String stripQuotes(String expression) {
/*      if ((expression.indexOf("\"") != -1) || (expression.indexOf("'") != -1)) {

        debugPrintln("Cleaned expression " + item[i].argument[j], debugPrintType::Debug);
    }*/
    // 1. strip starting and trailing white spaces
    // 2. strip starting and trailing commas
    // 3. check if beginning starts with a quote
    //  3.1 make sure that the end contains the same quote, if not, signal an error
    //  3.2 strip quotes from start and end
    expression = expression.trim();

    if ((String(expression[0]) == "\"") || (String(expression[0]) == "'")) {
        debugPrintln("Stripping start & end quotes from -" + expression + "-", debugPrintType::Debug);
        if (expression[0] != expression[expression.length()-1]) {
            debugPrintln("ERROR Quote start and end doesn't match", debugPrintType::Error);
        }
        expression = expression.substring(1, expression.length() - 1);
//        debugPrintln("Stripped expression is -" + expression + "-", debugPrintType::Debug);
    } else {
//        debugPrintln("The first character is -" + String(expression[0]) + "-", debugPrintType::Debug);
//        debugPrintln("No quotes found in the string -" + expression + "-", debugPrintType::Debug);
    }

    return expression;
}

#endif
