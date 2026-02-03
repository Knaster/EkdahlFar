#ifndef FARSINGLE_C
#define FARSINGLE_C

#include "farsingle.hpp"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

CREATE_MIDI_SOURCE_CALLBACKS(MIDI)
CREATE_MIDI_SOURCE_CALLBACKS(usbMIDI)

const ModuleCommandDeclaration FARSingle::moduleCommands[] = {
    { "calibrateall", "ca", "-", "Performs all calibration routines on the selected bow, see below for routines performed", false, false, &s_calibrateAll, eCommandType::Immediate },
    { "calibratebowspeed", "cbs", "-", "Finds the minimum and maximum bow speed of the selected bow", false, false, &s_calibrateBowSpeed, eCommandType::Immediate },
    { "calibratebowpressure", "cbp", "-", "Finds the minimum and maximum bow pressure of the selected bow", false, false, &s_calibrateBowPressure, eCommandType::Immediate },
    { "calibratemute", "cmu", "-", "Calibrate mute settings", false, false, &s_calibrateMute, eCommandType::Immediate },
    { "pickupstringfrequency", "psf", "-", "Returns the fundamental tone calculated from the current audio signal if appliccable", false, false, &s_pickupStringFrequency, eCommandType::Hertz },
    { "pickupaudiopeak", "pap", "-", "Returns the peak amplitude of the current audio signal", false, false, &s_pickupAudioPeak, eCommandType::SimpleInt16 },
    { "pickupaudiorms", "par", "-", "Returns the RMS amplitude of the current audio signal", false, false, &s_pickupAudioRMS, eCommandType::SimpleInt16 }
};

FARSingle::FARSingle(void *muteStepperCallback, void *pressureStepperCallback, void *tachometerCallback, void *pidCallback) {
//    moduleID = new ModuleID("", "", "FAR 1.1", eModuleType::hardware, true);
    muteControl = new MuteControl(-1, 5, 4, &Serial5, 13);

    hammerControl = new Solenoid(3);
    bowControl = new BowControl(2, 14, 15,12, 23, 11, -1, 10, 9, &Serial2,6);

    calibrateBow = new CalibrateBow(bowControl);
    pCalibrateMute = new CalibrateMute(*muteControl, *bowControl, *(bowControl->harmonicSeriesHandler));

    midiConfigurationHandler = new MIDIConfigurationHandler();
    MIDIsource = midiConfigurationHandler->addMIDISource(&MIDI);
    CONNECT_MIDI_CALLBACKS(MIDIsource, MIDI)
    usbMIDIsource = midiConfigurationHandler->addMIDISource(&usbMIDI);
    CONNECT_USBMIDI_CALLBACKS(usbMIDIsource, usbMIDI)

    bowControl->enableBowMotorPower();
    bowControl->getTMC2209Info();
    muteControl->getTMC2209Info();

    controlReader = new ControlReader(17, 16);

    muteControl->setStepIntervalCallback(muteStepperCallback);
    bowControl->setStepIntervalCallback(pressureStepperCallback);
    attachInterrupt(digitalPinToInterrupt(12), tachometerCallback, CHANGE);
    pidInterrupt0.begin(pidCallback, bowControl->getPIDUpdateInterval());

    startAudioAnalyze();

    addModule(hammerControl);
    addModule(bowControl);
    addModule(midiConfigurationHandler);
    addModule(muteControl);
    addModule(controlReader);
}

getModuleCount(FARSingle)

CREATE_MODULE_COMMAND_FUNCTION(calibrateAll, FARSingle) {
    if (calibrateBow == nullptr) { return eProcessResult::CommandFailed; }
    if (!request) {
        if (calibrateBow->calibrateAll()) {
            inCommandResponses->push_back({ thisItem.shortCommand + ":1", InfoRequest });
            return eProcessResult::Ok;
        } else {
            inCommandResponses->push_back({ thisItem.shortCommand + ":0", InfoRequest });
            return eProcessResult::CommandFailed;
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(calibrateBowSpeed, FARSingle) {
    eCalibrationResult calibrationResult;
    if (calibrateBow == nullptr) { return eProcessResult::CommandFailed; }
    if (!request) {
        calibrationResult = calibrateBow->findMinMaxSpeedPWM();
        if (calibrationResult == CR_Ok) {
            inCommandResponses->push_back({ thisItem.shortCommand + ":1", InfoRequest });
            return eProcessResult::Ok;
        } else {
            exitWithError(calibrationResult);
            inCommandResponses->push_back({ thisItem.shortCommand + ":0", InfoRequest });
            return eProcessResult::CommandFailed;
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(calibrateBowPressure, FARSingle) {
    eCalibrationResult calibrationResult;
    if (calibrateBow == nullptr) { return eProcessResult::CommandFailed; }
    if (!request) {
        calibrationResult = calibrateBow->findMinMaxPressure();
        if (calibrationResult == CR_Ok) {
            inCommandResponses->push_back({ thisItem.shortCommand + ":1", InfoRequest });
            return eProcessResult::Ok;
        } else {
            exitWithError(calibrationResult);
            inCommandResponses->push_back({ thisItem.shortCommand + ":0", InfoRequest });
            return eProcessResult::CommandFailed;
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(calibrateMute, FARSingle) {
    eCalibrationResult calibrationResult;
    if (pCalibrateMute == nullptr) { return eProcessResult::CommandFailed; }
    if (!request) {
        calibrationResult = pCalibrateMute->calibrateAll();
        if (calibrationResult == CR_Ok) {
            inCommandResponses->push_back({ thisItem.shortCommand + ":1", InfoRequest });
            return eProcessResult::Ok;
        } else {
            exitWithError(calibrationResult);
            inCommandResponses->push_back({ thisItem.shortCommand + ":0", InfoRequest });
            return eProcessResult::CommandFailed;
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(pickupStringFrequency, FARSingle) {
    if (audioFrequencyAvaliable()) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(audioFrequency()), InfoRequest });
    } else {
        inCommandResponses->push_back({ thisItem.shortCommand + ":0", InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(pickupAudioPeak, FARSingle) {
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(audioPeakAmplitude()), InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(pickupAudioRMS, FARSingle) {
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(audioRMSAmplitude()), InfoRequest });
    return eProcessResult::Ok;
};

void FARSingle::initFAR() {
    bowControl->home();
    muteControl->home();
}

void FARSingle::updateControlReader(std::vector<commandResponse> *inCommandResponses) {
    controlReader->readData(inCommandResponses);
}

void FARSingle::update() {
    if (midiConfigurationHandler != nullptr) { midiConfigurationHandler->update(); }
    if (bowControl != nullptr) {
        if (bowControl->harmonicSeriesHandler != nullptr) {
            if (bowControl->harmonicSeriesHandler->checkFrequencyChanged()) {
                bowControl->setBowSpeedHZ(bowControl->harmonicSeriesHandler->getCalculatedFrequency());
            }
        }
        bowControl->update();
    }
    if (hammerControl != nullptr) { hammerControl->update(); }
    if (muteControl != nullptr) { muteControl->update(); }
}

#endif // FARSINGLE_C
