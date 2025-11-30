#ifndef DCMOTORCONTROL_H
#define DCMOTORCONTROL_H

#define BITDEPTH 16
#define BITDIV 1 // = 2^(16-BITDEPTH)

#define DCDCLOWERBOUND 1.2
#define DCDCUPPERBOUND 9

class DCMotorControl {
private:
    Teensy_PWM* motorPWM;

    #define tachoFreqLength 10              ///< Sets the amount of tachometer values to store in the buffer
    double tachoFreq[tachoFreqLength];      ///< Tachometer buffer, used as a circular storage point of tachometer values for averaging
    int tachoFreqCount = 0;                 ///< Number of tachometer values stored, this will only be lower than tachoFreqLength at the very beginning of the program running
    int tachoFreqIndex = 0;                 ///< Current tachometer index
    #define tachoFreqPerimssibleMIN 10      ///< Minimum tachometer value acceptable, given in Hertz
    #define tachoFreqPerimssibleMAX 100000  ///< Maximum tachometer value acceptable, given in Hertz

    #define permissibleFreqDeviation 10     ///< Maximum permissible deviation from median value when averaging tachometer values, given in Hertz
    float tachoOldAverage = 0;              ///< Holds the last calculate tachometer average, given in Hertz

    char lastReflectorISRState;             ///< Hold the last reflector output state
    long reflectorCyclePeriod = 0;          ///< Holds the current tachometer cycle value in micro seconds
    elapsedMicros reflectorCounter;         ///< Local counter for the bow speed tachometer

    unsigned long reflectorZeroTimeoutValue = 50000; ///< Threshold at which the bow movement is considered zero, given in uS (was 500,000 for 0.5Hz, now 50,000 for 20Hz)

    uint16_t lastMotorPWM = 0;                   ///< Holds the last PWM speed value sent to the bowing wheel

public:
    char motorRevPin;                    ///< Pin for motor driver reverse PWM
    char reflectorInterruptPin;             ///< Pin for reflection sensor output
    char motorFaultPin;                  ///< Pin for motor driver ~FAULT signal
    uint8_t currentSensePin;             ///< Pin for motor current sense

    float motorCurrentLimit = 1.75;       ///< Limit for tripping bow current draw error
    float motorWattage = 12.84;

    bool overPowerFlag = false;       // External flag to show a over-power event has happend, taking into account the duration of the event etc.
    bool transientOverPower = false;  // Set to true at first over-power event, cleared if bow is not over power
    elapsedMillis lastOverPowerEvent;
    uint32_t overPowerDuration = 100; // If bow over power events are going past the duration (in ms), signal the over power event

    uint32_t emergencyCoolDownEvent;
    uint16_t emergencyCoolDownPeriod = 1000;
    bool emergencyCoolDown = false;

private:
    void addTachoFreq(float freq);

    uint8_t motorDCDCEnPin;
    bool motorDCDCEn = false;
    uint8_t motorVoltagePin;
    Teensy_PWM *motorVoltagePWM;

    float motorVoltage = 6.6;

public:
    bool setMotorVoltage(float voltage);
    float getMotorVoltage();
    bool disableMotorPower();
    bool enableMotorPower();
    DCMotorControl(char inMotorRevPin, char inMotorVoltagePin, char inMotorDCDCEnPin, char inTachoPin, char inCurrentSensePin, char inMotorFaultPin);

    float getLastTachoFreq();
    void clearTachoData();
    float averageFreq();
    void setSpeedPWMSafe(uint16_t speed);
    void setSpeedPWM(uint16_t speed);
    uint16_t getSpeedPWM();
    void tachoISRHandler();
    bool checkTimeout();
//    void updateBow();
    float getCurrent();
    bool overCurrent();
    bool overPower();
    bool emergencyDisable(uint16_t coolDown);
    bool getMotorFault();
};

#endif


