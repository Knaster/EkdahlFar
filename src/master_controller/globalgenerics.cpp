#ifndef GLOBALGENERICS_CPP
#define GLOBALGENERICS_CPP

#include <base/commandparser.hpp>
#include <base/module.hpp>
#include "master_controller/global_generics.hpp"
#include <commandlist.hpp>

void processCommandList(Module *inModule, CommandList *inCommands, std::vector<commandResponse> *commandResponses) {
    uint16_t i = 0;
    //debugPrintln("Internal list processing started with " + String(inCommands->item.size()) + " commands", debugPrintType::Debug);
    while (i < inCommands->item.size()) {
        String out = "Processing command " + String(i) + " ";
        for (int j = 0; j < inCommands->item[i].hierarchy.size(); j++) {
            out += inCommands->item[i].hierarchy[j].name;
            if (j < inCommands->item[i].hierarchy.size() - 1) { out += "."; }
        }
        out += ":";
        for (int j = 0; j < inCommands->item[i].argument.size(); j++) {
            out += inCommands->item[i].argument[j];
            if (j < inCommands->item[i].argument.size() - 1) { out += "."; }
        }

        //debugPrintln("Internally processing " + out, debugPrintType::Debug);

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
