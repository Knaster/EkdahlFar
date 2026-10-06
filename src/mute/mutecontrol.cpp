#ifndef MUTECONTROL_C
#define MUTECONTROL_C

#include "mute/mutecontrol.hpp"

const ModuleCommandDeclaration MuteControl::moduleCommands[] = {
    { "setposition", "sp", "0-65535", "Set mute position", false, false, &s_setPosition, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectLiveParameter },
    { "fullmute", "fm", "1|0", "Put mute in full mute position (conditional)", false, false, &s_fullMute, eCommandType_data::ectConditional | eCommandType_function::ectLiveParameter },
    { "halfmute", "hfm", "1|0", "Put mute in half mute position (conditional)", false, false, &s_halfMute, eCommandType_data::ectConditional | eCommandType_function::ectLiveParameter },
    { "rest", "rs", "1|0", "Put mute in rest position (conditional)", false, false, &s_rest, eCommandType_data::ectConditional | eCommandType_function::ectLiveParameter },
    { "savefull", "sf", "1|0", "Save current mute position as full mute position (conditional)", false, false, &s_saveFull, eCommandType_data::ectConditional | eCommandType_function::ectAdminAction },
    { "savehalf", "sh", "1|0", "Save current mute position as half mute position (conditional)", false, false, &s_saveHalf, eCommandType_data::ectConditional | eCommandType_function::ectAdminAction },
    { "saverest", "sr", "1|0", "Save current mute position as mute rest position (conditional)", false, false, &s_saveRest, eCommandType_data::ectConditional | eCommandType_function::ectAdminAction },
    { "fullmuteposition", "fmp", "0-65535", "Set the value of the full mute position", false, true, s_fullMutePosition, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectCalibration },
    { "halfmuteposition", "hmp", "0-65535", "Set the value of the Mute half mute position", false, true, s_halfMutePosition, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectCalibration },
    { "restposition", "rp", "0-65535", "Set the value of the mute rest position", false, true, s_restPosition, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectCalibration },
    { "sustain", "su", "0|1", "Setting sustain on (1) or off (0)", false, false, s_sustain, eCommandType_data::ectSimpleBool | eCommandType_function::ectLiveParameter },
    { "backoff", "bo", "0-65535", "Setting the time that the mute stays in the mutefullmute position before automatically going into rest, set in mS", false, true, &s_backoff,
        eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectCalibration },
    { "home", "hm", "-", "Home mute", false, false, &s_home, eCommandType_data::ectConditional | eCommandType_function::ectAdminAction | eCommandType_flags::ectSystem },
    { "hardwareposition", "hwp", "0-65535", "Sets the mute position without min/max scaling", true, false, &s_setHardwarePosition,
        eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectAdminSetting | eCommandType_flags::ectSystem },
    { "completetask", "cpt", "-", "Requests a callback when the current operation is finished", true, false, &s_completeTask,
        eCommandType_data::ectConditional | eCommandType_function::ectAdminAction | eCommandType_access::ectInvokeOnly },
    { "autocorrect", "ac", "1|0", "Enables auto correct positioning that uses the homing sensor to correct positioning on the fly", true, false, &s_autoCorrect,
        eCommandType_data::ectSimpleBool | eCommandType_function::ectAdminSetting | eCommandType_flags::ectVolatileSetting },
    { "homingsensed", "hms", "-", "Requests whether the homing sensor is currently active or not", true, false, &s_homingSensed,
        eCommandType_data::ectSimpleBool | eCommandType_access::ectRequest | eCommandType_flags::ectSystem | eCommandType_function::ectInternalStatistics },
    { "movedirection", "md", "-", "Requests the current direction of movement (0 Forward, 1 Reverse)", true, false, &s_moveDirection,
        eCommandType_data::ectSimpleBool | eCommandType_access::ectRequest | eCommandType_flags::ectSystem | eCommandType_function::ectInternalStatistics },
    { "currentstep", "cs", "-", "Requests the current step", true, false, &s_currentStep, eCommandType_data::ectSimpleUInt16 | eCommandType_access::ectRequest | eCommandType_flags::ectSystem |
        eCommandType_function::ectInternalStatistics},
    { "homingpoint", "hmp", "edge,direction", "Requests the given homing point", true, false, &s_homingPoint,
        eCommandType_data::ectData | eCommandType_access::ectRequest | eCommandType_flags::ectSystem | eCommandType_function::ectInternalStatistics },
    { "homingstage", "hms", "-", "Requests the current homing stage (UNHOMED, HOMED, FIRSTHOMINGRISING, SECONDHOMINGFALLING, SECONDHOMINGRISING, FIRSTHOMINGFALLING, MOVEPASTHOMESWITCH, MOVETOHOMESWITCH, GOTOOFFSET)",
        true, false, &s_homingStage, eCommandType_data::ectData | eCommandType_access::ectRequest | eCommandType_flags::ectSystem | eCommandType_function::ectInternalStatistics },
    { "tmcinfo", "tmi", "-", "Request statistical information from the TMC2209", true, false, &s_tmcInfo,
        eCommandType_data::ectData | eCommandType_access::ectRequest | eCommandType_flags::ectSystem | eCommandType_function::ectInternalStatistics },
};

getModuleCount(MuteControl)

MuteControl::MuteControl(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensor) {
    mute = new Mute(stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensor);

//    moduleID = new ModuleID("mute", "mu", "Mute controller v1.0", eModuleType::hardware);
}

CREATE_MODULE_COMMAND_FUNCTION(setPosition, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(mute->getPosition()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        mute->setPosition(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(mute->getPosition()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(fullMute, MuteControl) {
    if (request) {
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() == 1) {
            mute->fullMute();
            inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(halfMute, MuteControl) {
    if (request) {
    } else {
        mute->halfMute();
        inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(rest, MuteControl) {
    if (request) {
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() == 1) {
            mute->rest();
            inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(saveFull, MuteControl) {
    if (!request) {
        mute->saveFullMute();
        inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(saveHalf, MuteControl) {
    if (!request) {
        mute->saveHalfMute();
        inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(saveRest, MuteControl) {
    if (!request) {
        mute->saveRest();
        inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(fullMutePosition, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(mute->getFullMutePosition()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        //mute->setFullMutePosition(inCommandItem->argument[0].toInt());
        setFullMutePosition(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(mute->getFullMutePosition()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(halfMutePosition, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(mute->getHalfMutePosition()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        //mute->setHalfMutePosition(inCommandItem->argument[0].toInt());
        setHalfMutePosition(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(mute->getHalfMutePosition()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(restPosition, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(mute->getRestPosition()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        //mute->setRestPosition(inCommandItem->argument[0].toInt());
        setRestPosition(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(mute->getRestPosition()), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(sustain, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(mute->getSustain()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() == 1) {
            mute->setSustain(true);
        } else {
            mute->setSustain(false);
        }
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(mute->getSustain()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(backoff, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(mute->getBackOffTime()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        mute->setBackOffTime(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(mute->getBackOffTime()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(home, MuteControl) {
    if (!request) {
        if (!mute->homeMute()) {
            inCommandResponses->push_back({thisItem.shortCommand + ":0", InfoRequest});
        } else {
            inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(setHardwarePosition, MuteControl) {
    if (request) {
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setHardwarePosition(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + inCommandItem->argument[0], InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(completeTask, MuteControl) {
    if (!request) {
        if (completeTask()) {
            inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
        } else {
            inCommandResponses->push_back({thisItem.shortCommand + ":0", InfoRequest});
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(autoCorrect, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getAutoCorrect()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setAutoCorrect(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(getAutoCorrect()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(homingSensed, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getHomingSensed()), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(moveDirection, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getMoveDirection()), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(currentStep, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getCurrentStep()), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(homingPoint, MuteControl) {
    if (request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getHomingPoint(inCommandItem->argument[0].toInt(), inCommandItem->argument[1].toInt())), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(homingStage, MuteControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getHomingStage()), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(tmcInfo, MuteControl) {
    if (request) { getTMC2209Info(); }
    return eProcessResult::Ok;
};
#endif
