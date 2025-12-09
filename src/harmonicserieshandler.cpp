#ifndef HARMONICSERIESHANDLER_C
#define HARMONICSERIESHANDLER_C

#include "harmonicserieshandler.hpp"

HarmonicSeriesHandler::HarmonicSeriesHandler() {};

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

bool HarmonicSeriesHandler::calculateHarmonicShift() {
    int octave = harmonicSeriesList.series[currentHarmonicSeries].ratio.size();

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
    harmonic = __harmonic;
    return updateHarmonicData();
}

int HarmonicSeriesHandler::getHarmonic() { return harmonic; }

bool HarmonicSeriesHandler::setHarmonicAdd(int _harmonic) {
    harmonicAdd = _harmonic;
    return updateHarmonicData();
}

int HarmonicSeriesHandler::getHarmonicAdd() { return harmonicAdd; }

bool HarmonicSeriesHandler::updateHarmonicData() {
    int targetHarmonic = harmonic + harmonicAdd;

    int harmonicCount = harmonicSeriesList.series[currentHarmonicSeries].ratio.size();
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

    float freq = fundamentalFrequency * pow(2, octave) * harmonicSeriesList.series[currentHarmonicSeries].ratio[series]; // - 0.4;

    currentHarmonicFreq = freq;
    calculateHarmonicShift();

    return raiseFrequencyChanged();
}

bool HarmonicSeriesHandler::setBaseNote(int inBaseNote) {
    if ((inBaseNote < 0) || (inBaseNote > 127)) { return false; }
    baseNote = uint8_t (inBaseNote);
    return true;
}

bool HarmonicSeriesHandler::loadHarmonicSeries(int i) {
    if ((i > (harmonicSeriesList.series.size() - 1)) || (i < 0)) {
        return false;
    }
    currentHarmonicSeries = i;
    harmonicSeriesList.series[currentHarmonicSeries] = harmonicSeriesList.series[i];
    return true;
};

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

String HarmonicSeriesHandler::dumpData() {
    String dump = "";
    dump += "bcu:" + String(fundamentalFrequency) + ",";
    dump += "bchbn:" + String(baseNote) + ",";
    dump += "bchsr:" + String(harmonicShiftRange) + ",";
    dump += "bhs:" + String(currentHarmonicSeries) + ",";
    dump += harmonicSeriesList.dumpData();
    return dump;
}

#endif // HARMONICSERIESHANDLER_H
