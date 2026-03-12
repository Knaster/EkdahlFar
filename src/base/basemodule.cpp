#ifndef BASEMODULE_CPP
#define BASEMODULE_CPP

#include "base/basemodule.hpp"

const ModuleCommandDeclaration BaseModule::moduleCommands[] = {
    { "version", "ver", "-", "Gets the current firmware version", false, true, &s_version, eCommandType_data::ectData | eCommandType_access::ectRequest},
    { "debugprint", "dp", "command|usb|hardware|undefined|priority|error|inforequest|expressionparser|debug:1|0", "Turns on or off serial feedback for the given item", false, true,
        &s_debugPrint, eCommandType_data::ectData | eCommandType_function::ectSystem },
    { "requestinfo", "rqi", "command", "Retrives rather than sets data associated with a command, if applicable", false, false, &s_requestInfo,
        eCommandType_data::ectCommands | eCommandType_function::ectSystem },
    { "saveallparameters", "sap", "-", "Saves all avaliable parameters", false, false, &s_saveAllParameters, eCommandType_data::ectImmediate | eCommandType_function::ectSystem},
    { "loadallparameters", "lap", "-", "Loads all avaliable parameters", false, false, &s_loadAllParameters, eCommandType_data::ectImmediate | eCommandType_function::ectSystem},
    { "resetallparameters", "rap", "-", "Resets all saved parameters", false, false, &s_resetAllParameters, eCommandType_data::ectImmediate | eCommandType_function::ectSystem},
    { "reset", "rst", "0|1", "Resets the Ekdahl FAR, conditional", false, false, &s_reset, eCommandType_data::ectConditional | eCommandType_function::ectSystem},
    { "nick", "nick", "string", "Sets the nickname of this unit" , false, true, &s_nick, eCommandType_data::ectSimpleString | eCommandType_function::ectSetting},
    { "nooperation", "nop", "-", "Do absolutely, positively, nothing", false, false, &s_noOperation, eCommandType_data::ectImmediate | eCommandType_function::ectSystem }
};

getModuleCount(BaseModule)

BaseModule::BaseModule(Module *inModule)
{
    if (inModule != nullptr) {
        mainModule = inModule;
        tmoduleID = &(inModule->getModuleID());
    } else {
        tmoduleID = &defaultModuleID;
        debugPrintln("Base tModuleID is NULL, errors are going to ensue", debugPrintType::Error);
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
    String ver = String(version());
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if ((ver != inCommandItem->argument[0])) {
            firmwareChanged = true;
            ver = inCommandItem->argument[0];
            inCommandResponses->push_back({"Firmware version changed! Previous version: " + inCommandItem->argument[0], InfoRequest});
        }
    }
    inCommandResponses->push_back({thisItem.shortCommand + ":"  + delimitExpression(String(mainModule->getModuleID().description), true) + ":" + String(mainModule->getModuleID().longName) + ":" +
                                  String(mainModule->getModuleID().shortName) + ":" + delimitExpression(ver, true), InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(saveAllParameters, BaseModule) {
    String save = saveAllParameters();
    if (save == "") {
        inCommandResponses->push_back({thisItem.shortCommand + ":0", InfoRequest});
        return eProcessResult::Ok;
    }
    debugPrintln(save, Command);
    inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(loadAllParameters, BaseModule) {
    loadAllParameters();
    inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    //debugPrintln("Returning to command handler", debugPrintType::Debug);
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(resetAllParameters, BaseModule) {
    resetAllParameters();
    inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
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
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + pNickName, debugPrintType::InfoRequest});
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pNickName = stripQuotes(String(inCommandItem->argument[0]));
        pNickName.trim();
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + pNickName, debugPrintType::InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(noOperation, BaseModule) {
    inCommandResponses->push_back({ thisItem.shortCommand + ":", InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(requestInfo, BaseModule) {
    if (checkArgumentsMin(inCommandItem, inCommandResponses, 1)) {
        String commandString = inCommandItem->argument[0];
        commandString.trim();
        for (int i = 0; i < inCommandItem->argument.size() - 1; i++) {
            commandString += ":" + inCommandItem->argument[i + 1];
        }
        CommandItem tCommandItem(&commandString);
        return processCommands(&tCommandItem, inCommandResponses, true);
    }
    return eProcessResult::WrongArgumentCount;
}

eProcessResult BaseModule::processCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    return processSelf(inCommandItem, commandResponses, request, eProcessResult::NotFound);
}

eProcessResult BaseModule::processSelf(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, eProcessResult processResult) {
    //debugPrintln("Start base module processing", debugPrintType::Debug);

    if (processResult == eProcessResult::NotFound) {
        //debugPrintln("Base module processing", debugPrintType::Debug);
        processResult = Module::processCommands(inCommandItem, commandResponses, request);
        //debugPrintln("Base module processing complete", debugPrintType::Debug);
        if (processResult == eProcessResult::NotFound) {
            if (mainModule != nullptr) {
                //debugPrintln("Main module processing", debugPrintType::Debug);
                processResult = mainModule->processCommands(inCommandItem, commandResponses, request);
            }
        }
    }

    //debugPrintln("Building response", debugPrintType::Debug);

    String response = inCommandItem->hierarchy[0].name;
    for (int i = 0; i < inCommandItem->argument.size(); i++) {
        response += ":" + inCommandItem->argument[i];
    }

    //debugPrintln("Processing", debugPrintType::Debug);

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

    //debugPrintln("Returning", debugPrintType::Debug);
    return processResult;
}

void BaseModule::dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool inModules, bool commands, bool instances, bool instanceCount, bool recursive) {
    //debugPrintln("Base dir", debugPrintType::Debug);
    Module::dir(inCommandResponses, longPrefix, shortPrefix, hidden, inModules, commands, instances, instanceCount, recursive);
    if (mainModule != nullptr) {
        mainModule->dir(inCommandResponses, longPrefix, shortPrefix, hidden, inModules, commands, instances, instanceCount, recursive);
    }
}

void BaseModule::dumpData(std::vector<commandResponse> *dataDump) {
//    debugPrintln("Built in command count: " + String(getBuiltinCommandCount()), debugPrintType::Debug);

    Module::dumpData(dataDump);
    if (mainModule != nullptr) {
        mainModule->dumpData(dataDump);
    }
}

void BaseModule::help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix) {
    //debugPrintln("Base help", debugPrintType::Debug);

    String response;
    for (int i = 0; i < getBuiltinCommandCount(); i++) {
        const ModuleCommandDeclaration mcd = builtinCommands[i];
        response = "help:" + mcd.longCommand + ":" + mcd.shortCommand + ":" + String(mcd.cType) + ":" + delimitExpression(mcd.arguments, true) + ":" + mcd.hidden + ":" +
            delimitExpression(mcd.Help, true);
        inCommandResponses->push_back({ response, debugPrintType::InfoRequest });
    }

    Module::help(inCommandResponses, longPrefix, shortPrefix);
    if (mainModule != nullptr) {
        mainModule->help(inCommandResponses, longPrefix, shortPrefix);
    }
}

void BaseModule::update() {
    if (mainModule != nullptr) {
        mainModule->update();
    }
}

bool BaseModule::convertRequestInfo(CommandItem *inCommandItem) {
    return true;
};

String* BaseModule::nick(bool request) {
    return &pNickName;
}

const char* BaseModule::version() {
    return reinterpret_cast< char const* >(&completeVersion);
}

void BaseModule::reset() {
    debugPrintln("No reset implemented", debugPrintType::Debug);
}

void BaseModule::bootLoader() {
    debugPrintln("No bootloader reset implemented", debugPrintType::Debug);
}

#endif // BASEMODULE_CPP
