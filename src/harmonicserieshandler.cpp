#ifndef HARMONICSERIESHANDLER_C
#define HARMONICSERIESHANDLER_C

#include "harmonicserieshandler.hpp"

const ModuleCommandDeclaration HarmonicSeriesHandler::moduleCommands[] = {
    { "fundamental", "fu", "float", "Bow fundamental frequency, all harmonics are calculated from this number", false, true, &s_fundamental },
    { "harmonic", "h", "int", "Bow motor speed in terms of a harmonic number. A ratio is taken from the given harmonic in the current harmonic list, the ratio is \
        then multiplied by the bow fundamental frequency", false, false, &s_harmonic },
    { "harmonicadd", "ha", "int", "Additative version of bowcontrolharmonic, the number is given is added to the harmonic given", false, false, &s_harmonicAdd },
    { "harmonicbase", "hb", "int", "Same as bowcontrolharmonic but where the harmonic number is based on a MIDI note given by bowcontrolbasenote", false, false, &s_harmonicBase },
    { "basenote", "bn", "0-127", "Sets the MIDI base note of the string, used in conjunction with bowcontrolharmonicbase", false, true, &s_baseNote },
    { "shift", "sh", "-32767-32767", "Setting shift from the currently playing harmonic where 32767 equals the entire harmonic shift range shifted up", false, false, &s_shift },
    { "shiftrange", "sr", "0-36", "Set the number of harmonic numbers that constitutes the entire harmonic shift", false, true, &s_shiftRange },
    { "shift5", "sh5", "-32767-32767", "Setting shift from the currently playing harmonic over 5 octaves where 32767 equals 5 octaves shift up from the fundamental", false, false, &s_shift5 },

//    { "harmonicseries", "hs", "int", "Get/set the current harmonic series", false, true, nullptr },
//    { "bowharmonicseriessave", "bhss", "series:name", "Saves the current harmonic series in the slot given, or if the slot is out of range, creates a new one at the end of the list", false, false, nullptr },
    { "add", "a", "(name):(ratios)", "Add a new series with the given name and parameters", false, false, &s_add },
    { "remove", "rm", "series", "Remove the series given and shift any series accordingly. Cannot remove all series", false, false, &s_remove },
    { "count", "c", "-", "Returns the number of harmonic series in the list and their IDs", false, false, &s_count },
};

getModuleCount(HarmonicSeriesHandler)

float equalSeries[12] = { 1, 1.059463094, 1.122462048, 1.189207115, 1.25992105, 1.334839854, 1.414213562, 1.498307077, 1.587401052, 1.681792831, 1.781797436, 1.887748625 };
float justSeries[12] = {1, 1.06667, 1.125, 1.2, 1.25, 1.3333, 1.40625, 1.5, 1.6, 1.66667, 1.8, 1.875 };

HarmonicSeriesHandler::HarmonicSeriesHandler() {
    moduleID = new ModuleID("harmonicserieshandler", "hsh", "Harmonic series handler", ModuleID::software);
    HarmonicSeries tempSeries;
    ModuleGroup *group = addGroup(tempSeries.moduleID);
    group->mustHaveSelection = true;
    group->singleSelection = true;
    group->setIndexCallback(this, &s_harmonicSeriesIndexChanged);
    addHarmonicSeries("Just intonation", justSeries, 12);
    addHarmonicSeries("Equal temperament", equalSeries, 12);
    group->setSelection(0);
};

CREATE_INDEX_CALLBACK(harmonicSeriesIndexChanged, HarmonicSeriesHandler) {
    updateHarmonicData();
    return true;
}

CREATE_DATACHANGED_CALLBACK(dataChanged, HarmonicSeriesHandler) {
    debugPrintln("Data changed!", debugPrintType::Debug);
    return true;
}

CREATE_MODULE_COMMAND_FUNCTION(add, HarmonicSeriesHandler) {
    HarmonicSeries *lHarmonicSeries;

    switch(inCommandItem->argument.size()) {
    case 0:
        lHarmonicSeries = addHarmonicSeries("default just", justSeries, 12);
        break;
    case 1:
        lHarmonicSeries = addHarmonicSeries(delimitExpression(inCommandItem->argument[0], true), justSeries, 12);
        break;
    default:
        float newSeries[inCommandItem->argument.size() - 1];
        for (int i = 0; i < inCommandItem->argument.size() - 1; i++) {
            newSeries[i] = inCommandItem->argument[i + 1].toFloat();
        }
        lHarmonicSeries = addHarmonicSeries(delimitExpression(inCommandItem->argument[0], true), newSeries, (inCommandItem->argument.size() - 1));
    }

    String response = thisItem.shortCommand + ":" + delimitExpression(lHarmonicSeries->Id, true);
    for (int i = 0; i < lHarmonicSeries->ratios.size(); i++) {
        response += ":" + String(lHarmonicSeries->ratios[i]);
    }

    inCommandResponses->push_back({ response, InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(remove, HarmonicSeriesHandler) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }

    ModuleGroup *group = getGroup("harmonicseries");
    if (group == nullptr) { return eProcessResult::CommandFailed; }

    if (!request) {
//        harmonicSeries.erase(harmonicSeries.begin() + listNo);
        int listNo = inCommandItem->argument[0].toInt();
        if (!group->removeModule(listNo)) {
            return eProcessResult::CommandFailed;
        }
    }
    inCommandResponses->push_back({ thisItem.shortCommand, debugPrintType::InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(count, HarmonicSeriesHandler) {
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + getGroup("harmonicseries")->modules.size(), debugPrintType::InfoRequest });

    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(fundamental, HarmonicSeriesHandler) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(fundamentalFrequency), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        fundamentalFrequency = inCommandItem->argument[0].toFloat();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(fundamentalFrequency), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(harmonic, HarmonicSeriesHandler) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pHarmonic), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (!setHarmonic(inCommandItem->argument[0].toInt())) {
            inCommandResponses->push_back({"ERROR setting harmonic to " + String(inCommandItem->argument[0].toInt()), Error});
        } else {
            inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pHarmonic), InfoRequest });
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(harmonicAdd, HarmonicSeriesHandler) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pHarmonicAdd), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (!setHarmonicAdd(inCommandItem->argument[0].toInt())) {
            inCommandResponses->push_back({"ERROR setting harmonic add to " + String(inCommandItem->argument[0].toInt()), Error});
        } else {
            inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pHarmonicAdd), InfoRequest });
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(harmonicBase, HarmonicSeriesHandler) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pHarmonic + pBaseNote), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setHarmonic(inCommandItem->argument[0].toInt() - pBaseNote);
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pHarmonic + pBaseNote), InfoRequest });
        //inCommandResponses->push_back({ "bch:" + String(harmonic), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(baseNote, HarmonicSeriesHandler) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pBaseNote), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setBaseNote(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pBaseNote), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(shift, HarmonicSeriesHandler) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(harmonicShift), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setHarmonicShift(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(harmonicShift), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(shiftRange, HarmonicSeriesHandler) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(harmonicShiftRange), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setHarmonicShiftRange(inCommandItem->argument[0].toFloat());
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(harmonicShiftRange), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(shift5, HarmonicSeriesHandler) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(harmonicShift5), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setHarmonicShift5(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(harmonicShift5), InfoRequest });
    }
    return eProcessResult::Ok;
};

bool HarmonicSeriesHandler::calculateHarmonicShift() {
    //int octave = harmonicSeriesList.series[currentHarmonicSeries].ratio.size();
    ModuleGroup *group = getGroup("harmonicseries");
    if (group == nullptr) {
        return false;
    }

    HarmonicSeries *module = group->getSingleSelection();
    if (module == nullptr) {
        return false;
    }

    int octave = module->ratios.size();

    float freq = currentHarmonicFreq * pow(2, ((float) (((float) harmonicShiftRange) / octave) * harmonicShift / 32768 ));
    float freq5 = freq * pow(2, ((float) (((float) (5 * octave)) / octave) * harmonicShift5 / 32768 ));
//    if (outputDebugData) { debugPrintln("Current freq " + String(currentHarmonicFreq) + " shifted freq " + String(freq) + " shifted freq 5 octaves " + String(freq5), Debug); }
//    currentHarmonicShiftFreq = clamp(freq5, calibrationDataConnect->minHz, calibrationDataConnect->maxHz);
    currentHarmonicShiftFreq = freq5;
//    debugPrintln("Move the frequency min/max check to the receiving end", debugPrintType::Error);
//    if (currentHarmonicShiftFreq != freq5) { return false; }
    return true;
}

bool HarmonicSeriesHandler::setHarmonicShift(int inHarmonicShift) {
    harmonicShift = inHarmonicShift;
    if (!calculateHarmonicShift()) { return false; }
    if (!raiseFrequencyChanged()) { return false; };
    return true;
}

bool HarmonicSeriesHandler::setHarmonicShift5(int inHarmonicShift5) {
    harmonicShift5 = inHarmonicShift5;
    if (!calculateHarmonicShift()) { return false; }
    if (!raiseFrequencyChanged()) { return false; };
    return true;
}

int HarmonicSeriesHandler::getHarmonicShift() { return harmonicShift; }

int HarmonicSeriesHandler::getHarmonicShift5() { return harmonicShift5; }

bool HarmonicSeriesHandler::setHarmonicShiftRange(int inHarmonicShiftRange) {
    harmonicShiftRange = inHarmonicShiftRange;
    return true;
}

int HarmonicSeriesHandler::getHarmonicShiftRange() { return harmonicShiftRange; }

bool HarmonicSeriesHandler::setHarmonic(int _harmonic) {
    int __harmonic = clamp(_harmonic, lowerHarmonic, upperHarmonic);
    if (__harmonic != _harmonic) { return false; }
    pHarmonic = __harmonic;
    return updateHarmonicData();
}

int HarmonicSeriesHandler::getHarmonic() { return pHarmonic; }

bool HarmonicSeriesHandler::setHarmonicAdd(int _harmonic) {
    pHarmonicAdd = _harmonic;
    return updateHarmonicData();
}

int HarmonicSeriesHandler::getHarmonicAdd() { return pHarmonicAdd; }

bool HarmonicSeriesHandler::updateHarmonicData() {
    ModuleGroup *group = getGroup("harmonicseries");
    if (group == nullptr) {
        return false;
    }

    HarmonicSeries *module = group->getSingleSelection();
    if (module == nullptr) {
        return false;
    }

    int targetHarmonic = pHarmonic + pHarmonicAdd;

    int harmonicCount = module->ratios.size();
    if (harmonicCount == 0) {
        debugPrintln("Harmonic list empty!", debugPrintType::Error);
        return false;
    }
    // Calculate where in the series the current harmonic resides (0-11)
    int series = targetHarmonic % harmonicCount;
    if (series < 0) { series = harmonicCount + series; }
    // If harmonic is below 0 we need to reduce for the truncation to work properly
    if (targetHarmonic < 0) { targetHarmonic -= (harmonicCount - 1); }
    int octave = trunc(targetHarmonic / harmonicCount);

    float freq = fundamentalFrequency * pow(2, octave) * module->getHarmonic(series); // - 0.4;

    currentHarmonicFreq = freq;
    calculateHarmonicShift();

    return raiseFrequencyChanged();
}

bool HarmonicSeriesHandler::setBaseNote(int inBaseNote) {
    if ((inBaseNote < 0) || (inBaseNote > 127)) { return false; }
    pBaseNote = uint8_t (inBaseNote);
    return true;
}

bool HarmonicSeriesHandler::raiseFrequencyChanged() {
    frequencyChanged = true;
    return true;
}

bool HarmonicSeriesHandler::checkFrequencyChanged() {
    if (frequencyChanged) {
        frequencyChanged = false;
        return true;
    }
    return false;
}

HarmonicSeries* HarmonicSeriesHandler::addHarmonicSeries() {
    HarmonicSeries *lHarmonicSeries = new HarmonicSeries();
    ModuleGroup *group = addModule(lHarmonicSeries);
    Module *module = group->modules.back();
    module->setDataChangedCallback(this, &s_dataChanged);
    return module;
}

HarmonicSeries* HarmonicSeriesHandler::addHarmonicSeries(String id, float frequencies[], int size) {
    HarmonicSeries *hs = addHarmonicSeries(); //new HarmonicSeries();
    hs->Id = id;
    for (int i = 0; i < size; i++) {
        hs->setHarmonic(i, frequencies[i]);
    }
    return hs;
}

void HarmonicSeriesHandler::dumpData(std::vector<commandResponse> *inCommandResponses) {
//    std::vector<commandResponse> commandResponses;
//    commandResponses = ModuleHandler::dumpData();
//    commandResponses.push_back({ "hs[" + String(moduleGroups[0].selection[0]) + "]", debugPrintType::InfoRequest });
//    return commandResponses;
    ModuleHandler::dumpData(inCommandResponses);
    inCommandResponses->push_back({ "hs[" + String(moduleGroups[0].selection[0]) + "]", debugPrintType::InfoRequest });
}

#endif // HARMONICSERIESHANDLER_H
