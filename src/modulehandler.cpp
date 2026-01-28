#ifndef MODULEHANDLER_CPP
#define MODULEHANDLER_CPP

#include "modulehandler.hpp"
#include "../src/module.cpp"
#include "../src/modulegroup.cpp"

ModuleHandler::ModuleHandler() {
    isGroupHandler = true;
}

void ModuleHandler::dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive) {
    String lp = longPrefix, sp = shortPrefix;

    if (lp != "") { if (lp[lp.length() - 1] != '.') { lp +=  "."; } }
    if (sp != "") { if (sp[sp.length() - 1] != '.') { sp +=  "."; } }

    if (commands) {
        Module::dir(inCommandResponses, lp, sp, hidden, modules, commands, instances, instanceCount, recursive);
    }

    for (int i = 0; i < moduleGroups.size(); i++) {
        moduleGroups[i].dir(inCommandResponses,lp + moduleGroups[i].moduleID->getLongName(), sp + moduleGroups[i].moduleID->getShortName(), hidden, modules, commands, instances, instanceCount, recursive);
    }
}

void ModuleHandler::dumpData(std::vector<commandResponse> *inCommandResponses) {

    for (int i = 0; i < moduleGroups.size() + 1; i++) {
        if (i < moduleGroups.size()) {
            inCommandResponses->push_back({ "ins:" + moduleGroups[i].moduleID->getShortName() + ":" + String(moduleGroups[i].modules.size()), debugPrintType::InfoRequest });
        }

        if (i == 0) {
            Module::dumpData(inCommandResponses);
        } else {
            moduleGroups[i - 1].dumpData(inCommandResponses);
        }
    }
}

ModuleGroup* ModuleHandler::getGroup(String longName) {
    for (int i = 0; i < moduleGroups.size(); i++) {
        if (moduleGroups[i].moduleID->getLongName() == longName) {
            return &moduleGroups[i];
        }
    }
    return nullptr;
}

ModuleGroup* ModuleHandler::addGroup(ModuleID inModuleID) {
    ModuleGroup *group = getGroup(inModuleID.getLongName());
    if (group != nullptr) {
        debugPrintln("Group already exists!", debugPrintType::Error);
        return group;
    }
    moduleGroups.push_back(ModuleGroup(inModuleID));
    //debugPrintln("Added a group, for a total of " + String(moduleGroups.size()) + " groups", debugPrintType::Debug);
    return &moduleGroups.back();
}

ModuleGroup* ModuleHandler::addModule(Module *module) {
    ModuleGroup *group = nullptr;
    group = getGroup(module->moduleID->getLongName());
    if (group == nullptr) {
        //debugPrintln("Adding group " + module->moduleID->getLongName(), debugPrintType::Debug);
        group = addGroup(module->moduleID);
    }
    group->addModule(module);
    return &moduleGroups.back();
}

eProcessResult ModuleHandler::processModuleHandlerCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponse, bool request) {
    if (inCommandItem->hierarchy[inCommandItem->hierarchyIndex].name == "ins") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponse, 2)) { return eProcessResult::WrongArgumentCount; }
            addInstances(inCommandItem->argument[0], inCommandItem->argument[1].toInt());
            commandResponse->push_back({ "ins:" + inCommandItem->argument[0] + ":1", debugPrintType::InfoRequest });
            return eProcessResult::Ok;
        };
        for (int i = 0; i < moduleGroups.size(); i++) {
            commandResponse->push_back({ "ins:" + moduleGroups[i].moduleID->getShortName() + ":" + String(moduleGroups[i].modules.size()), debugPrintType::InfoRequest });
        }
        return eProcessResult::Ok;
    }
    return eProcessResult::NotFound;
}

eProcessResult ModuleHandler::processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    eProcessResult result = processBuiltInCommands(inCommandItem, commandResponses, request);
    if (result != eProcessResult::NotFound) { return result; }
    result = processModuleHandlerCommands(inCommandItem, commandResponses, request);
    if (result != eProcessResult::NotFound) { return result; }
    // if this is not the last item in the hierarchy, check groups. If this is the last but it has a selection it is an indexing command in which case we will call command "" on the group
    if ((inCommandItem->hierarchyIndex < (inCommandItem->hierarchy.size() - 1)) || (inCommandItem->hierarchy[inCommandItem->hierarchyIndex].selection.size() > 0)) {
        for (int i = 0; i < moduleGroups.size(); i++) {
            // If this group has the same name as the current hierarchy, enter
            if ((inCommandItem->hierarchy[inCommandItem->hierarchyIndex].name == moduleGroups[i].moduleID->getLongName()) ||
                (inCommandItem->hierarchy[inCommandItem->hierarchyIndex].name == moduleGroups[i].moduleID->getShortAlias())) {
//                debugPrintln("Module group for " + moduleGroups[i].moduleID->getDescriptor() + " has prefix " + moduleGroups[i].moduleID->getLongName(), debugPrintType::Debug);
                // Check if this is an indexing command
                if (inCommandItem->hierarchyIndex == (inCommandItem->hierarchy.size() - 1)) {
//                    debugPrintln("Indexing command in module handler, selection is " + String(inCommandItem->selection.size()) , debugPrintType::Debug);
                    if (!request) {
                        if (!moduleGroups[i].indexing(inCommandItem)) {
                            return eProcessResult::CommandFailed;
                        }
                    }
                    String returnString = moduleGroups[i].moduleID->getShortAlias();
                    if (moduleGroups[i].selection.size() > 0) {
                        for (int j = 0; j < moduleGroups[i].selection.size(); j++) {
                            commandResponses->push_back({ returnString + "[" + String(moduleGroups[i].selection[j]) + "]", debugPrintType::InfoRequest });
                        }
                    } else {
                        commandResponses->push_back({ returnString + ":none", debugPrintType::InfoRequest });
                    }
                    return eProcessResult::Ok;
                } else {
                    return moduleGroups[i].processCommands(inCommandItem, commandResponses, request);
                }
            } else {
                //debugPrintln(inCommandItem->prefix + " is not found for " + moduleGroups[i].moduleID->getLongName(), debugPrintType::Debug);
//                debugPrintln(inCommandItem->hierarchy[inCommandItem->hierarchyIndex].name + " is not found for " + moduleGroups[i].moduleID->getLongName(), debugPrintType::Debug);
            }
        }
        debugPrintln("Module " + inCommandItem->hierarchy[inCommandItem->hierarchyIndex].name + " not found in " + moduleID->getLongName() , debugPrintType::Error);
    } else {
//        debugPrintln("Last entry, checking commands", debugPrintType::Debug);
        return Module::processCommands(inCommandItem, commandResponses, request);
    }
    return eProcessResult::NotFound;
}
#endif
