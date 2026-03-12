#ifndef COMMANDLIST_HPP
#define COMMANDLIST_HPP

#include <vector>
#include <commanditem.hpp>

extern const uint8_t quoteStyle[];

/// Class containing an array of commandItem objects and ways of processing them
class CommandList {
public:
    std::vector<CommandItem> item;
    bool processingCommands = false;

    void waitIfProcessing();

    int parseQuote(String commandString, char quote, int start);

    int parseBracket(String commandString, int start);

  /// Parses a string of commands into several commandItem objects
    void addCommands(String commandItems);

//    void parseCommandExpressions(ExpressionParser expressionParser);

    CommandList(String commandItems);

    CommandList& operator = (const CommandList& other);

    CommandList();
};
#endif // COMMANDLIST_HPP
