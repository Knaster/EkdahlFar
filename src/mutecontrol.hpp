#ifndef MUTECONTROL_H
#define MUTECONTROL_H

#include "mute.cpp"
#include "tmc2209_servostepper.hpp"

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

class MuteControl {
public:
    MuteControl(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensor);

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
    String dumpData();

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

