#ifndef GLOBALGENERICS_CPP
#define GLOBALGENERICS_CPP

#include <base/commandparser.hpp>
#include <base/module.hpp>
#include "master_controller/global_generics.hpp"
#include <commandlist.hpp>

void processCommandList(Module *inModule, CommandList *inCommands, std::vector<commandResponse> *commandResponses) {
    uint16_t i = 0;
    while (i < inCommands->item.size()) {
        inModule->processCommands(&(inCommands->item[i]), commandResponses);
        i++;
    }
}

extern unsigned long _heap_end;
extern char *__brkval;

int freeram() {
  return (char *)&_heap_end - __brkval;
}

#endif // GLOBALGENERICS_CPP
