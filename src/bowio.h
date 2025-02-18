/*
 * This file is part of The Ekdahl FAR firmware.
 *
 * The Ekdahl FAR firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Ekdahl FAR firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Ekdahl FAR firmware. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2024 Karl Ekdahl
 */
#ifndef BOWIO_H
#define BOWIO_H

#include "WProgram.h"
#include "Teensy_PWM.h"
//#include "Adafruit_PWM_Servo_Driver_Library/Adafruit_PWMServoDriver.h"

//#include "/home/knas/Desktop/Knas/Projects/superstrings/EkdahlFARModule/EkdahlFARModule/SingleBow/steppertestcode/farstepper_c.h"

#include <TMC2209.cpp>

#include "servostepper.h"

/**
 * @file bowio.h
 *
 * This is the main definition file of the bowIO class
 */

/// The bowIO class deals with interfacing with the hardware of a string unit
class bowIO {
    Teensy_PWM* bowMotorPWM;

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

    int lastBowMotorPWM = 0;                ///< Holds the last PWM speed value sent to the bowing wheel

public:
    uint16_t lastTilt = 0;                  ///< Last tilt value set
/*
    uint16_t SERVO_MIN = 100; ///< Min permissible PWM setting for servo rotation (was 100)
    uint16_t SERVO_MAX = 390; ///< Max permissible PWM setting for servo rotation (was 600)
*/
    //  char ZXBMFwdPin;      ///< Pin for motor driver forward motor PWM (always set to 1)
    char bowMotorRevPin;                    ///< Pin for motor driver reverse PWM
    char reflectorInterruptPin;             ///< Pin for reflection sensor output
    char bowMotorFaultPin;                  ///< Pin for motor driver ~FAULT signal
    uint8_t bowCurrentSensePin;             ///< Pin for motor current sense

    float bowMotorCurrentLimit = 1.75;       ///< Limit for tripping bow current draw error
    float bowMotorWattage = 12.84;

//    char tiltPWMChannel;  ///< Arduino PWM shield channel for tilt servo
//    farStepper *stepper = NULL;
//    bool delegateTilt = false;

//#define HWSERIAL Serial1
//#define driverTMC2209

    servoStepper *stepServoStepper = nullptr;
private:
//    uint16_t hwTiltPos = 0;
//    uint16_t hwLastTiltPos = 0;

    void addTachoFreq(float freq);

    HardwareSerial *stepSerialStream;       ///< Serial port for tilt stepper
    const long stepBaudRate = 115200;       ///< Baud rate for tilt stepper serial
    const int stepConnectDelay = 200;       ///< Delay in between connecting to stepper and issuing commandss
    const uint8_t stepRunCurrentPercent = 40;   ///< Stepper current during run phase
    const uint8_t stepHomeCurrentPercent = 20;
//    const int32_t VELOCITY = 100000;
//    #define microstepping 32
    const char stepMicrostepping = 32;      ///< Tilt stepper micro stepping
    char stepEnPin;                         ///< Pin for tilt stepper driver enable
    char stepperDirPin;                     ///< Pin for tilt stepper driver direction
    char stepStepPin;                       ///< Pin for tilt stepper driver step
    TMC2209 *stepTMC2209Driver = nullptr;   ///< Tilt stepper driver class pointer
    bool setupTMC2209();

    uint8_t bowMotorDCDCEnPin;
    bool bowMotorDCDCEn = false;
    uint8_t bowMotorVoltagePin;
//    uint16_t bowMotorVoltage = 3.3;
    Teensy_PWM *bowMotorVoltagePWM;

    float bowMotorVoltage = 6.6;

public:
//    Adafruit_PWMServoDriver *pwmShield = nullptr;  ///< Pointer to globally created Adafruit_PWMServoDriver instance

    //  char maxHarmonic = 59;  ///< Maximum allowed harmonic number from fundamental frequency
    float tiltAdjust = 0;     ///< Adjustment to the tilt value to avoid bow stalling and oscillation

    //  bowIO(char motorFwdPin, char motorRevPin, char _tachoPin, char _tiltPWMChannel, char _faultPin, Adafruit_PWMServoDriver *_pwmShield);
    //  bowIO(char motorRevPin, char _tachoPin, char _tiltPWMChannel, char _faultPin, Adafruit_PWMServoDriver *_pwmShield);
//    bowIO(char motorRevPin, char _tachoPin, char _tiltPWMChannel, Adafruit_PWMServoDriver *_pwmShield);
    bowIO(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin, char stepCorrectionSensorPin);

    float getLastTachoFreq();
    float averageFreq();
    void clearTachoData();

    void setSpeedPWMSafe(uint16_t speed);
    void setSpeedPWM(uint16_t speed);
    uint16_t getSpeedPWM();

    void setTiltPWM(uint16_t tilt);
    bool waitForTiltToComplete(uint16_t timeout = 3000);
    void tachoISRHandler();
    bool checkTimeout();
    void updateBow();

    bool createStepper(byte _pinStep, byte _pinDir, byte _pinHome);
    bool setBowMotorVoltage(float voltage);
    float getBowMotorVoltage();
    bool disableBowPower();
    bool enableBowPower();
    void getTMC2209Info();

    float getBowCurrent();
    bool bowOverCurrent();
    bool bowOverPower();
    bool getMotorFault();

    String dumpData();

    bool bowOverPowerFlag = false;       // External flag to show a over-power event has happend, taking into account the duration of the event etc.
    bool transientOverPower = false;  // Set to true at first over-power event, cleared if bow is not over power
    elapsedMillis lastBowOverPowerEvent;
    uint32_t bowOverPowerDuration = 100; // If bow over power events are going past the duration (in ms), signal the over power event

    bool homeBow(bool invert = false);
};

#endif
