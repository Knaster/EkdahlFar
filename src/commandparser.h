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
      //debugPrintln("Adding argument " + *_argument, Debug);
      delete _argument;
      i = e;
    }
  }

  ~commandItem() {
    //debugPrintln("Item destroyed");
  }
};

/// Class containing an array of commandItem objects and ways of processing them
class commandList {
public:
  std::vector<commandItem> item;
  bool processingCommands = false;

  void waitIfProcessing() {
    while (processingCommands);
  }

  /// Parses a string of commands into several commandItem objects
  void addCommands(String commandItems) {
    waitIfProcessing();
    commandItems = commandItems.toLowerCase();
    int i = 0;
    while (i < (int(commandItems.length()) - 1)) {
      int e = commandItems.indexOf(",", i);
      int f = commandItems.indexOf("'", i);
      int h = commandItems.indexOf("\"",i);   // Added 2023-10-25

  //    debugPrintln("Found , @ " + String(e) + " ' @ " + String(f) + " \" @ " + String(h), Debug); // Added 2023-10-25

      // If we find quotes, and they come before the next command separator, process the quotes
//      if ((f != -1) && (f < e)) {   // commented out 2023-10-25
      if (((f != -1) && (f < e)) || ((h != -1) && (h < e))) { // Added 2023-10-25
//        debugPrintln("Found delimiter", Debug);
        String searchFor = "'"; // Added 2023-10-25
        if (((h < f) && (h != -1)) || (f == -1)) { searchFor = "\""; f = h; } else { searchFor = "'"; } // Added 2023-10-25
        int g = commandItems.indexOf(searchFor,f + 1);  // Added 2023-10-25
//        debugPrintln("Found other  " + searchFor + " at " + String(g), Debug);  // Added 2023-10-25
//        int g = commandItems.indexOf("'",f + 1); // commented out 2023-10-25
        if (g != -1) {
          e = g + 1;
        }
      };

      if (e == -1) {
        e = commandItems.length();
      }
      String* _item = new String(commandItems.substring(i, e).toLowerCase());
//      debugPrintln("New string from " + String(i) + " to " + String(e) + " is " + commandItems.substring(i, e).toLowerCase(), Debug);
      //debugPrintln("Creating command string " + *_item, EParser);
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
        debugOut +=  ":";
        int err;
        te_expr *n = te_compile(item[i].argument[j].c_str(), vars, varCount, &err);
        double r = -1;
        if (n) {
          r = te_eval(n);
          item[i].argument[j] = String(r);
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
bool validateNumber(int16_t number, int16_t min, int16_t max) {
    if ((number < min) || (number > max)) {
        debugPrintln("Number out of bounds", Error);
        return false;
    }
    return true;
}

#endif
