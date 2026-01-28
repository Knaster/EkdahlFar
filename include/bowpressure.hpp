#ifndef BOWPRESSURE_H
#define BOWPRESSURE_H

#include "../src/tmc2209_servostepper.cpp"

enum ePressureMode { Rest, Engage };

class BowPressure : public ModuleHandler {
public:
    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(baseline, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(modifier, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(rest, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(engage, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(stallPressure, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(engagePressure, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(restPressure, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(engageSpeed, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(modulationSpeed, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(hold, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(home, BowPressure)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(tmcinfo, BowPressure)

private:
    BowActuators *bowActuators = nullptr;

    uint16_t lastPressure = 0;
    Tmc2209ServoStepper *tmc2209ServoStepper = nullptr;

    uint16_t baselinePressure = 0;             ///< The baseline pressure value used when engaging the bow with the string
    uint16_t modifierPressure = 0;             ///< The modifier pressure value used in conjunction with the baselinePressure when engaing the bow with the string
    ePressureMode pressureMode = ePressureMode::Rest;

    float speedToEngage = 10;
    float speedWhileEngaged = 10;
    bool reachedEngage = false;

    bool pHold = false;

    bool restSignal = false;
public:

    BowPressure(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin);
/*
    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);
    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
*/
    void setRestSignal() { restSignal = true; }
    bool getRestSignal();

/***** Module specific commands mirroring serially attainable commands *****/
    void setPressureSafe(uint16_t pressure);

    void setPressureBaseline(uint16_t baseline);

    uint16_t getPressureBaseline();

    void setPressureModifier(uint16_t modifier);

    void setHold(bool inHold);

    bool rest(bool enact);

    bool engage(bool enact);

    bool setStallPressure(uint16_t inStallPressure);

    uint16_t getStallPressure();

    bool setEngagePressure(uint16_t inEngagePressure);

    uint16_t getEngagePressure();

    bool setRestPressure(uint16_t inRestPressure);

    uint16_t getRestPressure();

    bool getHold() { return pHold; }

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
    BowActuator* currentActuator();
};
#endif // BOWPRESSURE_H


