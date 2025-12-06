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

#include "bowcontrol.cpp"
#include "bowcalibration.hpp"

/// Class constructor. Binds to the associated bowIO class and reads any existing EEPROM data into calibrationData
CalibrateBow::CalibrateBow(BowControl *inBowControl) {
    bowControl = inBowControl;
}


/** \brief Waits for the bow speed to stabilize, used after setting a new bow speed
 *
 *  Goes through maxIterations number of iterations with a preset delay in between each delay.
 *  If bow speed has not stabilized after the max amount of iterations the function returns false otherwise it will return true
 */
bool CalibrateBow::waitForBowToStabilize(uint16_t maxIterations = 10) {
    int average;
    int iterations = 0;
    int newAverage = bowControl->getAverageTachometerFreq();
    do {
        average = newAverage;
        delay(250);
        bowControl->checkTachometerTimeout();
        newAverage = bowControl->getAverageTachometerFreq();
        iterations++;
        if (iterations > maxIterations) {
            debugPrintln("Bow did not stabilize within the given iterations", Priority);
            return false;
        }
    } while (average != newAverage);
    return true;
}

/* \brief Find the minimum pressure where the bow first hits the string aka where a recorded slow-down in bow speed happens using the minSpeedPWM as the metric
 *
 */
eCalibrationResult CalibrateBow::findMinPressure() {
    debugPrintln("Finding min contact pressure for minimum usable speed", TextInfo);

    uint16_t minPWM = bowControl->getBowMotorMinPWM();
    if (minPWM <= 0) { return CR_Err_MotorNotCalibrated; }
    bowControl->setBowMotorDirectPWM(minPWM);

    bowControl->setHardwarePressure(0);
    if (!bowControl->waitForPressureToSettle()) { return CR_Err_BowPressure; }
    if (!waitForBowToStabilize()) { return CR_Err_BowStabilize; }
    delay(500);

    float initialMotorFreq = 0;
    int tempTimeOut = 0;
    do {
        initialMotorFreq = bowControl->getAverageTachometerFreq();
        delayMicroseconds(10);
        tempTimeOut++;
    } while((initialMotorFreq <= 0) && (tempTimeOut<100));
    if (initialMotorFreq <= 0) { return CR_Err_MotorFrequencyReading; }
    debugPrintln("Starting freq " + String(initialMotorFreq) + String("Hz"), TextInfo);

    uint16_t i;
    int average;
    for (i =0; i<maxTestPressure; i+=10) {
        bowControl->setHardwarePressure(i);
        if (!bowControl->waitForPressureToSettle()) { return CR_Err_BowPressure; }

        do {
            average = bowControl->getAverageTachometerFreq();
        } while (average <= 0);

        if (bowControl->getAverageTachometerFreq() < (initialMotorFreq - deviation)) {
            break;
        }
    }
    if (bowControl->getAverageTachometerFreq() <= 0) { return CR_Err_MotorFrequencyReading; }
    debugPrintln("intial contact found at "  + String(i) + " frequency " + String(bowControl->getAverageTachometerFreq()) + "Hz", TextInfo);

    if (i > pressureTestRetract) {
        i -= pressureTestRetract;
    } else {
        i = 0;
    }

    bowControl->setBowMotorDirectPWM(0);
    bowControl->setHardwarePressure(i);
    if (!bowControl->waitForPressureToSettle()) { return CR_Err_BowPressure; }
    debugPrintln("starting over at PWM " + String(i), TextInfo);

    bowControl->setBowMotorDirectPWM(bowControl->getBowMotorMinPWM());
    if (!waitForBowToStabilize()) { return CR_Err_BowStabilize; }

    bool fault = false;

    bowControl->setHardwarePressure(i);
    if (!bowControl->waitForPressureToSettle()) { return CR_Err_BowPressure; }

    do {
        bowControl->setHardwarePressure(i);
        if (!bowControl->waitForPressureToSettle(1)) {
            fault = true;
            break;
        }

        delayMicroseconds(100);
        if (bowControl->getAverageTachometerFreq() < (initialMotorFreq - deviation)) {
            break;
        }
        i++;
    } while(1 != 2);
    if ((bowControl->getAverageTachometerFreq() <= 0) || (fault)) { return CR_Err_MotorFrequencyReading; }

    bowControl->setEngagePressure(i);

    debugPrintln("Final contact PWM " + String(i), TextInfo);
    debugPrintln(" at frequency " + String(bowControl->getAverageTachometerFreq()) + "Hz", TextInfo);

    //bowControl->setBowMotorDirectPWM(0);
    //bowControl->setHardwarePressure(0);

    return safeExitWithResult(CR_Ok);
}

/** \brief Find the minimum and maximum usable pressure
 * Maximum pressure is calculated as the pressure just before the bow motor stalls against the string using maxSpeedPWM as the metric
 */
eCalibrationResult CalibrateBow::findMaxPressure() {
    debugPrintln("Finding max contact", TextInfo);

    bowControl->setBowMotorDirectPWM(bowControl->getBowMotorMinPWM() + (bowControl->getBowMotorMaxPWM() - bowControl->getBowMotorMinPWM()) / 2);
    bowControl->setHardwarePressure(0);
    if (!bowControl->waitForPressureToSettle()) { return CR_Err_BowPressure; }

    delay(500);
    if (!waitForBowToStabilize(100)) { return CR_Err_BowStabilize; }

    float initialMotorFreq = bowControl->getAverageTachometerFreq();
    if (initialMotorFreq == 0) { return CR_Err_MotorFrequencyReading; }

    int16_t j = bowControl->getEngagePressure();

    if (j > pressureTestRetract) {
        j -= pressureTestRetract;
    } else {
        j = 0;
    }

    eCalibrationResult calibrationResult = CR_Ok;

    bowControl->setHardwarePressure(j);
    if (bowControl->waitForPressureToSettle(5000)) {
        debugPrintln("Starting freq " + String(initialMotorFreq) + "Hz", TextInfo);

        uint16_t i;
        float average;
        bool fault = false;

        for (i =j; i<maxTestPressure; i+=1) {
            bowControl->setHardwarePressure(i);
            if (!bowControl->waitForPressureToSettle(5000)) { return CR_Err_BowPressure; }

            delayMicroseconds(100);
            average = bowControl->getAverageTachometerFreq();
            bowControl->checkTachometerTimeout();

            if ((average == 0) || bowControl->bowMotorIsOverPower() || bowControl->bowMotorIsOverCurrent()) {
                fault = true;
                break;
            }
        }

        bowControl->setMaxPressure(i);
        if (!fault) {
            debugPrintln("Final contact PWM " + String(i) + " at frequency " + String(bowControl->getAverageTachometerFreq()) + "Hz", TextInfo);
        } else {
            calibrationResult = CR_Err_MotorFault;
        }
    } else {
        calibrationResult = CR_Err_BowPressure;
    }

    bowControl->setBowMotorDirectPWM(0);
    bowControl->setHardwarePressure(0);
    delay(1000);
    bowControl->home();

    return safeExitWithResult(calibrationResult);
}

eCalibrationResult CalibrateBow::findMinMaxPressure() {
    bool autoCorrectPositionSave = bowControl->getPressureAutocorrect();
    bowControl->setPressureAutocorrect(false);

    bowControl->setSpeedMode(eSpeedMode::Manual);
    bool PIDOn = bowControl->getPIDOn();
    bowControl->setPIDOn(false);
    bowControl->enableBowMotorPower();

    float speed = bowControl->getPressureMotorSpeed();
    bowControl->setPressureMotorSpeed(5);

    eCalibrationResult result = findMinPressure();
    if (result == CR_Ok) { result = findMaxPressure(); }

    if (result == CR_Ok) {
        if (bowControl->getEngagePressure() > 3000) {
            bowControl->setEngagePressure(bowControl->getEngagePressure() - 3000);
        } else {
            bowControl->setEngagePressure(0);
        }

        if (bowControl->getEngagePressure() > 3000) {
            bowControl->setRestPressure(bowControl->getEngagePressure() - 3000);
        } else {
            bowControl->setRestPressure(0);
        }
        debugPrintln("Good results acquired, setting firstTouchPressure to " + String(bowControl->getEngagePressure()) + " and restPosition to " +
                     String(bowControl->getRestPressure()), debugPrintType::Debug);
    }/* else {
        debugPrintln("Results invalid, not modifying data" , debugPrintType::Error);
    }*/

    bowControl->setPressureAutocorrect(autoCorrectPositionSave);
    bowControl->setPressureMotorSpeed(speed);

    return safeExitWithResult(result);
}

eCalibrationResult CalibrateBow::safeExitWithResult(eCalibrationResult result, bool pidState) {
    bowControl->setBowMotorDirectPWM(0);
    bowControl->setHardwarePressure(0);
    bowControl->setRun(false);
    bowControl->setPIDOn(pidState);
    bowControl->setSpeedMode(eSpeedMode::Automatic);

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
eCalibrationResult CalibrateBow::findMinMaxSpeedPWM() {
    int speed = 0;
    uint32_t i =0;
    float avgFreq = 0;
    eCalibrationResult result = CR_Ok;

    float bowMotorPowerLimit = bowControl->getBowMotorMaxPower();

    bool PIDOn = bowControl->getPIDOn();
    bowControl->setPIDOn(false);
    bowControl->enableBowMotorPower();

    debugPrintln("Finding min speed", TextInfo);
    bowControl->setBowMotorDirectPWM(0);
    bowControl->setHardwarePressure(i);
    if (!bowControl->waitForPressureToSettle()) { return safeExitWithResult(CR_Err_BowPressure, PIDOn); }

    // Go from 0 PWM to 65535 and break the first time that the frequency is above zero
    do {
        bowControl->setBowMotorDirectPWM(speed);
        delayMicroseconds(1000);
        speed++;
    } while ((bowControl->getAverageTachometerFreq() < minPermissibleBowSpeedForCalibration) && (speed < 65536));


    delay(500);
    if (!waitForBowToStabilize() || (speed == 65536)) { return safeExitWithResult(CR_Err_BowStabilize, PIDOn); }

    bowControl->setBowMotorMinPWM(speed);
    delay(1000);
    bowControl->setBowMotorMinHz(bowControl->getAverageTachometerFreq());
    debugPrintln("Min speed is " + String(bowControl->getBowMotorMinHz()) + "Hz given at PWM " + String(bowControl->getBowMotorMinPWM()), TextInfo);

    debugPrintln("Finding max speed", TextInfo);

    bowControl->setBowMotorMaxPower(bowControl->getBowMotorMaxPower() * maxPowerUseMultiplierDuringCalibration);

    debugPrintln("Setting calibration max power to " + String(bowControl->getBowMotorMaxPower()) + " watts", TextInfo);
    do {
        bowControl->setBowMotorDirectPWM(i);
        delayMicroseconds(100);
        i+=10;
        //debugPrintln("Motor power use " + String(bowControl->getBowMotorPower()) + " watts at PWM " + String(i), TextInfo);

    } while ((i < 65535) && (!bowControl->bowMotorIsOverPower()));
    if (i > 65535) { i = 65535; }

    bowControl->setBowMotorMaxPower(bowMotorPowerLimit);

    bowControl->setBowMotorMaxPWM(i);
    if (i < 65540) { debugPrintln("Motor current limited at " + String(bowControl->getBowMotorMaxPower()) + " watts", TextInfo); }
    delay(500);
    if (!waitForBowToStabilize()) { return safeExitWithResult(CR_Err_BowStabilize, PIDOn); }

    avgFreq = bowControl->getAverageTachometerFreq();

    if (avgFreq < minPermissibleBowSpeedForCalibration) { return safeExitWithResult(CR_Err_MotorFrequencyReading, PIDOn); }
    bowControl->setBowMotorMaxHz(avgFreq);

    debugPrintln("Max speed " + String(bowControl->getBowMotorMaxHz()) + "Hz given at PWM " + String(bowControl->getBowMotorMaxPWM()), TextInfo);

    return safeExitWithResult(result);
}
/*
bool CalibrateBow::findMinMaxSpeedPID() {
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
String CalibrateBow::dumpData() {
    String dump = "";
    return dump;
}

/// Perform all calibration tests
bool CalibrateBow::calibrateAll() {
    if (!bowControl->home()) {
        debugPrintln("Couldn't home bowing jack", Error);
        return false;
    }

    eCalibrationResult result;

    result = findMinMaxSpeedPWM();
    if (result != CR_Ok) { return exitWithError(result); }

    result = findMinMaxPressure();
    if (result != CR_Ok) { return exitWithError(result); }

    debugPrintln("Calibrations done", InfoRequest);

    return true;
}


#endif
