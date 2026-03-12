#ifndef COMMANDITEM_HPP
#define COMMANDITEM_HPP

#include "base/arduinorequired.hpp"
#include <vector>
#include "generalhelpers.hpp"

#define SELECTION std::vector<uint8_t>

struct CommandItemPart {
    String name;
    SELECTION selection;
};

/// Class containing command items consisting of one command and any number of arguments
class CommandItem {
public:
    std::vector<String> argument;
//    String command = "";
    std::vector<CommandItemPart> hierarchy;
    int hierarchyIndex = 0;
    //String originalCommand = "";
    SELECTION selection;

    CommandItem();
    CommandItem (String *commandString);
    CommandItem& operator = (const CommandItem& other);

private:
    void buildHierarchy(String command);
};

#endif // COMMANDITEM_HPP
