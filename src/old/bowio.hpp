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

#include "base/arduinorequired.hpp"
#include "Teensy_PWM.h"

#include "generic_functions/tmc2209_servostepper.hpp"
#include "generic_functions/dcmotorcontrol.hpp"
#include "bow/bowpressure.hpp"

/**
 * @file bowio.hpp
 *
 * This is the main definition file of the bowIO class
 */

/// The bowIO class deals with interfacing with the hardware of a string unit
class bowIO {
public:
//    uint16_t lastTilt = 0;                  ///< Last tilt value set

//    float bowMotorCurrentLimit = 1.75;       ///< Limit for tripping bow current draw error
//    float bowMotorWattage = 12.84;

//    Tmc2209ServoStepper *tmc2209ServoStepper = nullptr;
    DCMotorControl *dcMotorControl = nullptr;
    BowPressure *bowPressure = nullptr;

private:
//    bool setupTMC2209();

public:
    bowIO(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin, char stepCorrectionSensorPin);

//    float getLastTachoFreq();
//    float averageFreq();
//    void clearTachoData();

//    void setSpeedPWMSafe(uint16_t speed);
//    void setSpeedPWM(uint16_t speed);
//    uint16_t getSpeedPWM();

//    void setTiltPWM(uint16_t tilt);
//    bool waitForTiltToComplete(uint16_t timeout = 3000);
    void tachoISRHandler();
//    bool checkTimeout();

//    bool setBowMotorVoltage(float voltage);
//    float getBowMotorVoltage();
//    bool disableBowPower();
//    bool enableBowPower();
//    void getTMC2209Info();

//    float getBowCurrent();
//    bool bowOverCurrent();
//    bool bowOverPower();
//    bool getMotorFault();

    String dumpData();

//    bool emergencyBowDisable(uint16_t coolDown);    // Emergency disable of power with a required cooldown period before bow is allowed to be started again

//    bool homeBow(bool invert = false);
};

#endif
