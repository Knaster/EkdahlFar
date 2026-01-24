#ifndef FARSINGLE_H
#define FARSINGLE_H

#include "../src/midimessageconfiguration.cpp"
#include "../src/midiconfigurationhandler.cpp"

#include "module.hpp"

#include "../src/audioanalyze.h"
#include "../src/midi.cpp"

#include "../src/debugprint.hpp"

#include "../src/mutecontrol.cpp"
#include "../src/solenoid.cpp"
#include "../src/bowcontrol.cpp"
#include "../src/bowActuators.cpp"
#include "../src/harmonicserieshandler.cpp"
#include "../src/controlReader.cpp"

#include "../src/calibrationhelpers.hpp"

#include "../src/bowcalibration.cpp"
#include "../src/mutecalibration.cpp"

#define FAR_SINGLE_CREATE_INSTANCE(instance) \
    FARSingle *instance; \
    void instance##UpdateServoStepperPressure0() { instance->updatePressureServo(); } \
    void instance##UpdateServoStepperMute0() { instance->updateMuteServo(); } \
    void instance##UpdateTachometer0() { instance->updateTachometer(); } \
    void instance##UpdatePID0() { instance->updatePID(); } \

class FARSingle : public ModuleHandler {
public:
    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(calibrateAll, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(calibrateBowSpeed, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(calibrateBowPressure, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(calibrateMute, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(pickupStringFrequency, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(pickupAudioPeak, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(pickupAudioRMS, FARSingle)

public:
    FARSingle(void *muteStepperCallback, void *pressureStepperCallback, void *tachometerCallback, void *pidCallback);

    MIDIConfigurationHandler *midiConfigurationHandler;
//    void *midiConfigurationHandler = nullptr;

private:
    MuteControl *muteControl = nullptr;
    Solenoid *hammerControl = nullptr;
    BowControl *bowControl = nullptr;
    ControlReader *controlReader = nullptr;
//    void *controlReader = nullptr;

    CalibrateBow *calibrateBow = nullptr;
    CalibrateMute *pCalibrateMute = nullptr;

    IntervalTimer pidInterrupt0;
public:
    void initFAR();

    void updateControlReader(std::vector<commandResponse> *inCommandResponses);

    void update() override;

    void updatePressureServo() { bowControl->updateServo(); }
    void updateMuteServo() { muteControl->updateServo(); }
    void updateTachometer() { bowControl->updateTachometer(); }
    void updatePID() { bowControl->updatePID(); }
};

#endif // FARSINGLE_C
