#ifndef TMC2209_SERVOSTEPPER_H
#define TMC2209_SERVOSTEPPER_H

#include "TMC2209.cpp"
#include "../src/servostepper.hpp"

class Tmc2209ServoStepper {

private:
    HardwareSerial *stepSerialStream;           ///< Serial port for tilt stepper
    const long stepBaudRate = 115200;           ///< Baud rate for tilt stepper serial
    const int stepConnectDelay = 200;           ///< Delay in between connecting to stepper and issuing commandss
    TMC2209 *stepTMC2209Driver = nullptr;       ///< Tilt stepper driver class pointer

public:
    servoStepper *stepServoStepper = nullptr;

    const uint8_t stepRunCurrentPercent = 40;   ///< Stepper current during run phase
    const uint8_t stepHomeCurrentPercent = 20;
    const uint8_t stepHoldCurrentPercent = 10;
    const char stepMicrostepping = 32;          ///< Tilt stepper micro stepping

    Tmc2209ServoStepper(char inStepDirPin, char inStepStepPin, HardwareSerial *inStepSerialPort, char inStepHomeSensorPin);
    bool setupTMC2209();
    void getTMC2209Info();
    bool home(bool invert);
};

#endif

