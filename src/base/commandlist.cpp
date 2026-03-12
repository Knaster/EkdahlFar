#ifndef COMMANDLIST_CPP
#define COMMANDLIST_CPP

#include "commandlist.hpp"

const uint8_t quoteStyle[] = { '\'', '\"' };

/// Class containing an array of commandItem objects and ways of processing them
void CommandList::waitIfProcessing() {
    if (processingCommands) {
        debugPrintln("Waited for processing", debugPrintType::Debug);
    }
    while (processingCommands);
}

int CommandList::parseQuote(String commandString, char quote, int start) {
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
    } while ((commandString[start] != quoteStyle[quote]) && (start < (int(commandString.length()) - 1)));
    debugPrintln("Found quote end @ " + String(start), debugPrintType::EParser);
    return start;
}

int CommandList::parseBracket(String commandString, int start) {
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
void CommandList::addCommands(String commandItems) {
    debugPrintln("Building command list", debugPrintType::EParser);
    waitIfProcessing();
    commandItems.toLowerCase();
    commandItems = stripQuotes(commandItems);
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

        debugPrintln("New string from " + String(i) + " to " + String(f) + " is " + commandItems.substring(i, f), EParser);
        debugPrintln("Creating command string " + stripQuotes(commandItems.substring(i, f)), EParser);
        String strippedCmd = stripQuotes(commandItems.substring(i, f));
        item.push_back(CommandItem(&strippedCmd));

        i = e;
        i++;
        debugPrintln("Command created", EParser);
    }
    debugPrintln("List created", EParser);
    return;
}

CommandList::CommandList(String commandItems) {
    addCommands(commandItems);
}

CommandList& CommandList::operator = (const CommandList& other) {
    if (this == &other)
        return *this;

    for (int i = 0; i<other.item.size(); i++) {
        item.push_back(other.item[i]);
    }

    return *this;
}

CommandList::CommandList() {}

#endif
