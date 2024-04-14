#ifndef MUTE_H
#define MUTE_H

class mute {
private:
    uint16_t lastTilt;

    uint16_t restPosition = 0;
    uint16_t fullMutePosition = 0;
    uint16_t halfMutePosition = 0;

    bool sustain = false;

    uint16_t muteMaxPosition = 59000;

public:
    uint16_t hwMutePos = 0;
    uint16_t hwLastMutePos = 0;

    enum emutePosition {mpUndefined, mpRest, mpFull, mpHalf };
    emutePosition mutePosition = mpUndefined;

    servoStepper *stepServoStepper = nullptr;
private:
    HardwareSerial *stepSerialStream;       ///< Serial port for tilt stepper
    const long stepBaudRate = 115200;       ///< Baud rate for tilt stepper serial
    const int stepConnectDelay = 200;       ///< Delay in between connecting to stepper and issuing commandss
    const uint8_t stepRunCurrentPercent = 30;   ///< Stepper current during run phase
    const uint8_t stepHomeCurrentPercent = 10;
    const char stepMicrostepping = 32;      ///< Tilt stepper micro stepping
    char stepEnPin;                         ///< Pin for tilt stepper driver enable
    char stepperDirPin;                     ///< Pin for tilt stepper driver direction
    char stepStepPin;                       ///< Pin for tilt stepper driver step
    TMC2209 *stepTMC2209Driver = nullptr;   ///< Tilt stepper driver class pointer

public:
    mute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin);
    bool setupTMC2209();
    void getTMC2209Info();
    bool setTilt(uint16_t tilt);
    uint16_t getTilt();
    bool rest();
    bool fullMute();
    bool halfMute();
    bool setRestPosition(uint16_t inRestPosition);
    uint16_t getRestPosition();
    bool setFullMutePosition(uint16_t inFullMutePosition);
    uint16_t getFullMutePosition();
    bool setHalfMutePosition(uint16_t inHalfMutePosition);
    uint16_t getHalfMutePosition();
    bool saveRest();
    bool saveFullMute();
    bool saveHalfMute();
    bool setSustain(bool inSustain);
    bool getSustain();
    String dumpData();
    void updateMute();
};
#endif