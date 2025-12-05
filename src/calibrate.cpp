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
#include "avr_functions.h"
#include <stdint.h>
#include <sys/types.h>
#include "core_pins.h"
#include <sys/_stdint.h>
#include "arm_math.h"

#ifndef CALIBRATE_C
#define CALIBRATE_C

#include "calibrate.hpp"

/// Class constructor. Binds to the associated bowIO class and reads any existing EEPROM data into calibrationData
calibrate::calibrate(bowIO &_bowIO, CalibrationData &__calibrationData, bowControl &_bowControl) {
    bowIOConnect = &_bowIO;
    calibrationData = &__calibrationData;
    bowControlConnect = &_bowControl;
}


/** \brief Waits for the bow speed to stabilize, used after setting a new bow speed
 *
 *  Goes through maxIterations number of iterations with a preset delay in between each delay.
 *  If bow speed has not stabilized after the max amount of iterations the function returns false otherwise it will return true
 */
bool calibrate::waitForBowToStabilize(uint16_t maxIterations = 10) {
    int average;
    int iterations = 0;
    int newAverage = bowIOConnect->dcMotorControl->getAverageTachometerFreq();
    do {
        average = newAverage;
        delay(250);
        bowIOConnect->dcMotorControl->checkTachometerTimeout();
        newAverage = bowIOConnect->dcMotorControl->getAverageTachometerFreq();
        iterations++;
        if (iterations > maxIterations) {
            debugPrintln("Bow did not stabilize within the given iterations", Priority);
            return false;
        }
    } while (average != newAverage);
//    debugPrintln("Bow settled at ~" + String(newAverage) + "Hz, last value ~" + String(average) + "Hz", Debug);
    return true;
}

/* \brief Find the minimum pressure where the bow first hits the string aka where a recorded slow-down in bow speed happens using the minSpeedPWM as the metric
 *
 */
bool calibrate::findMinPressure() {
    debugPrintln("Finding min contact pressure for minimum usable speed", TextInfo);

    bowIOConnect->dcMotorControl->setSpeedPWMSafe(calibrationData->minSpeedPWM);
//    bowIOConnect->setTiltPWM(0);
//    bowIOConnect->waitForTiltToComplete();
    bowIOConnect->bowPressure->setHardwarePressure(0);
    if (!bowIOConnect->bowPressure->completeTask()) {
        debugPrintln("Failed at setting bow pressure", debugPrintType::Error);
        return false;
    }
    if (!waitForBowToStabilize()) { return false; }
    delay(500);

    float initialMotorFreq = 0;
    int tempTimeOut = 0;
    do {
        initialMotorFreq = bowIOConnect->dcMotorControl->getAverageTachometerFreq();
        delayMicroseconds(10);
        tempTimeOut++;
    } while((initialMotorFreq <= 0) && (tempTimeOut<100));
    if (initialMotorFreq <= 0) { return false; }
    debugPrintln("Starting freq " + String(initialMotorFreq) + String("Hz"), TextInfo);

    uint16_t i;
    int average;
    for (i =0; i<maxTestPressure; i+=10) {
//        bowIOConnect->setTiltPWM(i);
//        bowIOConnect->waitForTiltToComplete(10);    // was 3000 (2024-06-27)
        bowIOConnect->bowPressure->setHardwarePressure(i);
            if (!bowIOConnect->bowPressure->completeTask()) {
            debugPrintln("Failed at setting bow pressure", debugPrintType::Error);
            return false;
        }

        do {
//            average = bowIOConnect->getAverageTachometerFreq();
            average = bowIOConnect->dcMotorControl->getAverageTachometerFreq();
        } while (average <= 0);

//        if (bowIOConnect->getAverageTachometerFreq() < (initialMotorFreq - deviation)) {
        if (bowIOConnect->dcMotorControl->getAverageTachometerFreq() < (initialMotorFreq - deviation)) {
            break;
        }
    }
    //if (bowIOConnect->getAverageTachometerFreq() <= 0) { return false; }
    if (bowIOConnect->dcMotorControl->getAverageTachometerFreq() <= 0) { return false; }
//    debugPrintln("intial contact found at "  + String(i) + " frequency " + String(bowIOConnect->getAverageTachometerFreq()) + "Hz", TextInfo);
    debugPrintln("intial contact found at "  + String(i) + " frequency " + String(bowIOConnect->dcMotorControl->getAverageTachometerFreq()) + "Hz", TextInfo);

    if (i > pressureTestRetract) {
        i -= pressureTestRetract;
    } else {
        i = 0;
    }

//    bowIOConnect->setSpeedPWMSafe(0);
//    bowIOConnect->setTiltPWM(i);
//    bowIOConnect->waitForTiltToComplete();
    bowIOConnect->dcMotorControl->setSpeedPWMSafe(0);
    bowIOConnect->bowPressure->setHardwarePressure(i);
    if (!bowIOConnect->bowPressure->completeTask()) {
        debugPrintln("Failed at setting bow pressure", debugPrintType::Error);
        return false;
    }
    debugPrintln("starting over at PWM " + String(i), TextInfo);

    bowIOConnect->dcMotorControl->setSpeedPWMSafe(calibrationData->minSpeedPWM);
    if (!waitForBowToStabilize()) { return false; }

    bool fault = false;

    // Set the tilt PWM with a long delay allowance at first
//    bowIOConnect->setTiltPWM(i);
    bowIOConnect->bowPressure->setHardwarePressure(i);
    if (!bowIOConnect->bowPressure->completeTask()) {
        debugPrintln("Failed at setting bow pressure", debugPrintType::Error);
        return false;
    }
    // Then move on to the test
    do {
/*        bowIOConnect->setTiltPWM(i);
        if (!bowIOConnect->waitForTiltToComplete(1)) {  // was 100
            fault = true;
            break;
        }
*/
        bowIOConnect->bowPressure->setHardwarePressure(i);
        if (!bowIOConnect->bowPressure->completeTask(1)) {
            fault = true;
            break;
        }

        delayMicroseconds(100);
        if (bowIOConnect->dcMotorControl->getAverageTachometerFreq() < (initialMotorFreq - deviation)) {
            break;
        }
        i++;
    } while(1 != 2);
    if ((bowIOConnect->dcMotorControl->getAverageTachometerFreq() <= 0) || (fault)) { return false; }

//    calibrationData->firstTouchPressure = i;
    bowIOConnect->bowPressure->setEngagePressure(i);

//    debugPrintln("Final contact PWM " + String(calibrationData->firstTouchPressure), TextInfo);
//    debugPrintln(" at frequency " + String(bowIOConnect->getAverageTachometerFreq()) + "Hz", TextInfo);
    debugPrintln("Final contact PWM " + String(bowIOConnect->bowPressure->getEngagePressure()), TextInfo);
    debugPrintln(" at frequency " + String(bowIOConnect->dcMotorControl->getAverageTachometerFreq()) + "Hz", TextInfo);

//    bowIOConnect->setSpeedPWMSafe(0);
//    bowIOConnect->setTiltPWM(0);
    bowIOConnect->dcMotorControl->setSpeedPWMSafe(0);
    bowIOConnect->bowPressure->setHardwarePressure(0);

    return true;
}

/** \brief Find the minimum and maximum usable pressure
 * Maximum pressure is calculated as the pressure just before the bow motor stalls against the string using maxSpeedPWM as the metric
 */
bool calibrate::findMaxPressure() {
    debugPrintln("Finding max contact", TextInfo);

//    bowControlConnect->run = 1;
//    bowIOConnect->setSpeedPWMSafe( calibrationData->minSpeedPWM + (calibrationData->maxSpeedPWM - calibrationData->minSpeedPWM) / 2  );
//    bowIOConnect->setTiltPWM(0);
//    bowIOConnect->waitForTiltToComplete(5000);
    bowIOConnect->dcMotorControl->setSpeedPWMSafe(bowIOConnect->dcMotorControl->getMinSpeedPWM() + (bowIOConnect->dcMotorControl->getMaxSpeedPWM() - bowIOConnect->dcMotorControl->getMinSpeedPWM()) / 2);
    bowIOConnect->bowPressure->setHardwarePressure(0);
    if (!bowIOConnect->bowPressure->completeTask()) {
        debugPrintln("Failed at setting bow pressure", debugPrintType::Error);
        return false;
    }

    delay(500);
    if (!waitForBowToStabilize(100)) { return false; }
//    float initialMotorFreq = bowIOConnect->getAverageTachometerFreq();
    float initialMotorFreq = bowIOConnect->dcMotorControl->getAverageTachometerFreq();
    if (initialMotorFreq == 0) { return false; }

//    int16_t j = calibrationData->firstTouchPressure;
    int16_t j = bowIOConnect->bowPressure->getEngagePressure();

    if (j > pressureTestRetract) {
        j -= pressureTestRetract;
    } else {
        j = 0;
    }
    //bowIOConnect->setTiltPWM((calibrationData->firstTouchPressure - pressureTestRetract));
//    bowIOConnect->setTiltPWM(j);
//    if (bowIOConnect->waitForTiltToComplete(5000)) {
    bowIOConnect->bowPressure->setHardwarePressure(j);
    if (bowIOConnect->bowPressure->completeTask(5000)) {
        debugPrintln("Starting freq " + String(initialMotorFreq) + "Hz", TextInfo);

        uint16_t i;
        float average;
        bool fault = false;

//        for (i =(calibrationData->firstTouchPressure - pressureTestRetract); i<maxTestPressure; i+=1) {
        for (i =j; i<maxTestPressure; i+=1) {
//            bowIOConnect->setTiltPWM(i);
//            if (!bowIOConnect->waitForTiltToComplete(100)) {
            bowIOConnect->bowPressure->setHardwarePressure(j);
            if (bowIOConnect->bowPressure->completeTask(5000)) {
                debugPrintln("Pressure setting didn't finish", Debug);
                fault = true;
                break;
            }
            delayMicroseconds(100);
//            average = bowIOConnect->getAverageTachometerFreq();
//            bowIOConnect->checkTimeout();
            average = bowIOConnect->dcMotorControl->getAverageTachometerFreq();
            bowIOConnect->dcMotorControl->checkTachometerTimeout();

    //        debugPrintln("Tilt " + String (i) + " Average frequency " + String(average) + " power " + String(bowIOConnect->getBowCurrent() * bowIOConnect->getBowMotorVoltage())
    //            + ", current " + String(bowIOConnect->getBowCurrent()) + "A, motor fault " + String(bowIOConnect->getMotorFault()), Debug);

//            if ((average == 0) || bowIOConnect->bowOverPower() || bowIOConnect->bowOverCurrent()) {
            if ((average == 0) || bowIOConnect->dcMotorControl->isOverPower() || bowIOConnect->dcMotorControl->isOverCurrent()) {
                break;
            }
        }

        //calibrationData->stallPressure = i;
        bowIOConnect->bowPressure->setMaxPressure(i);
        if (!fault) {
//            debugPrintln("Final contact PWM " + String(calibrationData->stallPressure) + " at frequency " + String(bowIOConnect->getAverageTachometerFreq()) + "Hz"/* with max current " +
            debugPrintln("Final contact PWM " + String(bowIOConnect->bowPressure->getMaxPressure()) + " at frequency " + String(bowIOConnect->dcMotorControl->getAverageTachometerFreq()) + "Hz"/* with max current " +
            String(maxCurrentRead)*/, TextInfo);
        } else {
//            debugPrintln("Got Fault! Final contact PWM " + String(calibrationData->stallPressure) + " at frequency " + String(bowIOConnect->getAverageTachometerFreq()) + "Hz"/* with max current " +
              debugPrintln("Final contact PWM " + String(bowIOConnect->bowPressure->getMaxPressure()) + " at frequency " + String(bowIOConnect->dcMotorControl->getAverageTachometerFreq()) + "Hz"/* with max current " +
          String(maxCurrentRead)*/, TextInfo);
    //        debugPrintln("Max pressure calibration ERROR!", debugPrintType::Error);
        }
    } else {
        debugPrintln("Couldn't get calibration start conditions", debugPrintType::Error);
    }

//    bowIOConnect->setSpeedPWMSafe(0);
//    bowIOConnect->setTiltPWM(0);
    bowIOConnect->dcMotorControl->setSpeedPWMSafe(0);
    bowIOConnect->bowPressure->setHardwarePressure(0);
    delay(1000);
    //bowIOConnect->homeBow();
    bowIOConnect->bowPressure->home();

    return true;
}

bool calibrate::findMinMaxPressure() {
    // Save the current state of the stepper auto-correction setting and restore before exiting
    bool autoCorrectPositionSave = bowIOConnect->bowPressure->getAutoCorrect();
    bowIOConnect->bowPressure->setAutoCorrect(false);

    bool PIDOn = bowControlConnect->PIDon;
    bowControlConnect->PIDon = 0;
//    bowControlConnect->run = 1;
    bowIOConnect->dcMotorControl->enableMotorPower();

    uint16_t speed = bowIOConnect->bowPressure->getSpeed();
    bowIOConnect->bowPressure->setSpeed(5);

    //bool result = findMinPressure() && findMaxPressure();
    bool result = findMinPressure();
    if (result) {
        result = result & findMaxPressure();
    }

    if (result) {
/*        if (calibrationData->firstTouchPressure > 3000) {
            calibrationData->firstTouchPressure -= 3000;
        } else {
            calibrationData->firstTouchPressure =  0;
        }

        if (calibrationData->firstTouchPressure > 3000) {
            calibrationData->restPosition = calibrationData->firstTouchPressure - 3000;
        } else {
            calibrationData->restPosition =  0;
        }
        debugPrintln("Good results acquired, setting firstTouchPressure to " + String(calibrationData->firstTouchPressure) + " and restPosition to " + String(calibrationData->restPosition), debugPrintType::Debug);
*/
        if (bowIOConnect->bowPressure->getEngagePressure() > 3000) {
            bowIOConnect->bowPressure->setEngagePressure(bowIOConnect->bowPressure->getEngagePressure() - 3000);
        } else {
            bowIOConnect->bowPressure->setEngagePressure(0);
        }

        if (bowIOConnect->bowPressure->getEngagePressure() > 3000) {
            bowIOConnect->bowPressure->setRestPressure(bowIOConnect->bowPressure->getEngagePressure() - 3000);
        } else {
            bowIOConnect->bowPressure->setRestPressure(0);
        }
        debugPrintln("Good results acquired, setting firstTouchPressure to " + String(bowIOConnect->bowPressure->getEngagePressure()) + " and restPosition to " +
                     String(bowIOConnect->bowPressure->getRestPressure()), debugPrintType::Debug);
    } else {
        debugPrintln("Results invalid, not modifying data" , debugPrintType::Error);
    }

//    bowIOConnect->tmc2209ServoStepper->stepServoStepper->autoCorrectPosition = autoCorrectPositionSave;
//    bowIOConnect->tmc2209ServoStepper->stepServoStepper->setSpeed(speed);
    bowIOConnect->bowPressure->setAutoCorrect(autoCorrectPositionSave);
    bowIOConnect->bowPressure->setSpeed(speed);
    bowControlConnect->run = 0;
    bowControlConnect->PIDon = PIDOn;

    return result;
}

/** \brief Find the minimum and maximum bow speed using direct hardware PWM control (as opposed to PID control)
 *
 *  Finds the minimum PWM value fed to the bow motor controller where a bow speed higher than zero occurs and record PWM and frequency.
 *  The maximum speed is recorded as the lowest PWM value that gives a noticeable change in bow speed from the absolute maximum
 *  Min and max usable speed is pre-calculated using the usableMultiplier variable
 *
 *
 *
 *
 */
bool calibrate::findMinMaxSpeedPWM() {
//    bool finished = false;
    int speed = 0;
    uint32_t i =0;
//    float bowMotorWattage = bowIOConnect->bowMotorWattage;
    float bowMotorPowerLimit = bowIOConnect->dcMotorControl->getMotorMaxPower();

    bool PIDOn = bowControlConnect->PIDon;
    bowControlConnect->PIDon = 0;
//    bowControlConnect->run = 1;
    bowIOConnect->dcMotorControl->enableMotorPower();

    debugPrintln("Finding min speed", TextInfo);
    bowIOConnect->dcMotorControl->setSpeedPWMSafe(0);
    bowIOConnect->bowPressure->setPressureSafe(0);
    if (!bowIOConnect->bowPressure->completeTask()) {
        debugPrintln("Failed to set pressure", debugPrintType::Error);
        return false;
    }

    // Go from 0 PWM to 65535 and break the first time that the frequency is above zero
    do {
        //bowIOConnect->setSpeedPWMSafe(speed);
        bowIOConnect->dcMotorControl->setSpeedPWMSafe(speed);
        delayMicroseconds(1000);
        speed++;
//    } while ((bowIOConnect->getAverageTachometerFreq() < minPermissibleBowSpeedForCalibration) && (speed < 65536));
    } while ((bowIOConnect->dcMotorControl->getAverageTachometerFreq() < minPermissibleBowSpeedForCalibration) && (speed < 65536));

    delay(500);
    if (!waitForBowToStabilize() || (speed == 65536)) { goto skiptoend; }

    //calibrationData->minSpeedPWM = speed;
    //calibrationData->minHz = bowIOConnect->getAverageTachometerFreq();
    bowIOConnect->dcMotorControl->setMinSpeedPWM(speed);
    bowIOConnect->dcMotorControl->setMinSpeedHz(bowIOConnect->dcMotorControl->getAverageTachometerFreq());
//    debugPrintln("Min speed " + String(calibrationData->minSpeedPWM) + " at frequency " + String(calibrationData->minHz) + "Hz", TextInfo);
    debugPrintln("Min speed " + String(bowIOConnect->dcMotorControl->getMinSpeedPWM()) + " at frequency " + String(bowIOConnect->dcMotorControl->getMinSpeedHz()) + "Hz", TextInfo);

    debugPrintln("Finding max speed", TextInfo);

    //bowIOConnect->bowMotorWattage = float(bowIOConnect->bowMotorWattage * maxPowerUseMultiplierDuringCalibration);
    bowIOConnect->dcMotorControl->setMotorMaxPower(bowIOConnect->dcMotorControl->getMotorMaxPower() * maxPowerUseMultiplierDuringCalibration);
//    debugPrintln("Setting calibration max power to " + String(bowIOConnect->bowMotorWattage) + " watts", TextInfo);
    debugPrintln("Setting calibration max power to " + String(bowIOConnect->dcMotorControl->getMotorMaxPower()) + " watts", TextInfo);
    do {
//        bowIOConnect->setSpeedPWMSafe(i);
        bowIOConnect->dcMotorControl->setSpeedPWMSafe(i);
        delayMicroseconds(100);
        i+=10;
//        debugPrintln("Motor power use " + String(bowIOConnect->getBowCurrent() * bowIOConnect->getBowMotorVoltage()) + " watts at PWM " + String(i), TextInfo);
        debugPrintln("Motor power use " + String(bowIOConnect->dcMotorControl->getMotorCurrent() * bowIOConnect->dcMotorControl->getMotorVoltage()) + " watts at PWM " + String(i), TextInfo);
//    } while ((i < 65535) && (!bowIOConnect->bowOverPower()));
    } while ((i < 65535) && (!bowIOConnect->dcMotorControl->isOverPower()));
    if (i > 65535) { i = 65535; }
    //bowIOConnect->bowMotorWattage = bowMotorWattage;
    bowIOConnect->dcMotorControl->setMotorMaxPower(bowMotorPowerLimit);

    if (i < 65540) {
//        calibrationData->maxSpeedPWM = i;
        bowIOConnect->dcMotorControl->setMaxSpeedPWM(i);
        //debugPrintln("Motor current limited at " + String(bowIOConnect->bowMotorWattage * maxPowerUseMultiplierDuringCalibration) + " watts", TextInfo);
        debugPrintln("Motor current limited at " + String(bowIOConnect->dcMotorControl->getMotorMaxPower() * maxPowerUseMultiplierDuringCalibration) + " watts", TextInfo);
    }
    delay(500);
    if (!waitForBowToStabilize()) { goto skiptoend; }
//    if (bowIOConnect->getAverageTachometerFreq() < 1) { goto skiptoend; }
//    calibrationData->maxHz = bowIOConnect->getAverageTachometerFreq();
//    calibrationData->maxSpeedPWM = i;
    if (bowIOConnect->dcMotorControl->getAverageTachometerFreq() < 1) { goto skiptoend; }
    bowIOConnect->dcMotorControl->setMaxSpeedHz(bowIOConnect->dcMotorControl->getAverageTachometerFreq());
    bowIOConnect->dcMotorControl->setMaxSpeedPWM(i);

    // Start at 65535 and decrease PWM until the average frequency is below the recorded max hz, record this as maxSpeedPWM

//    debugPrintln("Max speed " + String(calibrationData->maxSpeedPWM) + " at frequency " + String(calibrationData->maxHz) + "Hz", TextInfo);
    debugPrintln("Max speed " + String(bowIOConnect->dcMotorControl->getMaxSpeedPWM()) + " at frequency " + String(bowIOConnect->dcMotorControl->getMaxSpeedHz()) + "Hz", TextInfo);

skiptoend:
//    bowIOConnect->setSpeedPWMSafe(0);
//    bowIOConnect->setTiltPWM(0);
    bowIOConnect->dcMotorControl->setSpeedPWMSafe(0);
    bowIOConnect->bowPressure->setHardwarePressure(0);

    bowControlConnect->run = 0;
    bowControlConnect->PIDon = PIDOn;

    return true;
}
/*
bool calibrate::findMinMaxSpeedPID() {
    debugPrintln("Finding min PID speed", TextInfo);

    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(0);
    bowIOConnect->waitForTiltToComplete();

    float targetFreq = calibrationData->minHz * 3;

    bowIOConnect->enableBowPower();
    bowControlConnect->setPIDTarget(targetFreq);
    _speedMode savedSpeedMode = bowControlConnect->speedMode;
    bowControlConnect->speedMode = _speedMode::Manual;
    bowControlConnect->run = 1;
    bowIOConnect->clearTachoData();
    delay(1500);

    uint16_t testStepDelay = 100;

    // Testing for stability of initial PID stability
    float readFreq = 0;
    do {
        readFreq = 0;
        for (int i=0; i<25; i++) {
            delay(1);
            readFreq += bowIOConnect->getAverageTachometerFreq();
        }
        readFreq = (readFreq / 25);
        debugPrintln("Testing freq " + String(targetFreq) + " read frequency " + String(readFreq), TextInfo);

        if (!((readFreq >= (targetFreq - 1)) && (readFreq <= (targetFreq + 1)))) {
            break;
        }
        targetFreq -= 1;

        bowControlConnect->setPIDTarget(targetFreq);
        delay(testStepDelay);
    } while (targetFreq < calibrationData->maxHz);

    calibrationData->minHz = readFreq;
    debugPrintln("Minimum PID frequency found at " + String(calibrationData->minHz) + " Hz", TextInfo);
    delay(500);

    bowIOConnect->clearTachoData();
    targetFreq = calibrationData->maxHz;
    bowControlConnect->setPIDTarget(targetFreq);
    // Testing for stability of initial PID stability
    readFreq = 0;
    do {
        readFreq = 0;
        for (int i=0; i<25; i++) {
            delay(1);
            readFreq += bowIOConnect->getAverageTachometerFreq();
        }
        readFreq = (readFreq / 25);
        debugPrintln("Testing freq " + String(targetFreq) + " read frequency " + String(readFreq), TextInfo);

        if ((readFreq >= (targetFreq - 1)) && (readFreq <= (targetFreq + 1))) {
            break;
        }
        targetFreq -= 1;

        bowControlConnect->setPIDTarget(targetFreq);
        delay(testStepDelay);
    } while (targetFreq > calibrationData->minHz);

    calibrationData->maxHz = readFreq;
    debugPrintln("Maximum pid frequency with no pressure found at " + String(calibrationData->maxHz) + " Hz", TextInfo);
    delay(500);

    bowIOConnect->setTiltPWM(calibrationData->stallPressure);
    bowIOConnect->waitForTiltToComplete();
    readFreq = 0;
    do {
        readFreq = 0;
        for (int i=0; i<25; i++) {
            delay(1);
            readFreq += bowIOConnect->getAverageTachometerFreq();
        }
        readFreq = (readFreq / 25);
        debugPrintln("Testing freq " + String(targetFreq) + " read frequency " + String(readFreq), TextInfo);

        if ((readFreq >= (targetFreq - 1)) && (readFreq <= (targetFreq + 1))) {
            break;
        }
        targetFreq -= 1;

        bowControlConnect->setPIDTarget(targetFreq);
        delay(testStepDelay);
    } while (targetFreq > calibrationData->minHz);

    calibrationData->maxHzPIDFullPressure = readFreq;
    debugPrintln("Maximum pid frequency with pressure " + String(calibrationData->maxHzPIDFullPressure) + " Hz", TextInfo);
    delay(500);

    bowControlConnect->speedMode = savedSpeedMode;
    bowControlConnect->run = 0;


    return true;
}
*/
String calibrate::dumpData() {
    String dump = "";
    dump += "bmsi:" + String(calibrationData->minHz) + ",";
    dump += "bmsx:" + String(calibrationData->maxHz) + ",";
//    dump += "bppe:" + String(calibrationData->firstTouchPressure) + ",";
//    dump += "bppx:" + String(calibrationData->stallPressure) + ",";
//    dump += "bppr:" + String(calibrationData->restPosition) + ",";
    dump += "bcu:" + String(calibrationData->fundamentalFrequency) + ",";
    return dump;
}

/// Perform all calibration tests
bool calibrate::calibrateAll() {
//    if (!bowIOConnect->homeBow()) {
    if (!bowIOConnect->bowPressure->home()) {
        debugPrintln("Couldn't home bowing jack", Error);
        return false;
    }

    if (!findMinMaxSpeedPWM()) {
        debugPrintln("findMinMaxSpeedPWM FAILED!", Error);
        return false;
    }

    if (!findMinPressure()) {
        debugPrintln("findMinPressure FAILED!", Error);
        return false;
    }

    if (!findMaxPressure()) {
        debugPrintln("findMaxPressure FAILED!", Error);
        return false;
    }

    debugPrintln("Calibrations done", InfoRequest);

    return true;
}

#endif
