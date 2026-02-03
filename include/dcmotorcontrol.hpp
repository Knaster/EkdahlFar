#ifndef DCMOTORCONTROL_H
#define DCMOTORCONTROL_H

#define BITDEPTH 16
#define BITDIV 1 // = 2^(16-BITDEPTH)

#define DCDCLOWERBOUND 1.2
#define DCDCUPPERBOUND 9
class DCMotorControl : public Module {
public:
    SETMODULEID("dcmotor", "dcm", "DC Motor controller v1.0", eModuleType::hardware, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(run, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(pwm, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(voltage, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(current, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(currentLimit, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(powerLimit, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(frequency, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(emergencyStop, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(minPWM, DCMotorControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(maxPWM, DCMotorControl)

    DCMotorControl(char inMotorRevPin, char inMotorVoltagePin, char inMotorDCDCEnPin, char inTachoPin, char inCurrentSensePin, char inMotorFaultPin);

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);

private:
    Teensy_PWM* motorPWM;

    Teensy_PWM *motorVoltagePWM;

    #define tachoFreqLength 10                          ///< Sets the amount of tachometer values to store in the buffer

    double tachoFreq[tachoFreqLength];                  ///< Tachometer buffer, used as a circular storage point of tachometer values for averaging

    int tachoFreqCount = 0;                             ///< Number of tachometer values stored, this will only be lower than tachoFreqLength at the very beginning of the program running

    int tachoFreqIndex = 0;                             ///< Current tachometer index

    #define tachoFreqPerimssibleMIN 10                  ///< Minimum tachometer value acceptable, given in Hertz

    #define tachoFreqPerimssibleMAX 100000              ///< Maximum tachometer value acceptable, given in Hertz

    #define permissibleFreqDeviation 10                 ///< Maximum permissible deviation from median value when averaging tachometer values, given in Hertz

    float tachoOldAverage = 0;                          ///< Holds the last calculate tachometer average, given in Hertz

    char lastReflectorISRState;                         ///< Hold the last reflector output state

    long reflectorCyclePeriod = 0;                      ///< Holds the current tachometer cycle value in micro seconds

    elapsedMicros reflectorCounter;                     ///< Local counter for the bow speed tachometer

    unsigned long reflectorZeroTimeoutValue = 50000;    ///< Threshold at which the bow movement is considered zero, given in uS (was 500,000 for 0.5Hz, now 50,000 for 20Hz)

    uint16_t lastMotorPWM = 0;                          ///< Holds the last PWM speed value sent to the bowing wheel

    uint16_t minSpeedPWM = 0;

    uint16_t maxSpeedPWM = 65535;

    float motorCurrentLimit = 1.75;                     ///< Limit for tripping bow current draw error

    float motorPowerLimit = 12.84;

    char motorRevPin;                                   ///< Pin for motor driver reverse PWM

    char motorFaultPin;                                 ///< Pin for motor driver ~FAULT signal

    uint8_t currentSensePin;                            ///< Pin for motor current sense

    bool overPowerFlag = false;       // External flag to show a over-power event has happend, taking into account the duration of the event etc.

    bool transientOverPower = false;  // Set to true at first over-power event, cleared if bow is not over power

    elapsedMillis lastOverPowerEvent;

    uint32_t overPowerDuration = 100; // If bow over power events are going past the duration (in ms), signal the over power event

    uint32_t emergencyCoolDownEvent;

    uint16_t emergencyCoolDownPeriod = 1000;

    bool emergencyCoolDown = false;

    uint8_t motorDCDCEnPin;

    bool motorDCDCEn = false;

    uint8_t motorVoltagePin;

    float motorVoltage = 6.6;

    bool pRun = false;

public:
    char reflectorInterruptPin;             ///< Pin for reflection sensor output

private:
    void addTachoFreq(float freq);

public:
/***** Module specific commands mirroring serially attainable commands *****/

    void setBowMotorRun(bool inRun);

    bool getBowMotorRun() { return pRun; }

    bool disableMotorPower();

    bool enableMotorPower();

    void setSpeedPWMSafe(uint16_t speed);

    void setSpeedPWM(uint16_t speed);

    uint16_t getSpeedPWM();

    uint16_t getMinSpeedPWM();

    uint16_t getMaxSpeedPWM();

    float getLastTachometerFreq();

    void clearTachometerData();

    float getAverageTachometerFreq();

    void tachometerISRHandler();

    float getMotorMaxPower();

    float getMotorVoltage();

    float getMotorCurrent();

    bool setMotorVoltage(float voltage);

    void setMotorMaxPower(float watts);

    void setMotorMaxCurrent(float amps);

    float getMotorMaxCurrent();

    bool isOverCurrent();

    bool isOverPower();

    bool getMotorFault();

    bool emergencyDisable(uint16_t coolDown);

/***** Hidden commands for modular use *****/

    void setMinSpeedPWM(uint16_t inSpeed);

    void setMaxSpeedPWM(uint16_t inSpeed);

    bool checkTachometerTimeout();

/***** Internal commands for stand-alone and semi-modular use *****/

};

#endif
