#ifndef GLOBALGENERICS_CPP
#define GLOBALGENERICS_CPP

void processCommandList(Module *inModule, commandList *inCommands, std::vector<commandResponse> *commandResponses);

//extern unsigned long _heap_start;
//extern unsigned long _heap_end;
//extern char *__brkval;

int freeram();

#endif
