#ifndef BOWPRESSURE_H
#define BOWPRESSURE_H

#include "tmc2209_servostepper.cpp"

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
/*
const serialCommandItem serialCommandsBowPressureHidden[] = {
}
*/
enum ePressureMode { Rest, Engage };

class BowPressure {
private:
    uint16_t lastPressure = 0;
    Tmc2209ServoStepper *tmc2209ServoStepper = nullptr;

    uint16_t baselinePressure = 0;             ///< The baseline pressure value used when engaging the bow with the string
    uint16_t modifierPressure = 0;             ///< The modifier pressure value used in conjunction with the baselinePressure when engaing the bow with the string
    ePressureMode pressureMode = ePressureMode::Rest;

    uint16_t maxPressure = 65535;
    uint16_t engagePressure = 0;
    uint16_t restPressure = 0;

    float speedToEngage = 10;
    float speedWhileEngaged = 10;
    bool reachedEngage = false;

    bool hold = false;
public:
    BowPressure(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin);

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);
    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
    String dumpData();

/***** Module specific commands mirroring serially attainable commands *****/
    void setPressureSafe(uint16_t pressure);

    void setPressureBaseline(uint16_t baseline);

    uint16_t getPressureBaseline();

    void setPressureModifier(uint16_t modifier);

    void setHold(bool inHold);

    bool rest(bool enact);

    bool engage(bool enact);

    void setMaxPressure(uint16_t inMaxPressure) { maxPressure = inMaxPressure; }

    uint16_t getMaxPressure() { return maxPressure; }

    void setEngagePressure(uint16_t inEngagePressure) { engagePressure = inEngagePressure; }

    uint16_t getEngagePressure() { return engagePressure; }

    void setRestPressure(uint16_t inRestPressure) { restPressure = inRestPressure; }

    uint16_t getRestPressure() { return restPressure; }

    bool getHold() { return hold; }

    ePressureMode getPressureMode() { return pressureMode; }

    bool home(bool invert = false) { return tmc2209ServoStepper->home(invert); }

/***** Hidden serial commands *****/
    void setHardwarePressure(uint16_t pressure);

    void getTMC2209Info();

    bool completeTask(uint16_t timeout = 3000) { return tmc2209ServoStepper->stepServoStepper->completeTask(timeout); }

    void setAutoCorrect(bool autoCorrect) { tmc2209ServoStepper->stepServoStepper->autoCorrectPosition = autoCorrect; }

    bool getAutoCorrect() {return tmc2209ServoStepper->stepServoStepper->autoCorrectPosition; };

    void setSpeed(float speed) { tmc2209ServoStepper->stepServoStepper->setSpeed(speed); }

    float getSpeed() { return tmc2209ServoStepper->stepServoStepper->getSpeed();}

/***** Internal commands for stand-alone and semi-modular use *****/
    void update();

    // Handles time-critical timing of stepper motor update - automatically called through stepInervalCallback - DO NOT MANUALLY CALL THIS
    void updateServo() { tmc2209ServoStepper->stepServoStepper->updatePosition(); };

    // This is to be set to a static function that in turn will call the class instance of updateMuteServo
    void setStepIntervalCallback(void *stepIntervalCallback) { tmc2209ServoStepper->stepServoStepper->stepIntervalCallback = stepIntervalCallback; };

/***** Internal commands for debugging use, most likely to be removed *****/
//    void setStepperID(uint16_t stepperID);

/***** Private commands *****/
private:
    bool calculateBaselineModifierPressure();
};
#endif // BOWPRESSURE_H


