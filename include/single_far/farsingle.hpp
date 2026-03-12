#ifndef FARSINGLE_H
#define FARSINGLE_H

#include <base/modulehandler.hpp>
#include <master_controller/midiconfigurationhandler.hpp>
#include <mute/mutecontrol.hpp>
#include <solenoid/solenoid.hpp>
#include <bow/bowcontrol.hpp>
#include <master_controller/midi.h>
#include <mute/mutecalibration.hpp>
#include <bow/bowcalibration.hpp>
#include <controlbox/controlReader.hpp>
#include <plugins/pluginhandler.hpp>

#define FAR_SINGLE_CREATE_INSTANCE(instance) \
    FARSingle *instance; \
    void instance##UpdateServoStepperPressure0() { instance->updatePressureServo(); } \
    void instance##UpdateServoStepperMute0() { instance->updateMuteServo(); } \
    void instance##UpdateTachometer0() { instance->updateTachometer(); } \
    void instance##UpdatePID0() { instance->updatePID(); } \

class FARSingle : public ModuleHandler {
public:
    SETMODULEID("", "", "FAR 1.1", eModuleType::hardware, true)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(calibrateAll, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(calibrateBowSpeed, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(calibrateBowPressure, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(calibrateMute, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(pickupStringFrequency, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(pickupAudioPeak, FARSingle)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(pickupAudioRMS, FARSingle)

public:
    FARSingle(void (*muteStepperCallback)(), void (*pressureStepperCallback)(), void (*tachometerCallback)(), void (*pidCallback)());

    MIDIConfigurationHandler *midiConfigurationHandler;

    ExpressionParser *expressionParser = nullptr;
private:
    MuteControl *muteControl = nullptr;
    Solenoid *hammerControl = nullptr;
    BowControl *bowControl = nullptr;
    ControlReader *controlReader = nullptr;
    MIDIHandler *midiHandler = nullptr;

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
