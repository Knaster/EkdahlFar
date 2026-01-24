#ifndef BASEMODULE_CPP
#define BASEMODULE_CPP

#include "basemodule.hpp"

const ModuleCommandDeclaration BaseModule::moduleCommands[] = {
    { "requestinfo", "rqi", "command", "Retrives rather than sets data associated with a command, if applicable", false, false, &s_requestInfo  },
    { "debugprint", "dp", "command|usb|hardware|undefined|priority|error|inforequest|expressionparser|debug:1|0", "Turns on or off serial feedback for the given item", false, true, &s_debugPrint},
    { "version", "ver", "-", "Gets the current firmware version", false, true, &s_version},
    { "saveallparameters", "sap", "-", "Saves all avaliable parameters", false, false, &s_saveAllParameters},
    { "loadallparameters", "lap", "-", "Loads all avaliable parameters", false, false, &s_loadAllParameters},
    { "resetallparameters", "rap", "-", "Resets all saved parameters", false, false, &s_resetAllParameters},
    { "uservariable", "uv", "variable(0-9):value", "Set user variable 0-9 to value", false, false, &s_userVariables},
    { "expressionparserevaluate", "epev", "expression", "Evaluates an arithmetric expression and sends back the output", false, false, &s_expressionParserEvaluate},
    { "ifequal", "ife", "variable:comparator:truecommandstring:elsecommandstring", "Performs 'IF [variable] EQUALS [comparator]' and adds [truecommandstring] to the \
        que if TRUE, otherwise adds [elsecommandstring]", false, false, &s_ifEqual},
    { "ifgreater", "ifg", "variable:comparator:truecommandstring:elsecommandstring", "Performs 'IF [variable] > [comparator]' and adds [truecommandstring] to the \
        que if TRUE, otherwise adds [elsecommandstring]", false, false, &s_ifGreater},
    { "ifless", "ifl", "variable:comparator:truecommandstring:elsecommandstring", "Performs 'IF [variable] < [comparator]' and adds [truecommandstring] to the \
        que if TRUE, otherwise adds [elsecommandstring]", false, false, &s_ifLess},
    { "reset", "rst", "0|1", "Resets the Ekdahl FAR, conditional", false, false, &s_reset},
    { "nick", "nick", "string", "Sets the nickname of this unit" , false, true, &s_nick},
    { "nooperation", "nop", "-", "Do absolutely, positively, nothing", false, false, &s_noOperation },
    { "freeram", "free", "-", "Shows free RAM memory", false, false, &s_freeRAM }
};

getModuleCount(BaseModule)

BaseModule::BaseModule(Module *inModule)
{
    if (inModule != nullptr) {
        mainModule = inModule;
        moduleID = inModule->moduleID;
    } else {
        moduleID = new ModuleID("unknown", "unknown", "unknown", 0);
    }
}

CREATE_MODULE_COMMAND_FUNCTION(debugPrint, BaseModule) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        bool found = false;
        for (int i=0; i<debugPrintTypes; i++ ) {
            if (debugPrintTypeName[i] == inCommandItem->argument[0]) {
                debugPrintEnabled[i] = inCommandItem->argument[1].toInt();
                found = true;
                break;
            }
        }
        if (!found) { return eProcessResult::WrongArgumentValue; }
    }

    for (int i=0; i<debugPrintTypes; i++ ) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + debugPrintTypeName[i] + ":" + debugPrintEnabled[i], debugPrintType::InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(version, BaseModule) {
    if (request) {
        inCommandResponses->push_back({thisItem.shortCommand + ":" + currentFirmwareVersion, InfoRequest});
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if ((currentFirmwareVersion != inCommandItem->argument[0])) {
            firmwareChanged = true;
            inCommandResponses->push_back({"Firmware version changed! Previous version: " + inCommandItem->argument[0], InfoRequest});
        }
        inCommandResponses->push_back({thisItem.shortCommand + ":" + currentFirmwareVersion, InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(saveAllParameters, BaseModule) {
    std::vector<commandResponse> inCommandResponse;
    dumpData(&inCommandResponse);
    String saveData = ""; //dumpData();
    for (int i = 0; i < inCommandResponse.size(); i++) {
        if (saveData != "") { saveData += ","; }
        saveData += inCommandResponse[i].response;
    }
    debugPrintln("Saving\n" + saveData + "\n" + "Total: " + String(EEPROMSaveString(&saveData, 0)) + " bytes", Command);
    inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(loadAllParameters, BaseModule) {
    loadAllParameters();
    inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    debugPrintln("Returning to command handler", debugPrintType::Debug);
    return eProcessResult::Ok;
};

void BaseModule::loadAllParameters() {
    String loadData; //  = new String();
    uint32_t datal = EEPROMLoadString(&loadData, 0);
    debugPrintln(loadData + "\nLoaded " + String(datal) + " bytes of data", Command);
    globalResponseCommands.addCommands(loadData);
    debugPrintln("Commands added", debugPrintType::Debug);
}

CREATE_MODULE_COMMAND_FUNCTION(resetAllParameters, BaseModule) {
    String saveData = "ver:" + currentFirmwareVersion + ","; // "ver:1,";
    debugPrintln("Saving\n" + saveData + "\n" + "Total: " + String(EEPROMSaveString(&saveData, 0)) + " bytes", Command);
    inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(userVariables, BaseModule) {
    if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
    if (!validateNumber(inCommandItem->argument[0].toInt(), 0, userVariableMax)) { return eProcessResult::WrongArgumentValue; }

    int userVariable = inCommandItem->argument[0].toInt();
    if (!request) {
//        expressionParser.duv[userVariable] = inCommandItem->argument[1].toFloat();
    }
//    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(userVariable) + ":" + String(expressionParser.duv[userVariable]), InfoRequest});
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(expressionParserEvaluate, BaseModule) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    commandList testCommands(thisItem.shortCommand + ":" + inCommandItem->argument[0]);
//    testCommands.parseCommandExpressions(expressionParser);
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(testCommands.item[0].argument[0]), debugPrintType::InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(ifEqual, BaseModule) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 4)) { return eProcessResult::WrongArgumentCount; }
        commandList *executeNow;
        if (inCommandItem->argument[0].toInt() == inCommandItem->argument[1].toInt()) {
            executeNow = new commandList(inCommandItem->argument[2]);
        } else {
            executeNow = new commandList(inCommandItem->argument[3]);
        }
        processCommandList(this, executeNow, inCommandResponses);
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(ifGreater, BaseModule) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 4)) { return eProcessResult::WrongArgumentCount; }
        commandList *executeNow;
        if (inCommandItem->argument[0].toInt() > inCommandItem->argument[1].toInt()) {
            executeNow = new commandList(inCommandItem->argument[2]);
        } else {
            executeNow = new commandList(inCommandItem->argument[3]);
        }
        processCommandList(this, executeNow, inCommandResponses);
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(ifLess, BaseModule) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 4)) { return eProcessResult::WrongArgumentCount; }
        commandList *executeNow;
        if (inCommandItem->argument[0].toInt() < inCommandItem->argument[1].toInt()) {
            executeNow = new commandList(inCommandItem->argument[2]);
        } else {
            executeNow = new commandList(inCommandItem->argument[3]);
        }
        processCommandList(this, executeNow, inCommandResponses);
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(reset, BaseModule) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() == 1) {
            reset();
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(nick, BaseModule) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + nickName, debugPrintType::InfoRequest});
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        nickName = String(inCommandItem->argument[0]).trim();
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + nickName, debugPrintType::InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(noOperation, BaseModule) {
    inCommandResponses->push_back({ thisItem.shortCommand + ":", InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(dump, BaseModule) {
    /*std::vector<commandResponse> returnResponses = Module::dumpData();
    for (int i = 0; i < returnResponses.size(); i++) {
        inCommandResponses->push_back(returnResponses[i]);
    }

    returnResponses = mainModule->dumpData();
    for (int i = 0; i < returnResponses.size(); i++) {
        inCommandResponses->push_back(returnResponses[i]);
    }*/
    Module::dumpData(inCommandResponses);
    if (mainModule != nullptr) {
        mainModule->dumpData(inCommandResponses);
    }

    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(requestInfo, BaseModule) {
    if (checkArgumentsMin(inCommandItem, inCommandResponses, 1)) {
        String commandString = inCommandItem->argument[0].trim();
        for (int i = 0; i < inCommandItem->argument.size() - 1; i++) {
            commandString += ":" + inCommandItem->argument[i + 1];
        }
//        debugPrintln("New command string is " + commandString, debugPrintType::Debug);
        //commandItem *tCommandItem = new commandItem(commandString);
        commandItem tCommandItem(commandString);
        return processCommands(&tCommandItem, inCommandResponses, true);
    }
    return eProcessResult::WrongArgumentCount;
}

CREATE_MODULE_COMMAND_FUNCTION(freeRAM, BaseModule) {
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(freeram()), InfoRequest });
    return eProcessResult::Ok;
}

eProcessResult BaseModule::processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    eProcessResult processResult;

    //debugPrintln("Before processing we have " + String(inCommandItem->argument.size()) + " arguments", debugPrintType::Debug);

    processResult = Module::processCommands(inCommandItem, commandResponses, request);
    //debugPrintln("After module processing we have " + String(inCommandItem->argument.size()) + " arguments", debugPrintType::Debug);
    if (processResult == eProcessResult::NotFound) {
//        debugPrintln("Main?", debugPrintType::Debug);
        if (mainModule != nullptr) {
//            debugPrintln("Main.", debugPrintType::Debug);
            processResult = mainModule->processCommands(inCommandItem, commandResponses, request);
        }
    }
    //debugPrintln("All commands processed, adding " + String(inCommandItem->argument.size()) + " response arguments", debugPrintType::Debug);
    String response = inCommandItem->command;
    for (int i = 0; i < inCommandItem->argument.size(); i++) {
        response += ":" + inCommandItem->argument[i];
    }
    //debugPrintln("All response arguments added", debugPrintType::Debug);

    switch (processResult) {
    case eProcessResult::NotFound:
        commandResponses->push_back({"Command not found: " + response, debugPrintType::Error});
        break;
    case eProcessResult::CommandFailed:
        commandResponses->push_back({"Command failed: " + response, debugPrintType::Error});
        break;
    case eProcessResult::WrongArgumentCount:
    case eProcessResult::WrongArgumentMinimum:
        commandResponses->push_back({"Wrong command arguments: " + response, debugPrintType::Error});
        break;
    case eProcessResult::WrongArgumentValue:
        commandResponses->push_back({"Wrong command argument values: " + response, debugPrintType::Error});
        break;
    case eProcessResult::Ok:
        //debugPrintln("Ok", debugPrintType::TextInfo);
        break;
    case eProcessResult::PassThrough:
        //debugPrintln("Ok, pass-through", debugPrintType::TextInfo);
        break;
    default:
        debugPrintln("Unknown response " + String(processResult), debugPrintType::Error);
    };

    //debugPrintln("Returning result", debugPrintType::Debug);

    return processResult;
}

void BaseModule::dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool inModules, bool commands, bool instances, bool instanceCount, bool recursive) {
    //debugPrintln("Base dir", debugPrintType::Debug);
    Module::dir(inCommandResponses, longPrefix, shortPrefix, hidden, inModules, commands, instances, instanceCount, recursive);
    mainModule->dir(inCommandResponses, longPrefix, shortPrefix, hidden, inModules, commands, instances, instanceCount, recursive);
}

void BaseModule::dumpData(std::vector<commandResponse> *dataDump) {
/*    std::vector<commandResponse> inCommandResponse;
    inCommandResponse = Module::dumpData();
    std::vector<commandResponse> inCommandResponse2;
    inCommandResponse2 = mainModule->dumpData();
    inCommandResponse.insert(inCommandResponse.end(), inCommandResponse2.begin(), inCommandResponse2.end());*/
    Module::dumpData(dataDump);
    if (mainModule != nullptr) {
        mainModule->dumpData(dataDump);
    }
//    return inCommandResponse;
}

void BaseModule::reset() {
    SCB_AIRCR = 0x05FA0004;
    asm volatile ("dsb");
}

void BaseModule::update() {
    if (mainModule != nullptr) {
        mainModule->update();
    }
}

#endif // BASEMODULE_CPP
