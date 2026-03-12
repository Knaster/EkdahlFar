
/*
const serialCommandItem serialCommandsFarSingle[] = {
    { "bowcalibrateall", "bca", "-", "Performs all calibration routines on the selected bow, see below for routines performed" },
    { "bowcalibratespeed", "bcs", "-", "Finds the minimum and maximum bow speed of the selected bow" },
    { "bowcalibratepressure", "bcp", "-", "Finds the minimum and maximum bow pressure of the selected bow" },
    { "pickupstringfrequency", "psf", "-", "Returns the fundamental tone calculated from the current audio signal if appliccable"},
    { "pickupaudiopeak", "pap", "-", "Returns the peak amplitude of the current audio signal"},
    { "pickupaudiorms", "par", "-", "Returns the RMS amplitude of the current audio signal"},
    { "mutecalibrate", "mca", "-", "Calibrate mute settings"}
};

eProcessResult FARSingle::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsFarSingle, sizeof(serialCommandsFarSingle)  / sizeof(serialCommandItem));

    eProcessResult processResult = eProcessResult::Ok;
    eCalibrationResult calibrationResult;

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsFarSingle, sizeof(serialCommandsFarSingle) / sizeof(serialCommandItem), commandResponses,"");
    }

    if (inCommandItem->command == "bowcalibratespeed") {
        if (!request) {
            calibrationResult = calibrateBow->findMinMaxSpeedPWM();
            if (calibrationResult == CR_Ok) { processResult = eProcessResult::Ok; } else { exitWithError(calibrationResult); processResult = eProcessResult::CommandFailed; }
            commandResponses->push_back({ "bcs", InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowcalibratepressure") {
        if (!request) {
            calibrationResult = calibrateBow->findMinMaxPressure();
            if (calibrationResult == CR_Ok) { processResult = eProcessResult::Ok; } else { exitWithError(calibrationResult); processResult = eProcessResult::CommandFailed; }
            commandResponses->push_back({ "bcp", InfoRequest });
        }
    } else
    if (inCommandItem->command == "mutecalibrate") {
        if (!request) {
            calibrationResult = calibrateMute->calibrateAll();
            if (calibrationResult == CR_Ok) { processResult = eProcessResult::Ok; } else { exitWithError(calibrationResult); processResult = eProcessResult::CommandFailed; }
            commandResponses->push_back({ "mca", InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowcalibrateall") {
        if (!request) {
            if (calibrateBow->calibrateAll()) { processResult = eProcessResult::Ok; } else { processResult = eProcessResult::CommandFailed; }
            commandResponses->push_back({ "bca", InfoRequest });
        }
    } else
    if (inCommandItem->command == "mutecalibrate") {
        if (!calibrateMute->calibrateAll()) {
            commandResponses->push_back({"mca:1", InfoRequest});
        } else {
            commandResponses->push_back({"mca:0", InfoRequest});
        }
    } else
    if (inCommandItem->command == "pickupstringfrequency") {
        if (audioFrequencyAvaliable()) {
            commandResponses->push_back({ "psf:" + String(audioFrequency(),1), InfoRequest });
        } else {
            commandResponses->push_back({ "psf: 0", InfoRequest });
        }
    }  else
    if (inCommandItem->command == "pickupaudiopeak") {
        commandResponses->push_back({ "pap:" + String(audioPeakAmplitude()), InfoRequest });
    }  else
    if (inCommandItem->command == "pickupaudiorms") {
        commandResponses->push_back({ "par:" + String(audioRMSAmplitude()), InfoRequest });
    } else {
        processResult = muteControl->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = hammerControl->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = bowControl->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = bowActuators->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = controlReader->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = harmonicSeriesHandler->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    }
    return processResult;
}

eProcessResult FARSingle::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    eProcessResult processResult;

    processResult = bowControl->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

    processResult = muteControl->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

    processResult = controlReader->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

    return processResult;
}



void FARSingle::dir(std::vector<commandResponse> *commandResponses, String longPrefix, String shortPrefix) {
//    commandResponses->push_back({ "dir:" + identifier + "'", debugPrintType::InfoRequest });

    dirProto(commandResponses, longPrefix, shortPrefix, serialCommandsFarSingle, sizeof(serialCommandsFarSingle) / sizeof(serialCommandItem), nullptr, 0);

    muteControl->dir(commandResponses, longPrefix, shortPrefix);
    hammerControl->dir(commandResponses, longPrefix, shortPrefix);
    bowControl->dir(commandResponses, longPrefix, shortPrefix);
    bowActuators->dir(commandResponses, longPrefix, shortPrefix);
    harmonicSeriesHandler->dir(commandResponses, longPrefix, shortPrefix);
}


bool FARSingle::enumerateModules() {
    //addModule(this);

    return true;
}

const serialCommandItem serialCommandsSolenoid[] = {
    { "solenoidengage", "se", "0-65535", "Sets the solenoid engagement of the current bow using the 1st argument as hardness" },
    { "solenoiddisengage", "sd", "0-1", "Sets the solenoid disengagement of the current bow, an argument of '0' will leave the solenoid at its current state while any other value will disengage the solenoid" },
    { "solenoidmaxforce", "sxf", "0 - 65535", "Set solenoid maximum usable force" },
    { "solenoidminforce", "sif", "0 - 65535", "Set solenoid minimum usable force" },
    { "solenoidforcemultiplier", "sfm", "0 - 1", "Set solenoid force multiplier" },
    { "solenoidengageduration", "sed", "uS", "Sets the duration of the solenoid hit in uS, if a value of 0 is set the solenoid will not disengage until a solenoiddisengage command has been given"}
};

eProcessResult Solenoid::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsSolenoid, sizeof(serialCommandsSolenoid) / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsSolenoid, sizeof(serialCommandsSolenoid) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "solenoidengage") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        solenoidEngage(inCommandItem->argument[0].toInt());
        commandResponses->push_back({"se:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "solenoiddisengage") {
        solenoidDisengage();
        commandResponses->push_back({"sd:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "solenoidengageduration") {
        if (request) {
            commandResponses->push_back({ "sed:" + String(solenoidEngageDuration), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setSolenoidDuration(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"sed:" + String(solenoidEngageDuration), InfoRequest});
        }
    } else
    if (inCommandItem->command == "solenoidmaxforce") {
        if (request) {
            commandResponses->push_back({ "sxf:" + String(forceMax), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setSolenoidMax(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"sfx:" + String(forceMax), InfoRequest});
        }
    } else
    if (inCommandItem->command == "solenoidminforce") {
        if (request) {
            commandResponses->push_back({ "sif:" + String(forceMin), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setSolenoidMin(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"sif:" + String(forceMin), InfoRequest});
        }
    } else
    if (inCommandItem->command == "solenoidforcemultiplier") {
        if (request) {
            commandResponses->push_back({ "sfm:" + String(fForceMultiplier), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setSolenoidMultiplier(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"sfm:" + String(fForceMultiplier), InfoRequest});
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

eProcessResult Solenoid::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {
    return eProcessResult::NotFound;
}

const serialCommandItem serialCommandsMute[] = {
    { "mutesetposition", "msp", "0-65535", "Set mute position" },
    { "mutefullmute", "mfm", "1|0", "Put mute in full mute position (conditional)" },
    { "mutehalfmute", "mhm", "1|0", "Put mute in half mute position (conditional)" },
    { "muterest", "mr", "1|0", "Put mute in rest position (conditional)" },
    { "mutesavefull", "msf", "1|0", "Save current mute position as full mute position (conditional)" },
    { "mutesavehalf", "msh", "1|0", "Save current mute position as half mute position (conditional)" },
    { "mutesaverest", "msr", "1|0", "Save current mute position as mute rest position (conditional)" },
    { "mutefullmuteposition", "mfmp", "0-65535", "Mute full mute position"},
    { "mutehalfmuteposition", "mhmp", "0-65535", "Mute half mute position"},
    { "muterestposition", "mrp", "0-65535", "Mute rest position"},
    { "mutesustain", "ms", "0|1", "Setting sustain on (1) or off (0)" },
    { "mutebackoff", "mbo", "0-65535", "Setting the time that the mute stays in the mutefullmute position before automatically going into rest, set in mS"},
    { "mutehome", "mh", "-", "Home mute" }
};

const serialCommandItem serialCommandsMuteHidden[] =  {
    { "mutesethardwareposition", "mshwp", "0-65535", "Sets the mute position without min/max scaling" },
    { "mutecompletetask", "mcpt", "-", "Requests a callback when the current operation is finished" },
    { "muteautocorrect", "mac", "1|0", "Enables auto correct positioning that uses the homing sensor to correct positioning on the fly" },
    { "mutehomingsensed", "mhmse", "-", "Requests whether the homing sensor is currently active or not" },
    { "mutemovedirection", "mmd", "-", "Requests the current direction of movement (0 Forward, 1 Reverse)" },
    { "mutecurrentstep", "mcs", "-", "Requests the current step" },
    { "mutehomingpoint", "mhmp", "edge,direction", "Requests the given homing point" },
    { "mutehomingstage", "mhms", "-", "Requests the current homing stage (UNHOMED, HOMED, FIRSTHOMINGRISING, SECONDHOMINGFALLING, SECONDHOMINGRISING, FIRSTHOMINGFALLING, MOVEPASTHOMESWITCH, MOVETOHOMESWITCH, GOTOOFFSET)" },
    { "mutetmcinfo", "mtmi", "-", "Request statistical information from the TMC2209" }
};

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

eProcessResult MuteControl::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {
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

serialCommandItem serialCommandsHarmonicSeriesHandler[] = {
    { "bowharmonicseriesfundamental", "bcu", "float", "Legacy command conversion" },
    { "bowharmonicseriesharmonic", "bch", "int", "Legacy command conversion" },
    { "bowharmonicseriesharmonicadd", "bcha", "int", "Legacy command conversion" },
    { "bowharmonicseriesharmonicbasenote", "bchb", "int", "Legacy command conversion" },
    { "bowharmonicseriesharmonicbasenote", "bchbn", "0-127", "Legacy command conversion"},
    { "bowharmonicseriesharmonicshift", "bchsh", "-32767-32767", "Legacy command conversion" },
    { "bowharmonicseriesharmonicshiftrange", "bchsr", "0-36", "Legacy command conversion" },
    { "bowharmonicseriesharmonicshift5", "bchs5", "-32767-32767", "Legacy command conversion" },


    { "bowharmonicseriesfundamental", "bhsu", "float", "Bow fundamental frequency, all harmonics are calculated from this number" },
    { "bowharmonicseriesharmonic", "bhsh", "int", "Bow motor speed in terms of a harmonic number. A ratio is taken from the given harmonic in the current harmonic list, the ratio is then multiplied by the bow fundamental frequency" },
    { "bowharmonicseriesharmonicadd", "bhsha", "int", "Additative version of bowcontrolharmonic, the number is given is added to the harmonic given" },
    { "bowharmonicseriesharmonicbase", "bhshb", "int", "Same as bowcontrolharmonic but where the harmonic number is based on a MIDI note given by bowcontrolbasenote" },
    { "bowharmonicseriesharmonicbasenote", "bhshbn", "0-127", "Sets the MIDI base note of the string, used in conjunction with bowcontrolharmonicbase"},
    { "bowharmonicseriesharmonicshift", "bhshsh", "-32767-32767", "Setting shift from the currently playing harmonic where 32767 equals the entire harmonic shift range shifted up" },
    { "bowharmonicseriesharmonicshiftrange", "bhshsr", "0-36", "Set the number of harmonic numbers that constitutes the entire harmonic shift" },
    { "bowharmonicseriesharmonicshift5", "bhshs5", "-32767-32767", "Setting shift from the currently playing harmonic over 5 octaves where 32767 equals 5 octaves shift up from the fundamental" },


    { "bowharmonicseries", "bhs", "int", "Get/set the current harmonic series" },
    { "bowharmonicseriesdata", "bhsd", "slot:name:ratios", "Sets all data for the harmonic series in the given slot" },
    { "bowharmonicseriesratio", "bhsr", "harmonic:ratio", "Sets the ratio of the given harmonic in current harmonic series, will increase the list size if needed to adress the harmonic" },
    { "bowharmonicseriesratioremove", "bhsrr", "ratio", "Remove the harmmonic ratio given in the current series and shift any ratios accordingly. Cannot remove all ratios"},
    { "bowharmonicseriescount", "bhsc", "-", "Returns the number of harmonic series in the list and their IDs"},
    { "bowharmonicseriessave", "bhss", "series:name", "Saves the current harmonic series in the slot given, or if the slot is out of range, creates a new one at the end of the list"},
    { "bowharmonicseriesremove", "bhsrm", "series", "Remove the series given and shift any series accordingly. Cannot remove all series"}
};

eProcessResult HarmonicSeriesHandler::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsHarmonicSeriesHandler, sizeof(serialCommandsHarmonicSeriesHandler)  / sizeof(serialCommandItem));

    eProcessResult processResult;

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsHarmonicSeriesHandler, sizeof(serialCommandsHarmonicSeriesHandler) / sizeof(serialCommandItem), commandResponses,"");
        processResult = eProcessResult::PassThrough;
    } else {

        processResult = processControlCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);

        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = processHarmonicSeriesCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    }

    return processResult;
}

eProcessResult HarmonicSeriesHandler::processControlCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    if (inCommandItem->command == "bowcontrolfundamental") {
        if (request) {
            commandResponses->push_back({ "bcu:" + String(fundamentalFrequency), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            fundamentalFrequency = inCommandItem->argument[0].toFloat();
            commandResponses->push_back({"bcu:" + String(fundamentalFrequency), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowcontrolharmonic") {
        if (request) {
            commandResponses->push_back({ "bch:" + String(harmonic), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (!setHarmonic(inCommandItem->argument[0].toInt())) {
                commandResponses->push_back({"ERROR setting harmonic to " + String(inCommandItem->argument[0].toInt()), Error});
            } else {
                commandResponses->push_back({ "bch:" + String(harmonic), InfoRequest });
            }
        }
    } else
    if (inCommandItem->command == "bowcontrolharmonicadd") {
        if (request) {
            commandResponses->push_back({ "bcha:" + String(harmonicAdd), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (!setHarmonicAdd(inCommandItem->argument[0].toInt())) {
                commandResponses->push_back({"ERROR setting harmonic add to " + String(inCommandItem->argument[0].toInt()), Error});
            } else {
                commandResponses->push_back({ "bcha:" + String(harmonicAdd), InfoRequest });
            }
        }
    } else
    if (inCommandItem->command == "bowcontrolharmonicbase") {
        if (request) {
            commandResponses->push_back({ "bchb:" + String(harmonic + baseNote), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setHarmonic(inCommandItem->argument[0].toInt() - baseNote);
            commandResponses->push_back({ "bchb:" + String(harmonic + baseNote), InfoRequest });
            commandResponses->push_back({ "bch:" + String(harmonic), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowcontrolharmonicbasenote") {
        if (request) {
            commandResponses->push_back({ "bchbn:" + String(baseNote), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setBaseNote(inCommandItem->argument[0].toInt());
            commandResponses->push_back({ "bchbn:" + String(baseNote), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowcontrolharmonicshift") {
        if (request) {
            commandResponses->push_back({ "bchsh:" + String(harmonicShift), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setHarmonicShift(inCommandItem->argument[0].toInt());
            commandResponses->push_back({ "bchsh:" + String(harmonicShift), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowcontrolharmonicshiftrange") {
        if (request) {
            commandResponses->push_back({ "bchsr:" + String(harmonicShiftRange), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setHarmonicShiftRange(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({ "bchsr:" + String(harmonicShiftRange), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowcontrolharmonicshift5") {
        if (request) {
            commandResponses->push_back({ "bchs5:" + String(harmonicShift5), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setHarmonicShift5(inCommandItem->argument[0].toInt());
            commandResponses->push_back({ "bchs5:" + String(harmonicShift5), InfoRequest });
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

eProcessResult HarmonicSeriesHandler::processHarmonicSeriesCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    if (inCommandItem->command == "bowharmonicseries") {
        if (request) {
            commandResponses->push_back({ "bhs:" + String(currentHarmonicSeries), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (!validateNumber(inCommandItem->argument[0].toInt(), 0, harmonicSeriesList.series.size())) { return eProcessResult::WrongArgumentValue; }
            uint8_t hs = inCommandItem->argument[0].toInt();
            if (hs > (harmonicSeriesList.series.size() - 1)) {
                commandResponses->push_back({"Harmonic list doesn't exist " + String(hs), Error});
                return eProcessResult::CommandFailed;
            }
            loadHarmonicSeries(hs);
            commandResponses->push_back({ "bhs:" + String(currentHarmonicSeries), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowharmonicseriesdata") {
        if (request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            int ser = inCommandItem->argument[0].toInt();
            if (!validateNumber(ser, 0, harmonicSeriesList.series.size() - 1)) {
                commandResponses->push_back({"Harmonic series out of range", Error});
                return eProcessResult::CommandFailed;
            }
            int j=0;
            String response = "bhsd";
            response += ":" + String(ser) + ":" + harmonicSeriesList.series[ser].Id;
            while (j < int(harmonicSeriesList.series[ser].ratio.size())) {
                response += ":" + String(harmonicSeriesList.series[ser].ratio[j], 4);
                j++;
            }
            commandResponses->push_back({response, InfoRequest});
        } else {
            if (!checkArgumentsMin(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
            String response = "Setting harmonic series data:" + String(inCommandItem->argument[0].toInt());

            while (inCommandItem->argument[0].toInt() > harmonicSeriesList.series.size() - 1) {
                harmonicSeriesList.addHarmonicSeries();
            }

            harmonicSeriesList.series[inCommandItem->argument[0].toInt()].ratio.clear();
            harmonicSeriesList.series[inCommandItem->argument[0].toInt()].Id = String(inCommandItem->argument[1]);

            for (int i = 1; i < (int(inCommandItem->argument.size()) - 1); i++) {
                response += ":" + inCommandItem->argument[i + 1];
                harmonicSeriesList.series[inCommandItem->argument[0].toInt()].setHarmonic(i - 1, inCommandItem->argument[i + 1].toFloat());
            }

            if (harmonicSeriesList.series[currentHarmonicSeries].ratio.size() == 0) {
                debugPrintln("Current harmonic series is zero, reloading", debugPrintType::Debug);
                loadHarmonicSeries(currentHarmonicSeries);
            }

            updateHarmonicData();
            commandResponses->push_back({response, InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowharmonicseriesratio") {
        if (!checkArguments(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        harmonicSeriesList.series[currentHarmonicSeries].setHarmonic(inCommandItem->argument[0].toInt(), inCommandItem->argument[1].toFloat());

        updateHarmonicData();

        commandResponses->push_back({"bhsr:" + String(inCommandItem->argument[0].toInt()) + ":" + String(inCommandItem->argument[1].toFloat()), InfoRequest});
    } else
    if (inCommandItem->command == "bowharmonicseriesratioremove") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (harmonicSeriesList.series[currentHarmonicSeries].ratio.size() < 2) {
            commandResponses->push_back({"Cannot remove all harmonics", Error});
            return eProcessResult::CommandFailed;
        }

        int i = inCommandItem->argument[0].toInt();
        harmonicSeriesList.series[currentHarmonicSeries].ratio.erase(harmonicSeriesList.series[currentHarmonicSeries].ratio.begin() + i);
        updateHarmonicData();

        commandResponses->push_back({"bhsrr:" + String(i), InfoRequest});
    } else
    if (inCommandItem->command == "bowharmonicseriescount") {
        String response = "bhsc:" + String(harmonicSeriesList.series.size());
        for (int i=0; i<harmonicSeriesList.series.size(); i++) {
            response += ":" + harmonicSeriesList.series[i].Id;
        }
        commandResponses->push_back({ response, InfoRequest });
    } else
    if (inCommandItem->command == "bowharmonicseriessave") {
        if (request) {
            commandResponses->push_back({ "bhss:" + String(harmonicSeriesList.series.size()), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
            if (!validateNumber(inCommandItem->argument[0].toInt(), 0, harmonicSeriesList.series.size() - 1, true)) {
                harmonicSeriesList.addHarmonicSeries(inCommandItem->argument[1], harmonicSeriesList.series[currentHarmonicSeries].ratio);
                currentHarmonicSeries = harmonicSeriesList.series.size() - 1;
            } else {
                harmonicSeriesList.series[inCommandItem->argument[0].toInt()] = harmonicSeriesList.series[currentHarmonicSeries];
                harmonicSeriesList.series[inCommandItem->argument[0].toInt()].Id = inCommandItem->argument[1];
                currentHarmonicSeries = inCommandItem->argument[0].toInt();
            }
            commandResponses->push_back({ "bhss:" + String(currentHarmonicSeries), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowharmonicseriesremove") {
        if (request) {
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            int listNo = inCommandItem->argument[0].toInt();
            debugPrintln("Attempting to remove list " + String(listNo), debugPrintType::Debug);
            if (!validateNumber(listNo, 0, harmonicSeriesList.series.size() - 1, true)) { return eProcessResult::WrongArgumentValue; }

            harmonicSeriesList.series.erase(
                harmonicSeriesList.series.begin() + listNo);

            if ((currentHarmonicSeries >= listNo) && (currentHarmonicSeries > 0)) {
                currentHarmonicSeries--;
            }

            commandResponses->push_back({ "bhsrm:" + String(currentHarmonicSeries), InfoRequest });
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

const serialCommandItem serialCommandsBowActuators[] = {
    { "bowactuator", "ba", "actuator", "Sets and loads the actuator given"},
    { "bowactuatorremove", "bar", "actuator", "Remove bow actuator"},
    { "bowactuatorsave", "bas", "-", "Save current bow parameters into currently selected bow actuator" },
    { "bowactuatordata", "bad", "actuator:engagepressure:stallpressure:restposition:id", "Set data of bow actuator [actuator]" },
    { "bowactuatorcount", "bac", "-", "Returns the amount of saved bow actuators"}
};

eProcessResult BowActuators::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsBowActuators, sizeof(serialCommandsBowActuators)  / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsBowActuators, sizeof(serialCommandsBowActuators) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "bowactuator") {
        if (request) {
            commandResponses->push_back({ "ba:" + String(m_currentBowActuator), debugPrintType::InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (!loadBowActuator(inCommandItem->argument[0].toInt())) {
                return eProcessResult::CommandFailed;
            }
            commandResponses->push_back({"ba:" + String(m_currentBowActuator), debugPrintType::InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowactuatorsave") {
        if (!checkArguments(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        if (!saveBowActuator(inCommandItem->argument[0].toInt(), inCommandItem->argument[1])) {
            return eProcessResult::CommandFailed;
        }
        //uint8_t actuator = getBowActuator();
        commandResponses->push_back({"bas:" + String(m_currentBowActuator) + ":" + getBowActuatorID(m_currentBowActuator), InfoRequest});
    }  else
    if (inCommandItem->command == "bowactuatordata") {
        int8_t bowIndex;
        if (request) {
            bowIndex = m_currentBowActuator;
            if (checkArguments(inCommandItem, commandResponses, 1, true)) {
                bowIndex = inCommandItem->argument[0].toInt();
                if (bowIndex > (getBowActuatorCount() - 1)) {
                    return eProcessResult::WrongArgumentValue;
                }
            } else {
                return eProcessResult::WrongArgumentCount;
            }
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 5)) { return eProcessResult::WrongArgumentCount; }
            setBowActuatorData(inCommandItem->argument[0].toInt(), inCommandItem->argument[1].toInt(), inCommandItem->argument[2].toInt(), inCommandItem->argument[3].toInt(), inCommandItem->argument[4]);
            bowIndex = inCommandItem->argument[0].toInt();
        }
        commandResponses->push_back({ "bad:" + String(bowIndex) + ":" + getBowActuatorFirstTouchPressure(bowIndex) + ":"  + getBowActuatorStallPressure(bowIndex) + ":" +
            getBowActuatorRestPosition(bowIndex) + ":" + getBowActuatorID(bowIndex), InfoRequest });
    }  else
    if (inCommandItem->command == "bowactuatorcount") {
        commandResponses->push_back({ "bac:" + String(getBowActuatorCount()), InfoRequest });
    } else
    if (inCommandItem->command == "bowactuatorremove") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (!removeBowActuator(inCommandItem->argument[0].toInt())) { return eProcessResult::CommandFailed; }
            commandResponses->push_back({"bar:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;

}


BowActuators::BowActuators(CalibrationData *t_calibrationDataConnect)
{
    m_calibrationDataConnect = t_calibrationDataConnect;
    m_currentBowActuator = 0;
    //m_bowActuator.push_back({0, 65535, 0, "default"});
    setBowActuatorData(0, 2000, 50000, 0, "default");
    loadBowActuator();
}

BowActuators::~BowActuators()
{
    //dtor
}


const serialCommandItem serialCommandsDCMotor[] = {
    { "bowmotorrun", "bmr", "1|0", "Set bow motor run on/off" },
    { "bowmotordirectpwm", "bmdp", "0-65535", "Bow motor direct power in 16-bit PWM values, requires that the PID is turned off" },
    { "bowmotorvoltage", "bmv", "float", "Bow motor voltage"},
    { "bowmotorcurrent", "bmc", "float", "Bow motor reported current use" },
    { "bowmotorcurrentlimit", "bmcl", "float", "Bow motor current limit (A)- !WARNING! Can ruin your instrument if changed" },
    { "bowmotorpowerlimit", "bmpl", "float", "Bow motor power limit (W) - !WARNING! Can ruin your instrument if changed" },
    { "bowmotorfrequency", "bmf", "-", "Bow motor reported frequency"},
    { "bowmotoremergencystop", "bmes", "ms (0-65535)", "Immediately stops the bowing motor and doesn't allow it to start again until the cool down period given in the first argument has lapsed (milliseconds)"},
    { "bowmotorpwmmin", "bmpi", "-", "Bow motor minimum PWM (for calibration)"},
    { "bowmotorpwmmax", "bmpx", "-", "Bow motor maximum PWM (for calibration)"}
};

eProcessResult DCMotorControl::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsDCMotor, sizeof(serialCommandsDCMotor) / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsDCMotor, sizeof(serialCommandsDCMotor) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "bowmotorrun") {
        if (request) {
            commandResponses->push_back({ "bmr:" + String(pRun), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setBowMotorRun(inCommandItem->argument[0].toInt());
            if (inCommandItem->argument[0].toInt() > 0) {
                run = 1;
                dcMotorControl->enableMotorPower();
            } else {
                run = 0;
                dcMotorControl->disableMotorPower();
            }
            commandResponses->push_back({"bmr:" + String(pRun), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotordirectpwm") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setSpeedPWM(inCommandItem->argument[0].toInt());
        }
        commandResponses->push_back({"bmdp:" + String(getSpeedPWM()), debugPrintType::InfoRequest});
    } else
    if (inCommandItem->command == "bowmotorvoltage") {
        if (request) {
            commandResponses->push_back({ "bmv:" + String(motorVoltage), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMotorVoltage(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"bmv:" + String(motorVoltage), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorcurrent") {
        if (request) {
            commandResponses->push_back({ "bmc:" + String(getMotorCurrent()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowmotorcurrentlimit") {
        if (request) {
            commandResponses->push_back({ "bmcl:" + String(motorCurrentLimit), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMotorMaxCurrent(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"bmcl" + String(motorCurrentLimit), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorpowerlimit") {
        if (request) {
            commandResponses->push_back({ "bmpl:" + String(motorPowerLimit), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMotorMaxPower(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"bmpl:" + String(motorPowerLimit), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorfrequency") {
        if (request) {
            commandResponses->push_back({ "bmf:" + String(getAverageTachometerFreq()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowmotoremergencystop") {
        if (request) {
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            emergencyDisable(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"bmes:" + inCommandItem->argument[0], InfoRequest});
            commandResponses->push_back({"EMERGENCY STOP! Cooling down for " + inCommandItem->argument[0] + " ms", debugPrintType::Error});
        }
    } else
    if (inCommandItem->command == "bowmotorpwmmin") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMinSpeedPWM(inCommandItem->argument[0].toInt());
        }
        commandResponses->push_back({ "bmpi:" + String(minSpeedPWM), InfoRequest });
    } else
    if (inCommandItem->command == "bowmotorpwmmax") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMaxSpeedPWM(inCommandItem->argument[0].toInt());
        }
        commandResponses->push_back({ "bmpx:" + String(maxSpeedPWM), InfoRequest });
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

eProcessResult DCMotorControl::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {
    return eProcessResult::NotFound;
}

String DCMotorControl::dumpData() {
    String dump = "";
    dump += "bmv:" + String(motorVoltage) + ",";
    dump += "bmcl:" + String(motorCurrentLimit) + ",";
    dump += "bmpl:" + String(motorPowerLimit) + ",";
    dump += "bmpi:" + String(minSpeedPWM) + ",";
    dump += "bmpx:" + String(maxSpeedPWM);
    return dump;
}

// *** POWER SUPPLY FUNCTIONS


const serialCommandItem serialCommandsPID[] = {
    { "bowpidtargetfreq", "bptf", "float", "Sets the PID target frequency" },
    { "bowpidki", "bpki", "float", "Sets the Ki parameter of the PID of the selected bow" },
    { "bowpidkp", "bpkp", "float", "Sets the Kp parameter of the PID of the selected bow" },
    { "bowpidkd", "bpkd", "float", "Sets the Kd parameter of the PID of the selected bow" },
    { "bowpidintegratorerror", "bpie", "float", "Sets the lower threshold of error values for the PID integrator to ignore of the selected bow" },
    { "bowpidr", "bpir", "-", "Resets the PID of the selected bow" },
    { "bowpidmaxerror", "bpme", "float", "Maximum error to correct in each PID loop, essentially sets acceleration" },
    { "bowpidpeakerror", "bpperr", "float", "Get latest PID peak error"},

    { "bowmotorspeedmax", "bmsx", "float", "Bow motor maximum speed limit" },
    { "bowmotorspeedmin", "bmsi", "float", "Bow motor minimum speed limit"},

    { "bowpidtargetfreq", "bcf", "float", "Legacy command conversion" },
    { "bowpidtargetfreq", "bowcontrolfrequency", "float", "Legacy command conversion" },
    { "bowpidtargetfreq", "bptf", "float", "Sets the PID target frequency" }
};

const serialCommandItem serialCommandsPIDHidden[] =  {
    { "bowmeasuretimetotarget", "bmtt", "float", "Measure the time it takes to change from the current frequency to the target frequency" }
};

eProcessResult PIDController::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsPID, sizeof(serialCommandsPID)  / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsPID, sizeof(serialCommandsPID) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "bowpidtargetfreq") {
        if (request) {
            commandResponses->push_back({ "bptf:" + String(pidTargetSpeed), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setPIDTarget(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({ "bptf:" + String(pidTargetSpeed), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowpidki") {
        if (request) {
            commandResponses->push_back({ "bpki:" + String(Ki), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            Ki = inCommandItem->argument[0].toFloat();
            commandResponses->push_back({"bpki:" + String(Ki), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidkp") {
        if (request) {
            commandResponses->push_back({ "bpkp:" + String(Kp), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            Kp = inCommandItem->argument[0].toFloat();
            commandResponses->push_back({"bpkp:" + String(Kp), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidkd") {
        if (request) {
            commandResponses->push_back({ "bpkd:" + String(Kd), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            Kd = inCommandItem->argument[0].toFloat();
            commandResponses->push_back({"bpkd:" + String(Kd), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidr") {
        if (!request) {
            commandResponses->push_back({"bowpidr:1", InfoRequest});
            previousError = 0;
            pidReset();
        }
    }  else
    if (inCommandItem->command == "bowpidintegratorerror") {
        if (request) {
            commandResponses->push_back({ "bpie:" + String(integratorIgnoreBelow), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            integratorIgnoreBelow = inCommandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            commandResponses->push_back({"bpie:" + String(integratorIgnoreBelow), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidmaxerror") {
        if (request) {
            commandResponses->push_back({ "bpme:" + String(pidMaxError), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            pidMaxError = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bpme:" + String(pidMaxError), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidpeakerror") {
        if (request) {
            commandResponses->push_back({ "bpperr:" + String(pidPeakError), InfoRequest });
            pidPeakError = 0;
        }
    } else
    if (inCommandItem->command == "bowmotorspeedmax") {
        if (request) {
            commandResponses->push_back({ "bmsx:" + String(maxSpeedHz), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMaxSpeedHz(inCommandItem->argument[0].toFloat()); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"bmsx:" + String(maxSpeedHz), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorspeedmin") {
        if (request) {
            commandResponses->push_back({ "bmsi:" + String(minSpeedHz), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMinSpeedHz(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"bmsi:" + String(minSpeedHz), InfoRequest});
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}


eProcessResult PIDController::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsPIDHidden, sizeof(serialCommandsPIDHidden)  / sizeof(serialCommandItem));

    if (inCommandItem->command == "bowmeasuretimetotarget") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            float startSpeed = pidTargetSpeed;
            float endSpeed = inCommandItem->argument[0].toFloat();
            float time = measureTimeToTarget(endSpeed);
            if (time == -1) {
                commandResponses->push_back({"bmtt:0", InfoRequest});
                return eProcessResult::CommandFailed;
            } else {
                commandResponses->push_back({"bmtt:" + String(time) + ":" + String(startSpeed) + ":" + String(endSpeed), InfoRequest});
            }
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}


String PIDController::dumpData() {
    String dump = "";
    dump += "bpki:" + String(Ki) + ",";
    dump += "bpkp:" + String(Kp) + ",";
    dump += "bpkd:" + String(Kd) + ",";
    dump += "bpie:" + String(integratorIgnoreBelow) + ",";
    dump += "bpme:" + String(pidMaxError) + ",";
    dump += "bmsx:" + String(maxSpeedHz) + ",";
    dump += "bmsi:" + String(minSpeedHz);

    return dump;
}


void PIDController::dir(std::vector<commandResponse> *commandResponses, String longPrefix, String shortPrefix) {
    longPrefix += prefix[0] + ".";
    shortPrefix += prefix[1] + ".";
    String longName, shortName;

    dirProto(commandResponses, longPrefix, shortPrefix, serialCommandsPID, sizeof(serialCommandsPID) / sizeof(serialCommandItem),
             serialCommandsPIDHidden, sizeof(serialCommandsPIDHidden) / sizeof(serialCommandItem));
}

const serialCommandItem serialCommandsBowPressure[] = {
    { "bowpressurebaseline", "bpb", "0-65535", "Bow pressure baseline, modulation is added to this point upward" },
    { "bowpressuremodifier", "bpm", "0-65535", "Bow pressure modulation, added to the baseline" },
    { "bowpressurerest", "bpr", "0|1", "Puts the bow pressure in the resting position (conditional)" },
    { "bowpressureengage", "bpe", "0|1", "Puts the bow pressure in the engage position (conditional)" },
    { "bowpressurepositionmax", "bppx", "0-65535", "Bow pressure stall/maximum position"},
    { "bowpressurepositionengage", "bppe", "0-65535", "Bow pressure touch/minimum position"},
    { "bowpressurepositionrest", "bppr", "0-65535", "Bow pressure rest position"},
    { "bowpressureengagespeed", "bpes", "1 - 100?", "Bow pressure movement speed when engaging or disengaging"},
    { "bowpressuremodulationspeed", "bpms", "0.1 - 10", "Bow pressure movement speed while engaged"},
    { "bowpressurehold", "bph", "0|1", "Sets bow hold on/off" },
    { "bowhome", "bh", "-", "Homing bow, used at startup and in case of the bow loosing position" },
};

const serialCommandItem serialCommandsBowPressureHidden[] = {
}


eProcessResult BowPressure::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsBowPressure, sizeof(serialCommandsBowPressure) / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsBowPressure, sizeof(serialCommandsBowPressure) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "bowpressurebaseline") {
        if (request) {
            commandResponses->push_back({ "bpb:" + String(baselinePressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setPressureBaseline(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"bpb:" + String(baselinePressure), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressuremodifier") {
        if (request) {
            commandResponses->push_back({ "bpm:" + String(modifierPressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setPressureModifier(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"bpm:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressurerest") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        rest(inCommandItem->argument[0].toInt());
        commandResponses->push_back({"bpr:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "bowpressureengage") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        engage(inCommandItem->argument[0].toInt());
        commandResponses->push_back({"bpe:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "bowpressurepositionmax") {
        if (request) {
            commandResponses->push_back({ "bppx:" + String(maxPressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            maxPressure = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bppx:" + String(maxPressure), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressurepositionengage") {
        if (request) {
            commandResponses->push_back({ "bppe:" + String(engagePressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            engagePressure = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bppe:" + String(engagePressure), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressurepositionrest") {
        if (request) {
            commandResponses->push_back({ "bppr:" + String(restPressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            restPressure = inCommandItem->argument[0].toInt();
            commandResponses->push_back({ "bppr:" + String(restPressure), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowpressureengagespeed") {
        if (request) {
            commandResponses->push_back({ "bpes:" + String(speedToEngage), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            uint16_t bes = inCommandItem->argument[0].toInt();
            if (bes > 100) { bes = 100; }
            speedToEngage = bes;
            commandResponses->push_back({"bpes:" + String(speedToEngage), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressuremodulationspeed") {
        if (request) {
            commandResponses->push_back({ "bpms:" + String(speedWhileEngaged), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            uint16_t bes = inCommandItem->argument[0].toInt();
            if (bes > speedWhileEngaged) { bes = speedToEngage; }
            speedWhileEngaged = bes;
            commandResponses->push_back({"bpms:" + String(speedWhileEngaged), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressurehold") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() > 0) { setHold(true); } else { setHold(false);}
        commandResponses->push_back({"bph:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "bowhome") {
        if (!request) {
            if (home()) {
                commandResponses->push_back({"bowhome:1", InfoRequest});
            } else {
                commandResponses->push_back({"bowhome:0", InfoRequest});
            }
        }
    } else {
        return eProcessResult::NotFound;
    }

    return eProcessResult::Ok;
}

eProcessResult BowPressure::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    return eProcessResult::NotFound;
}

String BowPressure::dumpData() {
    String dump = "";
    dump += "bpes:" + String(speedToEngage) + ",";
    dump += "bpms:" + String(speedWhileEngaged);
    return dump;
}


const serialCommandItem serialCommandsBowControl[] = {
    { "bowcontrolspeedmode", "bcsm", "0|1", "Bow motor speed mode, 0 = Automatic and 1 = Manual" },
    { "bowmotortimeout", "bmt", "ms(0-65535)", "Bow motor shutdown timeout after bow having been put into the rest position" },

    { "bowpid", "bpid", "1|0", "Sets the bow PID on/off" },
    { "bowmotorfaultcommands", "bmfc", "command list", "Commands to execute when a motor fault is tripped - !WARNING! Can ruin your instrument if changed" },
    { "bowmotoroverpowercommands", "bmopc", "command list", "Commands to execute when motor is over the power limit - !WARNING! Can ruin your instrument if changed" },
};

eProcessResult BowControl::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    eProcessResult processResult = eProcessResult::Ok;

    processCommandItems(inCommandItem, serialCommandsBowControl, sizeof(serialCommandsBowControl)  / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsBowControl, sizeof(serialCommandsBowControl) / sizeof(serialCommandItem), commandResponses,"");
    }

    if (((inCommandItem->command == "bowpressurerest") || (inCommandItem->command == "bpr")) && !request) {
        debugPrintln("Second rest", debugPrintType::Debug);
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() == 1) { rest(); }
    }

    if (inCommandItem->command == "bowpid") {
        if (request) {
            commandResponses->push_back({ "bpid:" + String(PIDon), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (inCommandItem->argument[0].toInt() > 0) { PIDon = true; } else { PIDon = false;}
            commandResponses->push_back({"bpid:" + String(PIDon), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowcontrolspeedmode") {
        if (request) {
            commandResponses->push_back({"bcsm:" + String(int(speedMode)), InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (!validateNumber(inCommandItem->argument[0].toInt(), 0, 1)) { return eProcessResult::WrongArgumentValue; }
            speedMode = (eSpeedMode) (inCommandItem->argument[0].toInt());
            commandResponses->push_back({"bcsm:" + String(int(speedMode)), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotordirectpwm") {
        if (request) {
            commandResponses->push_back({ "bmdp:" + String(manualMotorPWM), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            manualMotorPWM = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bmdp:" + String(manualMotorPWM), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotortimeout") {
        if (request) {
            commandResponses->push_back({ "bmt:" + String(bowShutoffTimeout), InfoRequest });
        } else {
            bowShutoffTimeout = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bmt:" + String(bowShutoffTimeout), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorfaultcommands") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            commandsMotorFault = delimitExpression(inCommandItem->argument[0], true);
        }
        commandResponses->push_back({ "bmfc:" + String(commandsMotorFault), InfoRequest });
    } else
    if (inCommandItem->command == "bowmotoroverpowercommands") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            commandsOverPowerCurrent = delimitExpression(inCommandItem->argument[0], true);
        }
        commandResponses->push_back({ "bmopc:" + String(commandsOverPowerCurrent), InfoRequest });
    } else {
        processResult = bowPressure->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = dcMotorControl->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = pidController->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    }

    return processResult;
}

eProcessResult BowControl::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    eProcessResult processResult = eProcessResult::NotFound;

    processResult = bowPressure->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

    processResult = dcMotorControl->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

    processResult = pidController->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }


    return eProcessResult::NotFound;
}

void BowControl::setBowSpeedPWM(uint16_t speed) {
    pidController->setPIDTarget(speed);
}


String BowControl::dumpData() {
    String dump = "";
    dump += "bmt:" + String(bowShutoffTimeout) + ",";
    dump += "bmfc:" + commandsMotorFault + ",";
    dump += "bmopc:" + commandsOverPowerCurrent + ",";
    dump += dcMotorControl->dumpData() + ",";
    dump += bowPressure->dumpData() + ",";
    dump += pidController->dumpData() + ",";
    return dump;
}


serialCommandItem serialCommandsControlReader[] = {
  { "adccommandmap", "acm", "channel:command string", "Sets the command string invoked when the value on ADC channel [channel] changes"},
  { "adcdefaults", "acd", "-" "Reverts all ADC command strings to default values" },
  { "adcread", "adcr", "channel:value", "Sent when a new value is presented on one of the ADC channels, cannot be invoked" },
  { "adcsettings", "adcs", "channel:averages:interrupterrorthreshold:continuouserrorthreshold:continuoustimeout", "Explain ADC settings here"}
};

serialCommandItem serialCommandsControlReaderHidden[] = {
  { "testadclatency", "tal", "0-65535", "Test ADC Latency" },
  { "testadclatencyreturn", "talr", "-", "Return from test" },
  { "testadcminmax", "tamm", "channel", "Measure min/max value for a given channel and resets the counter" }
};


eProcessResult ControlReader::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsControlReader, sizeof(serialCommandsControlReader)  / sizeof(serialCommandItem));

    eProcessResult processResult = eProcessResult::Ok;

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsControlReader, sizeof(serialCommandsControlReader) / sizeof(serialCommandItem), commandResponses,"");
        processResult = eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "adcread") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        int i = inCommandItem->argument[0].toInt();
        if ((i < 0) || (i > 7)) { return eProcessResult::WrongArgumentValue; }

        int32_t convertedValue;
        if (i < 5) {
            convertedValue = (int32_t) ((float) getData(i) * ((float) 65536 / 32767));
        } else {
            convertedValue = (int32_t) ((float) getData(i) * ((float) 65536 / 2048));
        }
        if (convertedValue > 65535) { convertedValue = 65535; }
        commandResponses->push_back({ "adcr:" + String(i) + ":" + String(convertedValue) + ":" + String(getData(i)), debugPrintType::InfoRequest});
    } else
    if (inCommandItem->command == "adccommandmap") {
        if (!checkArgumentsMin(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        uint8_t channel = inCommandItem->argument[0].toInt();

        if (request) {
            commandResponses->push_back({ "acm:" + String(channel) + ":" + delimitExpression(cvInputCommands[channel], true), InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
            setADCCommands(channel, stripQuotes(inCommandItem->argument[1]));
            commandResponses->push_back({ "acm:" + String(channel) + ":" + delimitExpression(cvInputCommands[channel], true), InfoRequest});
        }
    } else
    if (inCommandItem->command == "adcdefaults") {
        setDefaults();
        commandResponses->push_back({ "acd:1", debugPrintType::InfoRequest});
    } else
    if (inCommandItem->command == "adcsettings") {
        if (!checkArgumentsMin(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        uint8_t channel = inCommandItem->argument[0].toInt();
        if (channel > 7) { return eProcessResult::WrongArgumentValue; }

        if (request) {
            commandResponses->push_back({ "adcs:" + String(channel) + ":" + getADCAveragerSettings(channel), InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 5)) { return eProcessResult::WrongArgumentCount; }
            if (!setADCAveragerSettings(inCommandItem->argument[0].toInt(), inCommandItem->argument[1].toInt(), inCommandItem->argument[2].toInt(), inCommandItem->argument[3].toInt(),
                inCommandItem->argument[4].toInt())) { return eProcessResult::CommandFailed; }
            commandResponses->push_back({ "adcs:" + String(channel) + ":" + getADCAveragerSettings(channel), debugPrintType::InfoRequest});
        }
    } else {
        return eProcessResult::NotFound;
    }

    return processResult;
}

eProcessResult ControlReader::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsControlReaderHidden, sizeof(serialCommandsControlReaderHidden)  / sizeof(serialCommandItem));

    eProcessResult processResult = eProcessResult::Ok;

    if (inCommandItem->command == "helphidden") {
        addCommandHelp(serialCommandsControlReaderHidden, sizeof(serialCommandsControlReaderHidden) / sizeof(serialCommandItem), commandResponses,"");
        processResult = eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "testadclatency") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        testMeasurement = 0;
        testMeasurementOngoing = true;
        analogWrite(3, int(inCommandItem->argument[0].toInt()));
        commandResponses->push_back({"Starting ADC latency test", InfoRequest});
    } else
    if (inCommandItem->command == "testadclatencyreturn") {
        testMeasurementOngoing = false;
        commandResponses->push_back({"Returned from ADC latency test in " + String(testMeasurement) + " uS", InfoRequest});
    } else
    if (inCommandItem->command == "testadcminmax") {
        if (request) {
            commandResponses->push_back({ "tamm:" + String(testChannel) + ":" + String(testMin) + ":" + String(testMax), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            int testChannel = inCommandItem->argument[0].toInt();
            if ((testChannel < 0) || (testChannel > 7)) { return eProcessResult::WrongArgumentValue; }
            setADCMinMaxTestChannel(testChannel);
            commandResponses->push_back({ "Starting ADC min/max test on channel " + String(testChannel), InfoRequest });
        }
    } else {
        return eProcessResult::NotFound;
    }

    return processResult;
}
*/

