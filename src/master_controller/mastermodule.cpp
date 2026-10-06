#ifndef MASTERMODULE_CPP
#define MASTERMODULE_CPP

#include "master_controller/mastermodule.hpp"
#include "teensy_specific/eepromhelpers.hpp"

const ModuleCommandDeclaration MasterModule::masterModuleCommands[] = {
    { "uservariable", "uv", "variable(0-9):value", "Set user variable 0-9 to value", false, false, &s_userVariables, eCommandType_data::ectData | eCommandType_function::ectAdminSetting},
    { "expressionparserevaluate", "epev", "expression", "Evaluates an arithmetric expression and sends back the output", false, false, &s_expressionParserEvaluate,
        eCommandType_data::ectSimpleString | eCommandType_dataOptions::ectExpression | eCommandType_access::ectInvokeOnly | eCommandType_function::ectAdminAction },
    { "ifequal", "ife", "variable:comparator:truecommandstring:elsecommandstring",
        "Performs [IF [variable] EQUALS [comparator]] and adds [truecommandstring] to the que if TRUE, otherwise adds [elsecommandstring]", false, false, &s_ifEqual,
        eCommandType_data::ectSimpleString | eCommandType_dataOptions::ectExpression | eCommandType_access::ectInvokeOnly | eCommandType_function::ectLiveAction | eCommandType_flags::ectLogic },
    { "ifgreater", "ifg", "variable:comparator:truecommandstring:elsecommandstring",
        "Performs [IF [variable] > [comparator]] and adds [truecommandstring] to the que if TRUE, otherwise adds [elsecommandstring]",false, false, &s_ifGreater,
        eCommandType_data::ectSimpleString | eCommandType_dataOptions::ectExpression | eCommandType_access::ectInvokeOnly | eCommandType_function::ectLiveAction | eCommandType_flags::ectLogic },
    { "ifless", "ifl", "variable:comparator:truecommandstring:elsecommandstring",
        "Performs [IF [variable] < [comparator]] and adds [truecommandstring] to the que if TRUE, otherwise adds [elsecommandstring]", false, false, &s_ifLess,
        eCommandType_data::ectSimpleString | eCommandType_dataOptions::ectExpression | eCommandType_access::ectInvokeOnly | eCommandType_function::ectLiveAction | eCommandType_flags::ectLogic },
    { "freeram", "free", "-", "Shows free RAM memory", false, false, &s_freeRAM, eCommandType_data::ectData | eCommandType_access::ectRequest | eCommandType_function::ectLiveStatistics },
    { "external", "ext", "commandlist", "Directly sending commands to external units", false, false, s_external,
        eCommandType_data::ectCommands | eCommandType_dataOptions::ectExpression | eCommandType_access::ectInvokeOnly | eCommandType_function::ectAdminAction },
    { "test", "test", "-", "-", false, false, &s_test, eCommandType_data::ectData | eCommandType_access::ectInvokeOnly | eCommandType_function::ectAdminAction }
};

int MasterModule::getMasterModuleCommandCount() const {
    return sizeof(masterModuleCommands) / sizeof(masterModuleCommands[0]);
}

MasterModule::MasterModule(ModuleHandler *inModule, ExpressionParser *inExpressionParser):BaseModule(inModule) {
    if (inModule != nullptr) {
        expressionParser = inExpressionParser;
        pluginHandler = new PluginHandler(expressionParser);
        inModule->addModule(pluginHandler);

#ifdef USE_EXTERNAL_MODULES
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
    CommandList fart;
    globalResponseCommands.addCommands(loadData);
    //globalResponseCommands.addCommands("ver:'FAR 1.1':::'1.1a20260916235822',dp:command:1,dp:usb:1,dp:hardware:1,dp:undefined:1,dp:priority:1,dp:error:1,dp:inforequest:1,dp:expressionparser:0,dp:debug:1,dp:textinfo:1,dp:help:1,dp:internal:1,dp:external:1,nick:,ins:so:1,ins:bw:1,so.xf:65535,so.if:0,so.fm:65535,so.ed:15000,ins:mcf:1,bw.ins:dcm:1,bw.mt:1000,bw.mfc:'bw.dcm.ru:0,bw.bp.rs:1',bw.moc:'bw.dcm.ru:0,bw.bp.rs:1,bw.dcm.es:1000',bw.ins:pid:1,bw.dcm.vo:6.60,bw.dcm.cl:1.75,bw.dcm.pl:12.84,bw.dcm.ip:0,bw.dcm.xp:65535,bw.ins:bp:1,bw.pid.ki:7.00,bw.pid.kp:500.00,bw.pid.kd:200.00,bw.pid.ie:0.10,bw.pid.xe:50.00,bw.pid.msx:550.00,bw.pid.msi:20.00,bw.ins:hsh:1,bw.bp.ins:ah:1,bw.bp.ah.ins:ac:1,bw.bp.ah.ac.rp:0,bw.bp.ah.ac.sp:65535,bw.bp.ah.ac.ep:2000,");
    //globalResponseCommands.addCommands("bw.hsh.ins:hs:2,bw.hsh.fu:66.00,bw.hsh.bn:64,bw.hsh.sr:12,bw.hsh.hs[0].da:'Just intonation':1.000:1.067:1.125:1.200:1.250:1.333:1.406:1.500:1.600:1.667:1.800:1.875,bw.hsh.hs[1].da:'Equal temperament':1.000:1.059:1.122:1.189:1.260:1.335:1.414:1.498:1.587:1.682:1.782:1.888,");
    //globalResponseCommands.addCommands("bw.hsh.hs[0],ins:mu:1,mcf.ins:mc:1,mcf.mc.ins:cc:2,mcf.mc.na:default,mcf.mc.non:'bw.hsh.hb:note,bw.dcm.ru:1,bw.pe:1,bw.bp.en:1,so.en:(velocity*512)*(1-notecount),bw.sm:0',mcf.mc.nof:'bw.bp.rs:ibool(notecount),bw.sm:0',mcf.mc.pat:'',mcf.mc.cat:'bw.bp.mo:(pressure*512)',mcf.mc.pb:'bw.hsh.sh:pitch*4',mcf.mc.pc:'',mcf.mc.ccd:64:'bw.bp.hd:bool(value)',mcf.mc.ccd:123:'mcf.ano:1',mcf.mc[0],ins:cb:1,mu.fmp:65535,mu.hmp:0,mu.rp:0,mu.bo:0,ins:ph:1,cb.har:'bw.hsh.ha:\"value/1327.716667-20\"',cb.has:'bw.hsh.sh5:\"deadband(value-32236, 20)/2.425\"',cb.fin:'bw.hsh.sh:\"deadband((value-32600)*0.49064, 250)\"',cb.pre:'bw.bp.ba:value',cb.mut:'mu.sp:value',cb.hms:'so.fm:\"deadband(value,50)\"',cb.gt:'bw.dcm.ru:bool(value-10000),bw.pe:1,bw.sm:0,bw.bp.en:bool(value-10000),bw.bp.rs:ibool(value-10000),bw.bp.hd:ibool(value-10000)',cb.hmt:'so.en:value',cb.ads:0:1:40:2:10,cb.ads:1:1:40:2:10,cb.ads:2:1:40:2:10,cb.ads:3:1:40:2:10,cb.ads:4:1:40:2:10,cb.ads:5:1:40:2:10,cb.ads:6:1:40:2:10,cb.ads:7:1:40:2:10");
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
#ifdef USE_EXTERNAL_MODULES
    externalModuleHandler->update();
#endif
}

CREATE_MODULE_COMMAND_FUNCTION(test, MasterModule) {
    init();
    return eProcessResult::Ok;
}

void MasterModule::init() {
#ifdef USE_EXTERNAL_MODULES
    externalModuleHandler->scanForModules();
#endif
};

#endif
