#ifndef FARSINGLE_C
#define FARSINGLE_C

#include "farsingle.hpp"

FARSingle::FARSingle(void *muteStepperCallback, void *pressureStepperCallback, void *tachometerCallback, void *pidCallback) {
    muteControl = new MuteControl(-1, 5, 4, &Serial5, 13);
    hammerControl = new Solenoid(3);
    bowControl = new BowControl(2, 14, 15,12, 23, 11, -1, 10, 9, &Serial2,6);
    harmonicSeriesHandler = new HarmonicSeriesHandler();
    bowActuators = new BowActuators(bowControl->getBowPressureReference());

    calibrateBow = new CalibrateBow(bowControl);
    calibrateMute = new CalibrateMute(*muteControl, *bowControl, *harmonicSeriesHandler);

    bowControl->enableBowMotorPower();
    bowControl->getTMC2209Info();
    muteControl->getTMC2209Info();
    bowControl->home();
    muteControl->home();

    float equalSeries[12] = { 1, 1.059463094, 1.122462048, 1.189207115, 1.25992105, 1.334839854, 1.414213562, 1.498307077, 1.587401052, 1.681792831, 1.781797436, 1.887748625 };
    float justSeries[12] = {1, 1.06667, 1.125, 1.2, 1.25, 1.3333, 1.40625, 1.5, 1.6, 1.66667, 1.8, 1.875 };


    harmonicSeriesHandler->addHarmonicSeries("\"Just intonation\"", justSeries);
    harmonicSeriesHandler->addHarmonicSeries("\"Equal temperament\"", equalSeries);

    commands->addCommands("bhs:0");

    controlReader = new ControlReader(17, 16);

    muteControl->setStepIntervalCallback(muteStepperCallback);
    bowControl->setStepIntervalCallback(pressureStepperCallback);
    attachInterrupt(digitalPinToInterrupt(12), tachometerCallback, CHANGE);
    pidInterrupt0.begin(pidCallback, bowControl->getPIDUpdateInterval());
}

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
        if (processResult != eProcessResult::NotFound) { return processResult; }

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

    return eProcessResult::NotFound;
}

String FARSingle::dumpData() {
    String dump = "";
    dump += muteControl->dumpData();
    dump += hammerControl->dumpData();
    dump += bowControl->dumpData();
    dump += bowActuators->dumpData();
    dump += harmonicSeriesHandler->dumpData();
    return dump;
}

void FARSingle::updateControlReader() {
    controlReader->readData();
}

void FARSingle::update() {
    if (harmonicSeriesHandler->checkFrequencyChanged()) {
        bowControl->setBowSpeedHZ(harmonicSeriesHandler->getCalculatedFrequency());
    }
    hammerControl->update();
    bowControl->update();
    muteControl->update();
}

#endif // FARSINGLE_C
