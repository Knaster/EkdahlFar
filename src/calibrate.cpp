#include "avr_functions.h"
#include <stdint.h>
#include <sys/types.h>
#include "core_pins.h"
#include <sys/_stdint.h>
#include "arm_math.h"

#ifndef CALIBRATE_C
#define CALIBRATE_C

#include "calibrate.h"

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
    int newAverage = bowIOConnect->averageFreq();
    do {
        average = newAverage;
        delay(250);
        bowIOConnect->checkTimeout();
        newAverage = bowIOConnect->averageFreq();
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

    bowIOConnect->setSpeedPWMSafe(calibrationData->minSpeedPWM);
    bowIOConnect->setTiltPWM(0);
    bowIOConnect->waitForTiltToComplete();
    if (!waitForBowToStabilize()) { return false; }
    delay(500);

    float initialMotorFreq = 0;
    int tempTimeOut = 0;
    do {
        initialMotorFreq = bowIOConnect->averageFreq();
        delayMicroseconds(10);
        tempTimeOut++;
    } while((initialMotorFreq <= 0) && (tempTimeOut<100));
    if (initialMotorFreq <= 0) { return false; }
    debugPrintln("Starting freq " + String(initialMotorFreq) + String("Hz"), TextInfo);

    int i;
    int average;
    for (i =0; i<maxTestPressure; i+=10) {
        bowIOConnect->setTiltPWM(i);
        bowIOConnect->waitForTiltToComplete();
        do {
            average = bowIOConnect->averageFreq();
        } while (average <= 0);

        if (bowIOConnect->averageFreq() < (initialMotorFreq - deviation)) {
            break;
        }
    }
    if (bowIOConnect->averageFreq() <= 0) { return false; }
    debugPrintln("intial contact found at "  + String(i) + " frequency " + String(bowIOConnect->averageFreq()) + "Hz", TextInfo);
    i -= pressureTestRetract;
    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(i);
    bowIOConnect->waitForTiltToComplete();
    debugPrintln("starting over at PWM " + String(i), TextInfo);

    bowIOConnect->setSpeedPWMSafe(calibrationData->minSpeedPWM);
    if (!waitForBowToStabilize()) { return false; }

    bool fault = false;

    do {
        bowIOConnect->setTiltPWM(i);
        if (!bowIOConnect->waitForTiltToComplete(100)) {
            fault = true;
            break;
        }

        delayMicroseconds(100);
        if (bowIOConnect->averageFreq() < (initialMotorFreq - deviation)) {
            break;
        }
        i++;
    } while(1 != 2);
    if ((bowIOConnect->averageFreq() <= 0) || (fault)) { return false; }

    calibrationData->firstTouchPressure = i;

    debugPrintln("Final contact PWM " + String(calibrationData->firstTouchPressure), TextInfo);
    debugPrintln(" at frequency " + String(bowIOConnect->averageFreq()) + "Hz", TextInfo);

    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(0);

    return true;
}

/** \brief Find the minimum and maximum usable pressure
 * Maximum pressure is calculated as the pressure just before the bow motor stalls against the string using maxSpeedPWM as the metric
 */
bool calibrate::findMaxPressure() {
    debugPrintln("Finding max contact", TextInfo);

//    bowControlConnect->run = 1;
    bowIOConnect->setSpeedPWMSafe( calibrationData->minSpeedPWM + (calibrationData->maxSpeedPWM - calibrationData->minSpeedPWM) / 2  );
    bowIOConnect->setTiltPWM(0);
    bowIOConnect->waitForTiltToComplete(5000);
    delay(500);
    if (!waitForBowToStabilize(100)) { return false; }
    float initialMotorFreq = bowIOConnect->averageFreq();
    if (initialMotorFreq == 0) { return false; }

    bowIOConnect->setTiltPWM((calibrationData->firstTouchPressure - pressureTestRetract));
    if (bowIOConnect->waitForTiltToComplete(5000)) {
        debugPrintln("Starting freq " + String(initialMotorFreq) + "Hz", TextInfo);

        uint16_t i;
        float average;
        bool fault = false;

        for (i =(calibrationData->firstTouchPressure - pressureTestRetract); i<maxTestPressure; i+=1) {
            bowIOConnect->setTiltPWM(i);
            if (!bowIOConnect->waitForTiltToComplete(100)) {
                debugPrintln("Tilt didn't finish", Debug);
                fault = true;
                break;
            }
            delayMicroseconds(100);
            average = bowIOConnect->averageFreq();
            bowIOConnect->checkTimeout();

    //        debugPrintln("Tilt " + String (i) + " Average frequency " + String(average) + " power " + String(bowIOConnect->getBowCurrent() * bowIOConnect->getBowMotorVoltage())
    //            + ", current " + String(bowIOConnect->getBowCurrent()) + "A, motor fault " + String(bowIOConnect->getMotorFault()), Debug);

            if ((average == 0) || bowIOConnect->bowOverPower() || bowIOConnect->bowOverCurrent()) {
                break;
            }
        }

        calibrationData->stallPressure = i;
        if (!fault) {
            debugPrintln("Final contact PWM " + String(calibrationData->stallPressure) + " at frequency " + String(bowIOConnect->averageFreq()) + "Hz"/* with max current " +
            String(maxCurrentRead)*/, TextInfo);
        } else {
            debugPrintln("Got Fault! Final contact PWM " + String(calibrationData->stallPressure) + " at frequency " + String(bowIOConnect->averageFreq()) + "Hz"/* with max current " +
            String(maxCurrentRead)*/, TextInfo);
    //        debugPrintln("Max pressure calibration ERROR!", debugPrintType::Error);
        }
    } else {
        debugPrintln("Couldn't get calibration start conditions", debugPrintType::Error);
    }

    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(0);
    delay(1000);
    bowIOConnect->homeBow();

    return true;
}

bool calibrate::findMinMaxPressure() {
    bool autoCorrectPositionSave = bowIOConnect->stepServoStepper->autoCorrectPosition;
    bowIOConnect->stepServoStepper->autoCorrectPosition = false;

    bool PIDOn = bowControlConnect->PIDon;
    bowControlConnect->PIDon = 0;
//    bowControlConnect->run = 1;
    bowIOConnect->enableBowPower();

    uint16_t speed = bowIOConnect->stepServoStepper->speedRPM;
    bowIOConnect->stepServoStepper->setSpeed(5);

    //bool result = findMinPressure() && findMaxPressure();
    bool result = findMinPressure();
    if (result) {
        result = result & findMaxPressure();
    }

    if (result) {
        if (calibrationData->firstTouchPressure > 3000) {
            calibrationData->firstTouchPressure -= 3000;
        } else {
            calibrationData->firstTouchPressure =  0;
        }

        if (calibrationData->firstTouchPressure > 3000) {
            calibrationData->restPosition = calibrationData->firstTouchPressure - 3000;
        } else {
            calibrationData->restPosition =  0;
        }
    }

    bowIOConnect->stepServoStepper->autoCorrectPosition = autoCorrectPositionSave;
    bowIOConnect->stepServoStepper->setSpeed(speed);
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
    bool finished = false;
    int speed = 0;
    uint32_t i =0;
    float bowMotorWattage = bowIOConnect->bowMotorWattage;

    bool PIDOn = bowControlConnect->PIDon;
    bowControlConnect->PIDon = 0;
//    bowControlConnect->run = 1;
    bowIOConnect->enableBowPower();

    debugPrintln("Finding min speed", TextInfo);
    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(0);
    bowIOConnect->waitForTiltToComplete();

    // Go from 0 PWM to 65535 and break the first time that the frequency is above zero
    do {
        bowIOConnect->setSpeedPWMSafe(speed);
        delayMicroseconds(1000);
        speed++;
    } while ((bowIOConnect->averageFreq() < minPermissibleBowSpeedForCalibration) && (speed < 65536));
    delay(500);
    if (!waitForBowToStabilize() || (speed == 65536)) { goto skiptoend; }

    calibrationData->minSpeedPWM = speed;
    calibrationData->minHz = bowIOConnect->averageFreq();
    debugPrintln("Min speed " + String(calibrationData->minSpeedPWM) + " at frequency " + String(calibrationData->minHz) + "Hz", TextInfo);

    debugPrintln("Finding max speed", TextInfo);

    bowIOConnect->bowMotorWattage = float(bowIOConnect->bowMotorWattage * maxPowerUseMultiplierDuringCalibration);
    debugPrintln("Setting calibration max power to " + String(bowIOConnect->bowMotorWattage) + " watts", TextInfo);
    do {
        bowIOConnect->setSpeedPWMSafe(i);
        delayMicroseconds(100);
        i+=10;
        debugPrintln("Motor power use " + String(bowIOConnect->getBowCurrent() * bowIOConnect->getBowMotorVoltage()) + " watts at PWM " + String(i), TextInfo);
    } while ((i < 65535) && (!bowIOConnect->bowOverPower()));
    if (i > 65535) { i = 65535; }
    bowIOConnect->bowMotorWattage = bowMotorWattage;

    if (i < 65540) {
        calibrationData->maxSpeedPWM = i;
        debugPrintln("Motor current limited at " + String(bowIOConnect->bowMotorWattage * maxPowerUseMultiplierDuringCalibration) + " watts", TextInfo);
    }
    delay(500);
    if (!waitForBowToStabilize()) { goto skiptoend; }
    if (bowIOConnect->averageFreq() < 1) { goto skiptoend; }
    calibrationData->maxHz = bowIOConnect->averageFreq();
    calibrationData->maxSpeedPWM = i;

    // Start at 65535 and decrease PWM until the average frequency is below the recorded max hz, record this as maxSpeedPWM

    debugPrintln("Max speed " + String(calibrationData->maxSpeedPWM) + " at frequency " + String(calibrationData->maxHz) + "Hz", TextInfo);

skiptoend:
    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(0);

    bowControlConnect->run = 0;
    bowControlConnect->PIDon = PIDOn;

    return true;
}

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
            readFreq += bowIOConnect->averageFreq();
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
            readFreq += bowIOConnect->averageFreq();
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
            readFreq += bowIOConnect->averageFreq();
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

String calibrate::dumpData() {
    String dump = "";
    dump += "bmsi:" + String(calibrationData->minHz) + ",";
    dump += "bmsx:" + String(calibrationData->maxHz) + ",";
    dump += "bppe:" + String(calibrationData->firstTouchPressure) + ",";
    dump += "bppx:" + String(calibrationData->stallPressure) + ",";
    dump += "bppr:" + String(calibrationData->restPosition) + ",";
    dump += "bcu:" + String(calibrationData->fundamentalFrequency) + ",";
    return dump;
}

/// Perform all calibration tests
bool calibrate::calibrateAll() {
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
