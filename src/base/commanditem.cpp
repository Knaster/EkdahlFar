#ifndef COMMANDITEM_CPP
#define COMMANDITEM_CPP

#include "commanditem.hpp"

CommandItem::CommandItem() {
    //argument.reserve(6);
    argument.clear();
//    command = "";
    //hierarchy.reserve(5);
    hierarchy.clear();
    hierarchyIndex = 0;
    //originalCommand = "";
    selection.clear();
}

/// Parses commands of style command:argument1:argument2 [etc..]
CommandItem::CommandItem (String *commandString) {
    int cmlength = commandString->length();
    int i = commandString->indexOf(":");
/*    if (i == -1) {
        command = *commandString;
        //debugPrintln("Command is " + command, debugPrintType::EParser);
    } else {*/
    if (i != -1) {
        //command = commandString->substring(0, i);
        cmlength = i;
//        command.toLowerCase();
        //debugPrintln("Command is " + command, debugPrintType::EParser);
        while (i < (int(commandString->length()) - 1)) {
            i++;
            int e = commandString->indexOf(":", i);
            int f = commandString->indexOf("'", i);
            int h = commandString->indexOf("\"", i); // Added 2023-10-25
            if (((f != -1) && (f < e)) || ((h != -1) && (h < e))) { // Added 2023-10-25
                //String searchFor = "'"; // Added 2023-10-25
                char searchFor = '\'';
//                if (((h < f) && (h != -1)) || (f == -1)) { searchFor = "\""; f = h; } else { searchFor = "'"; } // Added 2023-10-25
                if (((h < f) && (h != -1)) || (f == -1)) { searchFor = '"'; f = h; } else { searchFor = '\''; } // Added 2023-10-25
                int g = commandString->indexOf(searchFor,f + 1);  // Added 2023-10-25
                if (g != -1) {
                    e = g + 1;
                }
            };

            if (e == -1) {
                e = commandString->length();
            }

            String arg = commandString->substring(i, e);
            arg.toLowerCase();
            argument.push_back(arg);
            i = e;
        }
    }
    buildHierarchy(commandString->substring(0, cmlength));
}

CommandItem& CommandItem::operator = (const CommandItem& other) {
    // Guard self assignment
    if (this == &other)
        return *this;

//    this->command = other.command;
    this->hierarchyIndex = other.hierarchyIndex;
    this->argument = other.argument;
    this->hierarchy = other.hierarchy;
//    this->originalCommand = other.originalCommand;
    this->selection = other.selection;

    return *this;
}

void CommandItem::buildHierarchy(String command) {
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

#endif
