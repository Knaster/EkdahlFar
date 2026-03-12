#ifndef EXTERNALMODULE_CPP
#define EXTERNALMODULE_CPP

#include "externalmodule.hpp"

ExternalModule::ExternalModule(String deviceString) {
    CommandItem ver(&deviceString);
    kLongName = ver.argument[1];
    kShortName = ver.argument[2];
    kDescription = ver.argument[0];
    kModuleType = eModuleType::hardware;
    kGroupHandler = ver.argument[3].toInt();
    tmoduleID = new tModuleID { kLongName.c_str(), kShortName.c_str(), kDescription.c_str(), kModuleType, kGroupHandler };

    debugPrintln("Creating external module with long name: " + String(tmoduleID->longName) + " short name " + String(tmoduleID->shortName) + " description " +
                 String(tmoduleID->description), debugPrintType::Debug);
}

eProcessResult ExternalModule::processCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request) {
    debugPrintln("External: processCommands", debugPrintType::Debug);
    String commandString = buildCommandString(inCommandItem);
    if (request) { commandString = "rqi:" + delimitExpression(commandString); }
//    debugPrintln("Built command string " + commandString, debugPrintType::Debug);
    transmit(commandString, extID, transmissionObject);
    return eProcessResult::Ok;
}

void ExternalModule::dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive) {
    debugPrintln("External: dir", debugPrintType::Debug);
    String lsStr = "ls";
    if (hidden) { lsStr += ":h"; }
    if (modules) { lsStr += ":m"; }
    if (commands) { lsStr += ":c"; }
    if (instances) { lsStr += ":i"; }
    if (instanceCount) { lsStr += ":ic"; }
    if (recursive) { lsStr += ":r"; }
    CommandItem cmd = CommandItem(&lsStr);
    processCommands(&cmd, inCommandResponses, false);
}

void ExternalModule::dumpData(std::vector<commandResponse> *dataDump) {
    debugPrintln("External: dump", debugPrintType::Debug);
    String cmdDump = "dump";
    //&String("dump")
    CommandItem cmd = CommandItem(&cmdDump);
    processCommands(&cmd, dataDump, false);
}

void ExternalModule::help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix) {
    debugPrintln("External: help", debugPrintType::Debug);
    String cmdHelp = "help";
    //String("help")
    CommandItem cmd = CommandItem(&cmdHelp);
    processCommands(&cmd, inCommandResponses, false);
}

#endif
