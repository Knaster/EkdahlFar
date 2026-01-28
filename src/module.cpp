#ifndef MODULE_CPP
#define MODULE_CPP

#include "module.hpp"

ModuleCommandDeclaration Module::builtinCommands[] = {
  { "dir", "dir", "hidden/h:modules/m:commands/c:instances/i:instancecounts/ic:recursive/r", "Lists all commands and modules under the current module depending on the parameters given; \
    'hidden' includes normally hidden commands, 'modules' includes any submodules, 'commands' includes commands, 'instances' shows data for each individual instance of a module, \
     'instancecounts' includes number of instances of each module, 'recursive' lists and child modules / commands ", false, false, &s_dir},
  { "help", "help", "recursive/r", "shows this brief explanation of all commands and modules, shows the help for all submodules as well if 'recursive'", false, false, nullptr },
  { "dumpdata", "dump", "recursive/r", "Dumps all data to be saved for the current module, will list the data of all submodules as well if 'recursive'", false, false, &s_dump },
};

int Module::getBuiltinCommandCount() {
    return sizeof(builtinCommands) / sizeof(builtinCommands[0]);
}

Module::Module() { }

eProcessResult Module::processBuiltInCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
//    debugPrintln("Processing built-in commands", debugPrintType::Debug);
    for (int i = 0; i < getBuiltinCommandCount(); i++) {
        auto command = getBuiltinCommand(i);

        String cmd = inCommandItem->hierarchy[inCommandItem->hierarchyIndex].name;
        //String cmd = inCommandItem->command;

        if ((getBuiltinCommand(i).longCommand == cmd) || (getBuiltinCommand(i).shortCommand == cmd)) {

            if (getBuiltinCommand(i).commandFunction == nullptr) {
                debugPrintln("Function " + getBuiltinCommand(i).longCommand + " not implemented", debugPrintType::Error);
                return eProcessResult::CommandFailed;
            }

            command.commandFunction(this, inCommandItem, commandResponses, request, command);
            return eProcessResult::Ok;
        }
    }
    return eProcessResult::NotFound;
}

eProcessResult Module::processModuleCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
//    debugPrintln("Processing module commands", debugPrintType::Debug);
    for (int i = 0; i < getModuleCommandCount(); i++) {
        auto command = getModuleCommand(i);

        String cmd = inCommandItem->hierarchy[inCommandItem->hierarchyIndex].name;
        //String cmd = inCommandItem->command;

        if ((getModuleCommand(i).longCommand == cmd) || (getModuleCommand(i).shortCommand == cmd)) {

            if (getModuleCommand(i).commandFunction == nullptr) {
                debugPrintln("Function " + getModuleCommand(i).longCommand + " not implemented", debugPrintType::Error);
                return eProcessResult::CommandFailed;
            }

            return command.commandFunction(this, inCommandItem, commandResponses, request, command);
        }
    }

    return eProcessResult::NotFound;
}

eProcessResult Module::processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    eProcessResult result;

    result = processBuiltInCommands(inCommandItem, commandResponses, request);
    if (result == eProcessResult::NotFound) {
        result = processModuleCommands(inCommandItem, commandResponses, request);
    }

    return result;
}

void Module::dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive) {
    //debugPrintln("Dir Module", debugPrintType::Debug);

    String lp = longPrefix, sp = shortPrefix;
    if (lp != "") { if (lp[lp.length() - 1] != '.') { lp +=  "."; } }
    if (sp != "") { if (sp[sp.length() - 1] != '.') { sp +=  "."; } }

    String response;

    for (int i = 0; i < getModuleCommandCount(); i++) {
        const ModuleCommandDeclaration mcd = getModuleCommand(i);
        if (mcd.longCommand == "") {
            inCommandResponses->push_back({ "dir: " + lp.substring(0, lp.length() - 1) + "[selection] : " + sp.substring(0, sp.length() - 1) + "[selection]", debugPrintType::InfoRequest });
        } else {
            if ((commands) && (!(mcd.hidden && (!hidden)))) {
                response = "dir: " + lp + mcd.longCommand + " : " + sp + mcd.shortCommand;
                if (mcd.commandFunction == nullptr) {
                    response += " : (not implemented)";
                }
                inCommandResponses->push_back({ response, debugPrintType::InfoRequest });
            } else {
                //debugPrintln("no because commands is " + String(commands) + " and this commands hidden is " + String(mcd.hidden) + " and show hidden is " + String(hidden), debugPrintType::Debug);
            }
        }
    }
}
/*
void Module::setDataChangedCallback(Module *inOwner, void (*inCallback)(Module *owner, Module *module)) {
    if ((owner != nullptr) && (owner != inOwner)) {
        debugPrintln("Object can only have on owner!", debugPrintType::Error);
        return;
    }
    debugPrintln("Callback set", debugPrintType::Debug);
    owner = inOwner;
    dataChangedCallback = inCallback;
}
*/
void Module::update() {
    debugPrintln("Do not use me either.", debugPrintType::Debug);
}

int Module::getModuleCommandCount() {
    debugPrintln("Do not use me.", debugPrintType::Debug);
    return -1;
}

bool Module::hasDataChange() {
    if (pDataChange) {
        pDataChange = false;
        return true;
    }
    return false;
}


void Module::dumpData(std::vector<commandResponse> *dataDump) {
    for (int i = 0; i < getModuleCommandCount(); i++) {
        ModuleCommandDeclaration mcd = getModuleCommand(i);
        if (mcd.save) {
            commandItem temp = commandItem(mcd.shortCommand);
            processCommands(&temp, dataDump, true);
        }
    }
//    return dataDump;
}

#endif
