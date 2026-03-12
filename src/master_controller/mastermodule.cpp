#ifndef MASTERMODULE_CPP
#define MASTERMODULE_CPP

#include "master_controller/mastermodule.hpp"
#include "teensy_specific/eepromhelpers.hpp"

const ModuleCommandDeclaration MasterModule::masterModuleCommands[] = {
    { "uservariable", "uv", "variable(0-9):value", "Set user variable 0-9 to value", false, false, &s_userVariables, eCommandType_data::ectData | eCommandType_function::ectParameter},
    { "expressionparserevaluate", "epev", "expression", "Evaluates an arithmetric expression and sends back the output", false, false, &s_expressionParserEvaluate,
        eCommandType_data::ectSimpleFloat | eCommandType_dataOptions::ectExpression | eCommandType_function::ectUndefined },
    { "ifequal", "ife", "variable:comparator:truecommandstring:elsecommandstring", "Performs [IF [variable] EQUALS [comparator]] and adds [truecommandstring] to the que if TRUE, otherwise adds [elsecommandstring]",
        false, false, &s_ifEqual, eCommandType_data::ectData | eCommandType_function::ectLogic },
    { "ifgreater", "ifg", "variable:comparator:truecommandstring:elsecommandstring", "Performs [IF [variable] > [comparator]] and adds [truecommandstring] to the que if TRUE, otherwise adds [elsecommandstring]",
        false, false, &s_ifGreater, eCommandType_data::ectData | eCommandType_function::ectLogic },
    { "ifless", "ifl", "variable:comparator:truecommandstring:elsecommandstring", "Performs [IF [variable] < [comparator]] and adds [truecommandstring] to the que if TRUE, otherwise adds [elsecommandstring]",
        false, false, &s_ifLess, eCommandType_data::ectData | eCommandType_function::ectLogic },
    { "freeram", "free", "-", "Shows free RAM memory", false, false, &s_freeRAM, eCommandType_data::ectData | eCommandType_function::ectSystem },
    { "external", "ext", "commandlist", "Directly sending commands to external units", false, false, s_external, eCommandType_data::ectCommands | eCommandType_function::ectSystem },
    { "test", "test", "-", "-", false, false, &s_test, eCommandType_data::ectData | eCommandType_function::ectSystem }
};

int MasterModule::getMasterModuleCommandCount() const {
    return sizeof(masterModuleCommands) / sizeof(masterModuleCommands[0]);
}

MasterModule::MasterModule(ModuleHandler *inModule, ExpressionParser *inExpressionParser):BaseModule(inModule) {
    if (inModule != nullptr) {
        expressionParser = inExpressionParser;
        pluginHandler = new PluginHandler(expressionParser);
        inModule->addModule(pluginHandler);

#ifndef NO_EXTERNAL_MODULES
        externalModuleHandler = new ExternalModuleHandler();
        externalModuleHandler->mainModule = inModule;
        externalModuleHandler->addSerialHardware(&Serial1);
#endif
    } else {
        debugPrintln("Base tModuleID is NULL, errors are going to ensue", debugPrintType::Error);
    }
}

CREATE_MODULE_COMMAND_FUNCTION(userVariables, MasterModule) {
    if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
    if (!validateNumber(inCommandItem->argument[0].toInt(), 0, userVariableMax)) { return eProcessResult::WrongArgumentValue; }

    int userVariable = inCommandItem->argument[0].toInt();
    if (!request) {
        expressionParser->duv[userVariable] = inCommandItem->argument[1].toFloat();
    }
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(userVariable) + ":" + String(expressionParser->duv[userVariable]), InfoRequest});
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(expressionParserEvaluate, MasterModule) {
    if (inCommandItem->argument.size() == 1) {
        String result = expressionParser->parseCommandExpressions(stripQuotes(inCommandItem->argument[0]));
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + result, debugPrintType::InfoRequest});
    } else
    if (inCommandItem->argument.size() == 2) {
        String result = expressionParser->parseCommandExpressions(stripQuotes(inCommandItem->argument[0]));
        result = inCommandItem->argument[1] + ":" + delimitExpression(result);
        globalResponseCommands.addCommands(result);
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(result), debugPrintType::InfoRequest});
    } else {
        return eProcessResult::WrongArgumentCount;
    }

    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(ifEqual, MasterModule) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 4)) { return eProcessResult::WrongArgumentCount; }
        CommandList *executeNow;
        if (inCommandItem->argument[0].toInt() == inCommandItem->argument[1].toInt()) {
            executeNow = new CommandList(inCommandItem->argument[2]);
        } else {
            executeNow = new CommandList(inCommandItem->argument[3]);
        }
        processCommandList(this, executeNow, inCommandResponses);
        delete executeNow;
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(ifGreater, MasterModule) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 4)) { return eProcessResult::WrongArgumentCount; }
        CommandList *executeNow;
        if (inCommandItem->argument[0].toInt() > inCommandItem->argument[1].toInt()) {
            executeNow = new CommandList(inCommandItem->argument[2]);
        } else {
            executeNow = new CommandList(inCommandItem->argument[3]);
        }
        processCommandList(this, executeNow, inCommandResponses);
        delete executeNow;
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(ifLess, MasterModule) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 4)) { return eProcessResult::WrongArgumentCount; }
        CommandList *executeNow;
        if (inCommandItem->argument[0].toInt() < inCommandItem->argument[1].toInt()) {
            executeNow = new CommandList(inCommandItem->argument[2]);
        } else {
            executeNow = new CommandList(inCommandItem->argument[3]);
        }
        processCommandList(this, executeNow, inCommandResponses);
        delete executeNow;
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(freeRAM, MasterModule) {
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(freeram()), InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(external, MasterModule) {
    //inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(freeram()), InfoRequest });
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
//    Serial1.println(stripQuotes(inCommandItem->argument[0]));
    return eProcessResult::Ok;
}

eProcessResult MasterModule::processMasterModuleCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    for (int i = 0; i < getMasterModuleCommandCount(); i++) {
        auto command = getMasterModuleCommand(i);
        String cmd = inCommandItem->hierarchy[inCommandItem->hierarchyIndex].name;
        if ((getMasterModuleCommand(i).longCommand == cmd) || (getMasterModuleCommand(i).shortCommand == cmd)) {
            if (getMasterModuleCommand(i).commandFunction == nullptr) {
                debugPrintln("Function " + getMasterModuleCommand(i).longCommand + " not implemented", debugPrintType::Error);
                return eProcessResult::CommandFailed;
            }
            return command.commandFunction(this, inCommandItem, commandResponses, request, command);
        }
    }
    return eProcessResult::NotFound;
}

eProcessResult MasterModule::processCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    eProcessResult processResult = eProcessResult::NotFound;

    //debugPrintln("Master module processing start", debugPrintType::Debug);
    processResult = processMasterModuleCommands(inCommandItem, commandResponses, request);
    //debugPrintln("Base module processing start", debugPrintType::Debug);
    return BaseModule::processSelf(inCommandItem, commandResponses, request, processResult);
}

void MasterModule::dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool inModules, bool commands, bool instances, bool instanceCount, bool recursive) {
    String response;
    for (int i = 0; i < getMasterModuleCommandCount(); i++) {
        if ((commands) && (!(masterModuleCommands[i].hidden && (!hidden)))) {
            response = "ls: " + masterModuleCommands[i].longCommand + " : " + masterModuleCommands[i].shortCommand;
            if (masterModuleCommands[i].commandFunction == nullptr) { response += " : (not implemented)"; }
            inCommandResponses->push_back({ response, debugPrintType::InfoRequest });
        }
    }
    BaseModule::dir(inCommandResponses, longPrefix, shortPrefix, hidden, inModules, commands, instances, instanceCount, recursive);
}

void MasterModule::dumpData(std::vector<commandResponse> *dataDump) {
    BaseModule::dumpData(dataDump);
}

void MasterModule::help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix) {
    String response;
    for (int i = 0; i < getMasterModuleCommandCount(); i++) {
        response = "help:" + masterModuleCommands[i].longCommand + ":" + masterModuleCommands[i].shortCommand + ":" + String(masterModuleCommands[i].cType) + ":" +
            delimitExpression(masterModuleCommands[i].arguments, true) + ":" + masterModuleCommands[i].hidden + ":" + delimitExpression(masterModuleCommands[i].Help, true);
        inCommandResponses->push_back({ response, debugPrintType::InfoRequest });
    }

    BaseModule::help(inCommandResponses, longPrefix, shortPrefix);
}

void MasterModule::loadAllParameters() {
    String loadData; //  = new String();
    uint32_t datal = EEPROMLoadString(&loadData, 0);
    debugPrintln(loadData + "\nLoaded " + String(datal) + " bytes of data", Command);
    globalResponseCommands.addCommands(loadData);
    debugPrintln("Commands added", debugPrintType::Debug);
};

String MasterModule::saveAllParameters() {
    std::vector<commandResponse> inCommandResponse;
    dumpData(&inCommandResponse);
    String saveData = ""; //dumpData();
    for (int i = 0; i < inCommandResponse.size(); i++) {
        if (saveData != "") { saveData += ","; }
        saveData += inCommandResponse[i].response;
    }
    return "Saving\n" + saveData + "\n" + "Total: " + String(EEPROMSaveString(&saveData, 0)) + " bytes";
};

void MasterModule::resetAllParameters() {
    String ver = *version();
    String saveData = "ver:" + ver + ","; // "ver:1,";
    debugPrintln("Saving\n" + saveData + "\n" + "Total: " + String(EEPROMSaveString(&saveData, 0)) + " bytes", Command);
};

void MasterModule::reset() {
    SCB_AIRCR = 0x05FA0004;
    asm volatile ("dsb");
};

void MasterModule::update() {
    BaseModule::update();
    pluginHandler->update();
#ifndef NO_EXTERNAL_MODULES
    externalModuleHandler->update();
#endif
}

CREATE_MODULE_COMMAND_FUNCTION(test, MasterModule) {
    init();
    return eProcessResult::Ok;
}

void MasterModule::init() {
#ifndef NO_EXTERNAL_MODULES
    externalModuleHandler->scanForModules();
#endif
};

#endif
