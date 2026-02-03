#ifndef MODULEGROUP_CPP
#define MODULEGROUP_CPP

#include "../src/modulegroup.cpp"
#include "../src/module.cpp"
/*
ModuleGroup::ModuleGroup(Module *inModule)
{
    moduleID = new ModuleID(inModule->moduleID);
    addModule(inModule);
}*/
/*
ModuleGroup::ModuleGroup(ModuleID inModuleID) {
//    moduleID = new ModuleID(inModuleID);
}
*/
ModuleGroup::ModuleGroup(tModuleID inModuleID) {
    //tmoduleID = &inModuleID;
    tmoduleID.description = inModuleID.description;
    tmoduleID.longName = inModuleID.longName;
    tmoduleID.shortName = inModuleID.shortName;
    tmoduleID.isGroupHandler = inModuleID.isGroupHandler;
    tmoduleID.shortAlias = inModuleID.shortAlias;
}

ModuleGroup::~ModuleGroup() { }

void ModuleGroup::dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool inModules, bool commands, bool instances, bool instanceCount, bool recursive) {
    if (modules.size() == 0) {
//        debugPrintln("Dir empty module group", debugPrintType::Debug);
        if (inModules) {
//            inCommandResponses->push_back({ "dir: " + longPrefix + "[0] : " + shortPrefix + "[0] : " + String(modules.size()) + " : " + moduleID->getModuleTypeS(), debugPrintType::InfoRequest });
            inCommandResponses->push_back({ "ls: " + longPrefix + "[0] : " + shortPrefix + "[0] : " + String(modules.size()) + " : " + getModuleType(tmoduleID.moduleType), debugPrintType::InfoRequest });
        }
        return;
    }

    if (recursive) {
        if (!instances) {
            if (inModules) {
//                debugPrintln("Dir module group " + String(tmoduleID.longName) + " commands", debugPrintType::Debug);
//                inCommandResponses->push_back({ "dir: " + longPrefix + "[0] : " + shortPrefix + "[0] : " + String(modules.size()) + " : " + moduleID->getModuleTypeS(), debugPrintType::InfoRequest });
                inCommandResponses->push_back({ "ls: " + longPrefix + "[0] : " + shortPrefix + "[0] : " + String(modules.size()) + " : " + getModuleType(tmoduleID.moduleType), debugPrintType::InfoRequest });
            }
            modules[0]->dir(inCommandResponses, longPrefix, shortPrefix, hidden, inModules, commands, instances, instanceCount, recursive);
        } else {
//            debugPrintln("Dir module group instances", debugPrintType::Debug);
            for (int i = 0; i < modules.size(); i++) {
                String index = "[" + String (i) + "]";
                if (inModules) {
//                    inCommandResponses->push_back({ "dir: " + longPrefix + index + " : " + shortPrefix + index + " : " + String(modules.size()) + " : " + moduleID->getModuleTypeS(), debugPrintType::InfoRequest });
                    inCommandResponses->push_back({ "ls: " + longPrefix + index + " : " + shortPrefix + index + " : " + String(modules.size()) + " : " + getModuleType(tmoduleID.moduleType), debugPrintType::InfoRequest });
                }
                modules[i]->dir(inCommandResponses, longPrefix + index, shortPrefix + index, hidden, inModules, commands, instances, instanceCount, recursive);
            }
        }

    }
}

void ModuleGroup::help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix) {
    if (modules.size() == 0) {
        debugPrintln("Help: empty module group", debugPrintType::Debug);
//        inCommandResponses->push_back({ "ls: " + longPrefix + "[0] : " + shortPrefix + "[0] : " + String(modules.size()) + " : " + getModuleType(tmoduleID.moduleType), debugPrintType::InfoRequest });
        return;
    }

//            debugPrintln("Dir module group instances", debugPrintType::Debug);
//        if (inModules) {
//            inCommandResponses->push_back({ "ls: " + longPrefix + index + " : " + shortPrefix + index + " : " + String(modules.size()) + " : " + getModuleType(tmoduleID.moduleType), debugPrintType::InfoRequest });
//        }
     modules[0]->help(inCommandResponses, longPrefix, shortPrefix);
}

void ModuleGroup::dumpData(std::vector<commandResponse> *inCommandResponses) {
    std::vector<commandResponse> outDataDump;
    String out;

    for (int i = 0; i < modules.size(); i++) {
        modules[i]->dumpData(&outDataDump);
        for (int j = 0; j < outDataDump.size(); j++) {
//            out = modules[i]->moduleID->getShortAlias();
            out = String(modules[i]->getModuleID().shortAlias.c_str());
            if (modules.size() > 1) { out += "[" + String(i) + "]"; }
            out += "." + outDataDump[j].response;
            inCommandResponses->push_back({ out, outDataDump[j].responseType });
        }
        outDataDump.clear();
    }
}

eProcessResult ModuleGroup::processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    // Use any incoming selection
    SELECTION localSel;
    localSel = inCommandItem->hierarchy[inCommandItem->hierarchyIndex].selection;
    if (localSel.size() == 0) {
        // If there's no incoming selection, use the local selection if any, otherwise default to item 0
        if (selection.size() > 0) {
            localSel = selection;
        } else {
//            debugPrintln("Selection empty, adding [0]", debugPrintType::Debug);
            localSel.push_back(0);

        }
    }

    // After we have gotten the selected objects from the commandItem, advance to the next command item; the one we are performing on the object(s)
    inCommandItem->hierarchyIndex += 1;

    std::vector<commandResponse> childReturn;
    String returnString;
    uint8_t item;
//    debugPrintln("Group has " + String(modules.size()) + " children and we have " + String(localSel.size()) + " selections", debugPrintType::Debug);
    for (int i = 0; i < localSel.size(); i++) {
//        debugPrintln("Checking selection " + String(i), debugPrintType::Debug);
        childReturn.clear();
        item = localSel[i];
        if (item < modules.size()) {
//            debugPrintln("In module " + String(item), debugPrintType::Debug);
            eProcessResult result = modules[item]->processCommands(inCommandItem, &childReturn, request);

            //returnString = moduleID->getShortAlias();
            returnString = tmoduleID.shortAlias.c_str();
            //if (modules.size() > 1) {
                    returnString += "[" + String(localSel[i]) + "]";
            //}
            returnString += ".";
            for (int j = 0; j < childReturn.size(); j++) {
                commandResponses->push_back({ returnString + childReturn[j].response, childReturn[j].responseType });
            }
            if (result != eProcessResult::Ok) {
                return result;
            }
        } else {
//            debugPrintln("Index " + String(item) + " out of bounds for module group " + moduleID->getLongName() + " (size is " + String(modules.size()) + ")", debugPrintType::Error);
            debugPrintln("Index " + String(item) + " out of bounds for module group " + tmoduleID.longName + " (size is " + String(modules.size()) + ")", debugPrintType::Error);
        }
    }
    return eProcessResult::Ok;
}

bool ModuleGroup::indexing(commandItem *inCommandItem) {
    if ((inCommandItem->hierarchy[inCommandItem->hierarchyIndex].selection.size() > 1) && (singleSelection)) {
        debugPrintln("Group cannot have more than a single selection", debugPrintType::Error);
        return false;
    }
    selection.clear();
    for (int i = 0; i < inCommandItem->hierarchy[inCommandItem->hierarchyIndex].selection.size(); i++) {
        uint8_t j = inCommandItem->hierarchy[inCommandItem->hierarchyIndex].selection[i];
        if ((j < 0) || (j >= modules.size())) {
            debugPrintln("Selection out of range", debugPrintType::Error);
            if (selection.size() < 1) {
                selection.push_back(0);
            }
            return false;
        }
        selection.push_back(j);
    }
    if (indexCallback != nullptr) {
        return indexCallback(owner, this);
    }
    return false;
}

std::vector<Module*> ModuleGroup::getSelection() {
    std::vector<Module*> toReturn;
    for (int i = 0; i < selection.size(); i++) {
        toReturn.push_back(modules[selection[i]]);
    }
    return toReturn;
}

Module* ModuleGroup::getSingleSelection() {
    if (selection.size() < 1) {
        debugPrintln("No selection avaliable", debugPrintType::Error);
        return nullptr;
    } else {
        return modules[selection[0]];
    }
}

String ModuleGroup::getSelectionLiteral() {
    String returnString = "[";
    for (int i = 0; i < selection.size(); i++) {
        if (i > 0) { returnString += ", "; }
        returnString += String(selection[i]);
    }
    returnString += "]";
    return returnString;
}

bool ModuleGroup::setSelection(std::vector<int> inSelection) {
    if ((singleSelection) && (inSelection.size() > 1)) {
        debugPrintln("Group is single selection", debugPrintType::Error);
        return false;
    }
    if ((mustHaveSelection) && (inSelection.size() == 0)) {
        debugPrintln("Group must have selection", debugPrintType::Error);
        return false;
    }

    selection.clear();
    if (modules.size() == 0) {
        debugPrintln("Group empty, selection cleared", debugPrintType::Error);
        return false;
    }

    for (int i = 0; i < inSelection.size(); i++) {
        if ((inSelection[i] >= 0) && (inSelection[i] < modules.size())) {
            selection.push_back(inSelection[i]);
        } else {
            debugPrintln("Error in selection", debugPrintType::Error);
        }
    }

    if (indexCallback != nullptr) {
        return indexCallback(owner, this);
    }

    return true;
}

bool ModuleGroup::setSelection(int inSelection) {
    std::vector<int> sel;
    sel.push_back(inSelection);
    return setSelection(sel);
}

Module* ModuleGroup::addModule(Module *inModule) {
    modules.push_back(inModule);
    //debugPrintln("Adding module to group " + moduleID->getLongName() + ", new count is " + String(modules.size()), debugPrintType::Debug);

    //Module *b = modules.back();
    //debugPrintln("address of input module: " + String(reinterpret_cast<uintptr_t> (inModule), HEX) + " address of std::vector item: "  + String(reinterpret_cast<uintptr_t> (b), HEX), debugPrintType::Debug);

    return modules.back();
}

bool ModuleGroup::removeModule(int index) {
    if (index >= modules.size()) { return false; }

    if ((modules.size() == 1) && (mustHaveSelection)) {
        debugPrintln("Cannot remove the last module", debugPrintType::Error);
        return false;
    }

    modules.erase(modules.begin() + index);
    int localIndex;
    for (int i = 0; i < selection.size(); i++) {
        localIndex = selection[i];
        if (localIndex == index) {
//            debugPrintln("Selection " + String(i) + " which was " + String(localIndex) + " is being removed as it selected the deleted item", debugPrintType::Debug);
            selection.erase(selection.begin());
            i++;
        } else
        if (localIndex > index) {
            localIndex--;
            if (localIndex < 0) {
//                debugPrintln("Selection " + String(i) + " which was " + String(localIndex + 1) + " is being removed as it is below 0", debugPrintType::Debug);
                selection.erase(selection.begin());
                i++;
            } else {
                selection[i] = localIndex;
//                debugPrintln("Selection " + String(i) + " which was " + String(localIndex + 1) + " is being changed to " + localIndex, debugPrintType::Debug);
            }
        } else {
//            debugPrintln("Selection " + String(i) + " which was " + String(localIndex) + " is untouched", debugPrintType::Debug);
        }
    }

    if ((selection.size() == 0) && (mustHaveSelection)) {
//        debugPrintln("Must have selection, setting it to 0", debugPrintType::Debug);
        selection.push_back(0);
    }

    return true;
}

void ModuleGroup::setIndexCallback(Module *inOwner, void (*inCallback)(Module *owner, ModuleGroup *moduleGroup)) {
    if ((owner != nullptr) && (owner != inOwner)) {
        debugPrintln("Object can only have on owner!", debugPrintType::Error);
        return;
    }
    debugPrintln("Callback set", debugPrintType::Debug);
    owner = inOwner;
    indexCallback = inCallback;
}


#endif
