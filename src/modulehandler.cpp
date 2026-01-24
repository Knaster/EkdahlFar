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
//        debugPrintln("Dir Groups: " + String(moduleGroups[i].moduleID->getLongName()), debugPrintType::Debug);
        moduleGroups[i].dir(inCommandResponses,lp + moduleGroups[i].moduleID->getLongName(), sp + moduleGroups[i].moduleID->getShortName(), hidden, modules, commands, instances, instanceCount, recursive);
    }
}

//std::vector<commandResponse> ModuleHandler::dumpData() {
void ModuleHandler::dumpData(std::vector<commandResponse> *inCommandResponses) {
//    std::vector<commandResponse> dataDump;
//    std::vector<commandResponse> outDataDump;

    for (int i = 0; i < moduleGroups.size() + 1; i++) {
        if (i == 0) {
            //outDataDump = Module::dumpData();
            Module::dumpData(inCommandResponses);
        } else {
            //outDataDump = moduleGroups[i - 1].dumpData();
            moduleGroups[i - 1].dumpData(inCommandResponses);
        }
/*        for (int j = 0; j < outDataDump.size(); j++) {
            dataDump.push_back(outDataDump[j]);
        }*/
    }
    //return dataDump;
}

ModuleGroup* ModuleHandler::getGroup(String longName) {
//    debugPrintln("We have " + String(moduleGroups.size()) + " groups", debugPrintType::Debug);

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
    //group = new ;
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

eProcessResult ModuleHandler::processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    eProcessResult result = processBuiltInCommands(inCommandItem, commandResponses, request);
    if (result != eProcessResult::NotFound) {
        return result;
    }

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
                        for (int j = 0; j < inCommandItem->hierarchy[inCommandItem->hierarchyIndex].selection.size(); j++) {
                            commandResponses->push_back({ returnString + "[" + String(inCommandItem->hierarchy[inCommandItem->hierarchyIndex].selection[j]) + "]", debugPrintType::InfoRequest });
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
