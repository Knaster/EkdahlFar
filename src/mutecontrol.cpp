#ifndef MUTECONTROL_C
#define MUTECONTROL_C

#include "mutecontrol.hpp"

MuteControl::MuteControl(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensor) {
    mute = new Mute(stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensor);
}

eProcessResult MuteControl::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsMute, sizeof(serialCommandsMute)  / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsMute, sizeof(serialCommandsMute) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "mutesetposition") {
        if (request) {
            commandResponses->push_back({ "msp:" + String(mute->getPosition()), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            mute->setPosition(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"msp:" + String(mute->getPosition()), InfoRequest});
        }
    } else
    if (inCommandItem->command == "mutefullmute") {
        if (request) {
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (inCommandItem->argument[0].toInt() == 1) {
                mute->fullMute();
                commandResponses->push_back({"mfm:1", InfoRequest});
            }
        }
    } else
    if (inCommandItem->command == "mutehalfmute") {
        if (request) {
        } else {
            mute->halfMute();
            commandResponses->push_back({"mhm:1", InfoRequest});
        }
    } else
    if (inCommandItem->command == "muterest") {
        if (request) {
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (inCommandItem->argument[0].toInt() == 1) {
                mute->rest();
                commandResponses->push_back({"mr:1", InfoRequest});
            }
        }
    } else
    if (inCommandItem->command == "mutesavefull") {
        mute->saveFullMute();
        commandResponses->push_back({"msf:1", InfoRequest});
    } else
    if (inCommandItem->command == "mutesavehalf") {
        mute->saveHalfMute();
        commandResponses->push_back({"msh:1", InfoRequest});
    } else
    if (inCommandItem->command == "mutesaverest") {
        mute->saveRest();
        commandResponses->push_back({"msr:1", InfoRequest});
    } else
    if (inCommandItem->command == "mutefullmuteposition") {
        if (request) {
            commandResponses->push_back({ "mfmp:" + String(mute->getFullMutePosition()), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            mute->setFullMutePosition(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"mfmp:" + String(mute->getFullMutePosition()), InfoRequest});
        }
    } else
    if (inCommandItem->command == "mutehalfmuteposition") {
        if (request) {
            commandResponses->push_back({ "mhmp:" + String(mute->getHalfMutePosition()), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            mute->setHalfMutePosition(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"mhmp:" + String(mute->getHalfMutePosition()), InfoRequest});
        }
    } else
    if (inCommandItem->command == "muterestposition") {
        if (request) {
            commandResponses->push_back({ "mrp:" + String(mute->getRestPosition()), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            mute->setRestPosition(inCommandItem->argument[0].toInt());
            commandResponses->push_back({ "mrp:" + String(mute->getRestPosition()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "mutesustain") {
        if (request) {
            commandResponses->push_back({ "ms:" + String(mute->getSustain()), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (inCommandItem->argument[0].toInt() == 1) {
                mute->setSustain(true);
            } else {
                mute->setSustain(false);
            }
            commandResponses->push_back({"ms:" + String(mute->getSustain()), InfoRequest});
        }
    } else
    if (inCommandItem->command == "mutebackoff") {
        if (request) {
            commandResponses->push_back({ "mbo:" + String(mute->getBackOffTime()), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            mute->setBackOffTime(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"mbo:" + String(mute->getBackOffTime()), InfoRequest});
        }
    } else
    if (inCommandItem->command == "mutehome") {
        if (!mute->homeMute()) {
            commandResponses->push_back({"mutehome:0", Error});
        } else {
            commandResponses->push_back({"mutehome:1", InfoRequest});
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

eProcessResult MuteControl::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false, commandList *delegatedCommands = nullptr) {
    processCommandItems(inCommandItem, serialCommandsMuteHidden, sizeof(serialCommandsMuteHidden)  / sizeof(serialCommandItem));

    if (inCommandItem->command == "helphidden") {
        addCommandHelp(serialCommandsMuteHidden, sizeof(serialCommandsMuteHidden) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "mutesethardwareposition") {
        if (request) {
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setHardwarePosition(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"msp:" + inCommandItem->argument[0], InfoRequest});
        }
    } else
    if (inCommandItem->command == "mutecompletetask") {
        if (completeTask()) {
            commandResponses->push_back({"mcpt:1", InfoRequest});
        } else {
            commandResponses->push_back({"mcpt:0", InfoRequest});
        }
    } else
    if (inCommandItem->command == "muteautocorrect") {
        if (request) {
            commandResponses->push_back({ "mac:" + String(getAutoCorrect()), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setAutoCorrect(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"mac:" + String(getAutoCorrect()), InfoRequest});
        }
    } else
    if (inCommandItem->command == "mutehomingsensed") {
        if (request) {
            commandResponses->push_back({ "mhmse:" + String(getHomingSensed()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "mutemovedirection") {
        if (request) {
            commandResponses->push_back({ "mmd:" + String(getMoveDirection()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "mutecurrentstep") {
        if (request) {
            commandResponses->push_back({ "mcs:" + String(getCurrentStep()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "mutehomingpoint") {
        if (request) {
            if (!checkArguments(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
            commandResponses->push_back({ "mhmp:" + String(getHomingPoint(inCommandItem->argument[0].toInt(), inCommandItem->argument[1].toInt())), InfoRequest });
        }
    } else
    if (inCommandItem->command == "mutehomingstage") {
        if (request) {
            commandResponses->push_back({ "mhms:" + String(getHomingStage()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "mutetmcinfo") {
        if (request) { getTMC2209Info(); }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

String MuteControl::dumpData() {
    String dump = "";
    dump += "mfmp:" + String(mute->getFullMutePosition()) + ",";
    dump += "mhmp:" + String(mute->getHalfMutePosition()) + ",";
    dump += "mrp:" + String(mute->getRestPosition()) + ",";
    dump += "mbo:" + String(mute->getBackOffTime()) + ",";
    return dump;
}

#endif
