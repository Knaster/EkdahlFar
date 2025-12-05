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
/** \brief Constructor for the bowIO class
 *
 *  + Sets up all hardware pins associated with the string unit
 *  + Creates a new Teensy_PWM object for the bow motor driver
 *  + Associates a channel on the Arduino PWM shield for the tilt servo motor
 */

#ifndef BOWIO_C
#define BOWIO_C

#include "bowio.hpp"

#define BITDEPTH 16
#define BITDIV 1 // = 2^(16-BITDEPTH)

#define DCDCLOWERBOUND 1.2
#define DCDCUPPERBOUND 9

bowIO::bowIO(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin, char stepCorrectionSensorPin) {
    dcMotorControl = new DCMotorControl(motorRevPin, motorVoltagePin, motorDCDCEnPin, tachoPin, currentSensePin, motorFaultPin);
    bowPressure = new BowPressure(stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
//    setTiltPWM(0);
//    tmc2209ServoStepper = new Tmc2209ServoStepper(stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
//    homeBow();
}
/*
bool bowIO::setBowMotorVoltage(float voltage) { return dcMotorControl->setMotorVoltage(voltage); }

float bowIO::getBowMotorVoltage() { return dcMotorControl->getMotorVoltage(); }

bool bowIO::disableBowPower() { return dcMotorControl->disableMotorPower(); }

bool bowIO::enableBowPower() { return dcMotorControl->enableMotorPower(); }

//bool bowIO::homeBow(bool invert) { return tmc2209ServoStepper->home(invert); }

float bowIO::getLastTachoFreq() { return dcMotorControl->getLastTachoFreq(); }

void bowIO::clearTachoData() { dcMotorControl->clearTachoData(); }

float bowIO::averageFreq() { return dcMotorControl->averageFreq(); }

void bowIO::setSpeedPWMSafe(uint16_t speed) { dcMotorControl->setSpeedPWMSafe(speed); }

void bowIO::setSpeedPWM(uint16_t speed) { dcMotorControl->setSpeedPWM(speed); }

uint16_t bowIO::getSpeedPWM() { return dcMotorControl->getSpeedPWM(); }
*/
/** \brief Sets the tilt servo PWM value using a unsigned 16-bit int (RECALCULATED)
 *
 *  Recalculated internally to use SERVO_MIN and SERVO_MAX as lower and upper bound respectively
 */
 /*
void bowIO::setTiltPWM(uint16_t tilt) {
    if ((tmc2209ServoStepper != nullptr) && (tmc2209ServoStepper->stepServoStepper != nullptr)) {
        debugPrintln("Setting stepper tilt to " + String(tilt), Hardware);
        tmc2209ServoStepper->stepServoStepper->setPosition(tilt);
        lastTilt = tilt;
    }
}

bool bowIO::waitForTiltToComplete(uint16_t timeout) { return tmc2209ServoStepper->stepServoStepper->completeTask(timeout); }
*/
void bowIO::tachoISRHandler() { /*dcMotorControl->tachoISRHandler();*/ }
/*
bool bowIO::checkTimeout() { return dcMotorControl->checkTimeout(); }

//bool bowIO::setupTMC2209() { return tmc2209ServoStepper->setupTMC2209(); }

//void bowIO::getTMC2209Info() { return tmc2209ServoStepper->getTMC2209Info(); }

float bowIO::getBowCurrent() { return dcMotorControl->getCurrent(); }

bool bowIO::bowOverCurrent() { return dcMotorControl->overCurrent(); }

bool bowIO::bowOverPower() { return dcMotorControl->overPower(); }

bool bowIO::emergencyBowDisable(uint16_t coolDown) { return dcMotorControl->emergencyDisable(coolDown); }

bool bowIO::getMotorFault() {return dcMotorControl->getMotorFault(); }
*/
String bowIO::dumpData() {
    String dump = "";
//    dump = "bmv:" + String(dcMotorControl->getMotorVoltage()) + ",";
    return dump;
}
#endif
