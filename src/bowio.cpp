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

bool bowIO::setBowMotorVoltage(float voltage) { return dcMotorControl->setMotorVoltage(voltage); }

float bowIO::getBowMotorVoltage() { return dcMotorControl->getMotorVoltage(); }

bool bowIO::disableBowPower() { return dcMotorControl->disableMotorPower(); }

bool bowIO::enableBowPower() { return dcMotorControl->enableMotorPower(); }

bool bowIO::homeBow(bool invert) { return tmc2209ServoStepper->home(invert); }

bowIO::bowIO(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin, char stepCorrectionSensorPin) {
/*    bowMotorRevPin = motorRevPin;
    reflectorInterruptPin = tachoPin;
    bowCurrentSensePin = currentSensePin;
    bowMotorFaultPin = motorFaultPin;

    pinMode(bowMotorRevPin, OUTPUT);
    pinMode(reflectorInterruptPin, INPUT);
    pinMode(bowCurrentSensePin, INPUT);
    pinMode(bowMotorFaultPin, INPUT);

    bowMotorPWM = new Teensy_PWM(bowMotorRevPin, 20000, 0);
    bowMotorPWM->setResolution(BITDEPTH);

    bowMotorDCDCEnPin = motorDCDCEnPin;
    bowMotorVoltagePin = motorVoltagePin;
    pinMode(motorDCDCEnPin, OUTPUT);
    disableBowPower();

    bowMotorVoltagePWM = new Teensy_PWM(motorVoltagePin, 20000, 0);
    bowMotorVoltagePWM->setResolution(16);
    setBowMotorVoltage(bowMotorVoltage);

    analogWrite(bowMotorRevPin, 1);
    setSpeedPWM(0);*/
    dcMotorControl = new DCMotorControl(motorRevPin, motorVoltagePin, motorDCDCEnPin, tachoPin, currentSensePin, motorFaultPin);

    setTiltPWM(0);
/*
    stepSerialStream = stepSerialPort;
    stepServoStepper = new servoStepper(stepStepPin, stepDirPin, stepHomeSensorPin); //, stepEnPin);
    stepTMC2209Driver = new TMC2209();
    setupTMC2209();*/
    tmc2209ServoStepper = new Tmc2209ServoStepper(stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
    homeBow();
}

/** \brief Adds a tachometer value to the tachometer buffer, given in Hertz
 *
 *  Advances the tachoFreq index counter tachoFreqIndex and increases tachFreqCount if it has not yet reached tachoFreqLength.
 *  Values outside of the range defined by tachoFreqPermissibleMAX and tachoFreqPermissibleMIN are set to 0
 */
 /*
void bowIO::addTachoFreq(float freq) {
    if ((freq >= tachoFreqPerimssibleMAX) || (freq <= tachoFreqPerimssibleMIN)) {
        freq = 0;
    }

    tachoFreq[tachoFreqIndex] = freq;
    tachoFreqIndex++;
    if (tachoFreqCount < tachoFreqLength) { tachoFreqCount++; }
    if (tachoFreqIndex >= tachoFreqLength) { tachoFreqIndex = 0; }
}
*/
/// Returns the last added tachometer value
float bowIO::getLastTachoFreq() {
/*    if (tachoFreqIndex < 1) {
        return tachoFreq[tachoFreqLength - 1];
    } else {
        return tachoFreq[tachoFreqIndex - 1];
    }*/
    return dcMotorControl->getLastTachoFreq();
}

void bowIO::clearTachoData() {
/*    tachoFreqCount = 0;
    tachoFreqIndex = 0;*/
    dcMotorControl->clearTachoData();
}

/** \brief Returns the average bow speed frequency
 *
 *  - Calculates the average bow speed frequency using the values stored in the tachometer buffer by
 *    -# finding the median of the buffer
 *    -# recalculating the average skipping values straying from the median by a frequency given by permissibleFreqDeviation
 *
 */
float bowIO::averageFreq() {
/*    if (tachoFreqCount < tachoFreqLength) {
        if (tachoFreq[tachoFreqIndex] == 0) {
            return 0;
        } else {
            debugPrintln("AverageFreq: error, not enough point stored.", Priority);
            return -1;
        }
    }

    float localBuffer[tachoFreqLength];
    for (int i=0; i<tachoFreqLength; i++) { localBuffer[i] = tachoFreq[i]; }

    // Sort the buffer so that middle will be median
    std::sort(localBuffer, localBuffer + tachoFreqLength);
    // Take out the median value
    float median = localBuffer[tachoFreqLength / 2];

    float average = 0;
    int averageCount = tachoFreqLength;

    for (int i=0; i < tachoFreqLength; i++) {
        if ((localBuffer[i] > (median + permissibleFreqDeviation)) || (localBuffer[i] < (median - permissibleFreqDeviation))) {
            averageCount--;
        } else {
            average += localBuffer[i];
        }
    }

    average /= averageCount;
    // oldAverage = average; // moved to below
    /// \todo What the fuck this wont work, move oldAverage = average to after this if statement and try again
    if ((average < 0) || (average > 1000)) {
        debugPrint("AverageFreq: error, data out of range " + String(average) + " with data points " + String(averageCount), Error);
        debugPrint("Series ", Error);
        for (int i=0; i<tachoFreqLength; i++) { debugPrint(String(localBuffer[i]) + ", ", Error); }
        debugPrintln(".", Error);
        //average = -1;
        average = tachoOldAverage;
    }
    tachoOldAverage = average; // was above before, check if problem

    return average;*/
    return dcMotorControl->averageFreq();
}

/// Ramps the bow speed PWM from the last PWM value given to the value given in speed, blocking, only use for testing purposes
void bowIO::setSpeedPWMSafe(uint16_t speed) {
/*    int a = speed;
    // ramp the speed if going up, driver doesn't like sudden fast changes
    analogWrite(bowMotorRevPin, 65535 - a);
    if (lastBowMotorPWM < a) {
        for (int i=lastBowMotorPWM; i<a; i++) {
            delayMicroseconds(5); //5
            analogWrite(bowMotorRevPin, 65535 - i);
        };
    } else {
        for (int i=lastBowMotorPWM; i>a; i--) {
            delayMicroseconds(5); //5
            analogWrite(bowMotorRevPin, 65535 - i);
        };
    }

    lastBowMotorPWM = a;*/
    dcMotorControl->setSpeedPWMSafe(speed);
}


/// Sets the bow speed PWM using a unsigned 16-bit int (RAW)
void bowIO::setSpeedPWM(uint16_t speed) {
/*    int a = speed;
    bowMotorPWM->setPWM_manual(bowMotorRevPin, (65535 - a) / BITDIV);
    lastBowMotorPWM = a;*/
    dcMotorControl->setSpeedPWM(speed);
}

uint16_t bowIO::getSpeedPWM() {
    return 0;
    //return lastBowMotorPWM;
    if (dcMotorControl == nullptr) {
        debugPrintln("NULL", debugPrintType::Error);
        return 0;
    }
    return dcMotorControl->getSpeedPWM();
}

/** \brief Sets the tilt servo PWM value using a unsigned 16-bit int (RECALCULATED)
 *
 *  Recalculated internally to use SERVO_MIN and SERVO_MAX as lower and upper bound respectively
 */
void bowIO::setTiltPWM(uint16_t tilt) {
    //if (stepServoStepper != nullptr) {
    if ((tmc2209ServoStepper != nullptr) && (tmc2209ServoStepper->stepServoStepper != nullptr)) {
        debugPrintln("Setting stepper tilt to " + String(tilt), Hardware);
        //stepServoStepper->setPosition(tilt);
        tmc2209ServoStepper->stepServoStepper->setPosition(tilt);
        lastTilt = tilt;
    }
}

bool bowIO::waitForTiltToComplete(uint16_t timeout) {
    //return stepServoStepper->completeTask(timeout);
    return tmc2209ServoStepper->stepServoStepper->completeTask(timeout);
}


/// Secondary callback handler for tachometer events
void bowIO::tachoISRHandler() {
/*    asm("nop");
    uint8_t state = digitalRead(reflectorInterruptPin);
    if (state == lastReflectorISRState) { return; }
    if (state == 0) {
        reflectorCyclePeriod = reflectorCounter;
        reflectorCounter = 0;
    }
    addTachoFreq(((double) 1000000 / reflectorCyclePeriod));
    asm("dsb");
    lastReflectorISRState = state;*/
    dcMotorControl->tachoISRHandler();
}

/// Check whether bow has timed out and add a value of 0 Hertz if so
bool bowIO::checkTimeout() {
/*    if (reflectorCounter > reflectorZeroTimeoutValue) {
        addTachoFreq(0);
        return true;
    } else {
        return false;
    }*/
    return dcMotorControl->checkTimeout();
}

//void bowIO::updateBow() {
/*    if (bowOverPower()) {

    }*/
//    dcMotorControl->updateBow();
//}

bool bowIO::setupTMC2209() {
    return tmc2209ServoStepper->setupTMC2209();
}

void bowIO::getTMC2209Info() {
/*    debugPrintln("*************************", Debug);
    debugPrintln("getSettings()", Debug);
    TMC2209::Settings settings = stepTMC2209Driver->getSettings();
    debugPrintln("settings.is_communicating = " + String(settings.is_communicating), Debug);
    debugPrintln("settings.is_setup = " + String(settings.is_setup), Debug);
    debugPrintln("settings.software_enabled = " + String(settings.software_enabled), Debug);
    debugPrintln("settings.microsteps_per_step = " + String(settings.microsteps_per_step), Debug);
    debugPrintln("settings.inverse_motor_direction_enabled = " + String(settings.inverse_motor_direction_enabled), Debug);
    debugPrintln("settings.stealth_chop_enabled = " + String(settings.stealth_chop_enabled), Debug);
    debugPrintln("settings.standstill_mode = ", Debug);
    switch (settings.standstill_mode)
    {
    case TMC2209::NORMAL:
        debugPrintln("normal", Debug);
        break;
    case TMC2209::FREEWHEELING:
        debugPrintln("freewheeling", Debug);
        break;
    case TMC2209::STRONG_BRAKING:
        debugPrintln("strong_braking", Debug);
        break;
    case TMC2209::BRAKING:
        debugPrintln("braking", Debug);
        break;
    }
    debugPrintln("settings.irun_percent = " + String(settings.irun_percent), Debug);
    debugPrintln("settings.irun_register_value = " + String(settings.irun_register_value), Debug);
    debugPrintln("settings.ihold_percent = " + String(settings.ihold_percent), Debug);
    debugPrintln("settings.ihold_register_value = " + String(settings.ihold_register_value), Debug);
    debugPrintln("settings.iholddelay_percent = " + String(settings.iholddelay_percent), Debug);
    debugPrintln("settings.iholddelay_register_value = " + String(settings.iholddelay_register_value), Debug);
    debugPrintln("settings.automatic_current_scaling_enabled = " + String(settings.automatic_current_scaling_enabled), Debug);
    debugPrintln("settings.automatic_gradient_adaptation_enabled = " + String(settings.automatic_gradient_adaptation_enabled), Debug);
    debugPrintln("settings.pwm_offset = " + String(settings.pwm_offset), Debug);
    debugPrintln("settings.pwm_gradient = " + String(settings.pwm_gradient), Debug);
    debugPrintln("settings.cool_step_enabled = " + String(settings.cool_step_enabled), Debug);
    debugPrintln("settings.analog_current_scaling_enabled = " + String(settings.analog_current_scaling_enabled), Debug);
    debugPrintln("settings.internal_sense_resistors_enabled = " + String(settings.internal_sense_resistors_enabled), Debug);
    debugPrintln("*************************", Debug);
    debugPrintln("", Debug);

    debugPrintln("*************************", Debug);
    debugPrintln("hardwareDisabled()", Debug);
    bool hardware_disabled = stepTMC2209Driver->hardwareDisabled();
    debugPrint("hardware_disabled = " + String(hardware_disabled), Debug);
    debugPrintln("*************************", Debug);
    debugPrintln("", Debug);

    debugPrintln("*************************", Debug);
    debugPrintln("getStatus()", Debug);
    TMC2209::Status status = stepTMC2209Driver->getStatus();
    debugPrintln("status.over_temperature_warning = " + String(status.over_temperature_warning), Debug);
    debugPrintln("status.over_temperature_shutdown = " + String(status.over_temperature_shutdown), Debug);
    debugPrintln("status.short_to_ground_a = " + String(status.short_to_ground_a), Debug);
    debugPrintln("status.short_to_ground_b = " + String(status.short_to_ground_b), Debug);
    debugPrintln("status.low_side_short_a = " + String(status.low_side_short_a), Debug);
    debugPrintln("status.low_side_short_b = " + String(status.low_side_short_b), Debug);
    debugPrintln("status.open_load_a = " + String(status.open_load_a), Debug);
    debugPrintln("status.open_load_b = " + String(status.open_load_b), Debug);
    debugPrintln("status.over_temperature_120c = " + String(status.over_temperature_120c), Debug);
    debugPrintln("status.over_temperature_143c = " + String(status.over_temperature_143c), Debug);
    debugPrintln("status.over_temperature_150c = " + String(status.over_temperature_150c), Debug);
    debugPrintln("status.over_temperature_157c = " + String(status.over_temperature_157c), Debug);
    debugPrintln("status.current_scaling = " + String(status.current_scaling), Debug);
    debugPrintln("status.stealth_chop_mode = " + String(status.stealth_chop_mode), Debug);
    debugPrintln("status.standstill = " + String(status.standstill), Debug);
    debugPrintln("*************************", Debug);
    debugPrintln("", Debug);
    return;*/
    return tmc2209ServoStepper->getTMC2209Info();
}

float bowIO::getBowCurrent() {
//      return analogRead(bowCurrentSensePin) * 3.3 / 4096;
    return dcMotorControl->getCurrent();
}

bool bowIO::bowOverCurrent() {
/*    if (getBowCurrent() >= bowMotorCurrentLimit) {
        return true;
    } else {
        return false;
    }*/
    return dcMotorControl->overCurrent();
}

bool bowIO::bowOverPower() {
/*    if (getBowCurrent() >= (bowMotorWattage / bowMotorVoltage)) {
        // If we have already set the internal over-power flag
        if (transientOverPower) {
            // And that was set more than the allowed time-span ago, aka the over-power event has been going on for X ms
            if (lastBowOverPowerEvent > bowOverPowerDuration) {
                // Signal the external over power flag
                bowOverPowerFlag = true;
            }
        } else {
            // If this is a new over-power event set the flag and clear the event-time
            transientOverPower = true;
            lastBowOverPowerEvent = 0;
        }
        return true;
    } else {
        // Clear everything if we no longer have an over power event
        bowOverPowerFlag = false;
        transientOverPower = false;
        return false;
    }*/
    return dcMotorControl->overPower();
}

// Emergency disable of power with a required cooldown period before bow is allowed to be started again
bool bowIO::emergencyBowDisable(uint16_t coolDown) {
/*    if (emergencyCoolDown) { return false; }
    emergencyCoolDownEvent = millis();
    emergencyCoolDown = true;
    debugPrintln("Emergency disable! Setting cool down to " + String(emergencyCoolDownEvent), Debug);
    disableBowPower();
    return true;*/
    return dcMotorControl->emergencyDisable(coolDown);
}

bool bowIO::getMotorFault() {
/*    if (digitalRead(bowMotorFaultPin) == 0) {    // changed 2024-05-07
        return  true;
    } else {
        return false;
    }*/
    return dcMotorControl->getMotorFault();
}

String bowIO::dumpData() {
    String dump;
//    dump = "bmv:" + String(bowMotorVoltage) + ",";
    dump = "bmv:" + String(dcMotorControl->getMotorVoltage()) + ",";
    return dump;
}
#endif
