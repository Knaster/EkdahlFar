#ifndef MUTECONTROL_H
#define MUTECONTROL_H

#include "../src/mute.cpp"
#include "tmc2209_servostepper.hpp"

class MuteControl : public Module {
public:
    SETMODULEID("mute", "mu", "Mute controller v1.0", eModuleType::hardware, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(setPosition, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(fullMute, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(halfMute, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(rest, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(saveFull, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(saveHalf, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(saveRest, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(fullMutePosition, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(halfMutePosition, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(restPosition, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(sustain, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(backoff, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(home, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(setHardwarePosition, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(completeTask, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(autoCorrect, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(homingSensed, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(moveDirection, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(currentStep, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(homingPoint, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(homingStage, MuteControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(tmcInfo, MuteControl)

    MuteControl(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensor);
/*
    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
*/
private:
    Mute *mute = nullptr;
public:
/***** Module specific commands mirroring serially attainable commands *****/
    bool setPosition(uint16_t position) { return mute->setPosition(position); };

    uint16_t getPosition() { return mute->getPosition(); };

    bool setFullMutePosition(uint16_t inFullMutePosition) { return mute->setFullMutePosition(inFullMutePosition);}

    uint16_t getFullMutePosition() { return mute->getFullMutePosition(); };

    bool setHalfMutePosition(uint16_t inHalfMutePosition) { return mute->setHalfMutePosition(inHalfMutePosition); };

    uint16_t getHalfMutePosition() { return mute->getHalfMutePosition(); };

    bool setRestPosition(uint16_t inRestPosition) { return mute->setRestPosition(inRestPosition); }

    uint16_t getRestPosition() { return mute->getRestPosition(); }

    bool home(bool invert = false) { return mute->tmc2209ServoStepper->home(invert); };

/***** Hidden commands for modular use *****/
    bool setHardwarePosition(uint16_t position) { return mute->tmc2209ServoStepper->stepServoStepper->setPosition(position); };

    bool completeTask(uint16_t timeout = 5000) { return mute->tmc2209ServoStepper->stepServoStepper->completeTask(timeout); };

    void setAutoCorrect(bool autoCorrect) { mute->tmc2209ServoStepper->stepServoStepper->autoCorrectPosition = autoCorrect; };

    bool getAutoCorrect() {return mute->tmc2209ServoStepper->stepServoStepper->autoCorrectPosition; };

    bool getHomingSensed() { return mute->tmc2209ServoStepper->stepServoStepper->getHomingSensed(); };

    eStepDirection getMoveDirection() { return mute->tmc2209ServoStepper->stepServoStepper->getMoveDirection(); };

    uint16_t getCurrentStep() { return mute->tmc2209ServoStepper->stepServoStepper->getCurrentStep(); };

    uint16_t getHomingPoint(uint8_t x, uint8_t y) { return mute->tmc2209ServoStepper->stepServoStepper->getHomingPoint(x,y); };

    eHomingStage getHomingStage() { return mute->tmc2209ServoStepper->stepServoStepper->getHomingStage(); };

    void getTMC2209Info() { mute->tmc2209ServoStepper->getTMC2209Info(); };

/***** Internal commands for stand-alone and semi-modular use *****/
    // Update function to be periodically called in order for backoff and other things to work
    void update() { mute->updateMute(); };
    // Handles time-critical timing of stepper motor update - automatically called through stepInervalCallback - DO NOT MANUALLY CALL THIS
    void updateServo() { mute->tmc2209ServoStepper->stepServoStepper->updatePosition(); };
    // This is to be set to a static function that in turn will call the class instance of updateMuteServo
    void setStepIntervalCallback(void *stepIntervalCallback) { mute->tmc2209ServoStepper->stepServoStepper->stepIntervalCallback = stepIntervalCallback; };

/***** Internal commands for debugging use, most likely to be removed *****/
//    void setStepperID(uint16_t stepperID) { mute->tmc2209ServoStepper->stepServoStepper->stepperID = stepperID; };
};

#endif

