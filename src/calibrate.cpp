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
calibrate::calibrate(bowIO &_bowIO, _calibrationData &__calibrationData, bowControl &_bowControl) {
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

        //delay(testDelay);
        do {
            average = bowIOConnect->averageFreq();
        } while (average <= 0);

        if (bowIOConnect->averageFreq() < (initialMotorFreq - deviation)) {
            break;
        }
    }
    if (bowIOConnect->averageFreq() <= 0) { return false; }
    debugPrintln("intial contact found at "  + String(i) + " frequency " + String(bowIOConnect->averageFreq()) + "Hz", TextInfo);
    i -= 7500;
    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(i);
    bowIOConnect->waitForTiltToComplete();
    debugPrintln("starting over at PWM " + String(i), TextInfo);

    //delay(2500);
    bowIOConnect->setSpeedPWMSafe(calibrationData->minSpeedPWM);
    //delay(2500);
    if (!waitForBowToStabilize()) { return false; }

    do {
        bowIOConnect->setTiltPWM(i);
        bowIOConnect->waitForTiltToComplete();

        delayMicroseconds(100);
        if (bowIOConnect->averageFreq() < (initialMotorFreq - deviation)) {
            break;
        }
        i++;
    } while(1 != 2);
    if (bowIOConnect->averageFreq() <= 0) { return false; }

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

    bowIOConnect->setSpeedPWMSafe(calibrationData->maxSpeedPWM);
    bowIOConnect->setTiltPWM(0);
    bowIOConnect->waitForTiltToComplete(5000);
    delay(500);
    if (!waitForBowToStabilize(100)) { return false; }
    float initialMotorFreq = bowIOConnect->averageFreq();
    if (initialMotorFreq == 0) { return false; }

    debugPrintln("Starting freq " + String(initialMotorFreq) + "Hz", TextInfo);

    uint16_t i;
    float average;
//    float maxCurrentRead = 0, tempCurrent;

    for (i =calibrationData->firstTouchPressure; i<maxTestPressure; i+=1) {
        bowIOConnect->setTiltPWM(i);
        if (!bowIOConnect->waitForTiltToComplete(1000)) {
            debugPrintln("Tilt didn't finish", Debug);
            break;
        }
        delayMicroseconds(100);
        average = bowIOConnect->averageFreq();
        bowIOConnect->checkTimeout();

//        tempCurrent = bowIOConnect->getBowCurrent();
//        if (tempCurrent > maxCurrentRead) { maxCurrentRead = tempCurrent; }

        debugPrintln("Tilt " + String (i) + " Average frequency " + String(average) + " power " + String(bowIOConnect->getBowCurrent() * bowIOConnect->getBowMotorVoltage())
            + ", current " + String(bowIOConnect->getBowCurrent()) + "A, motor fault " + String(bowIOConnect->getMotorFault()), Debug);

        if ((average == 0) || bowIOConnect->bowOverPower() || bowIOConnect->bowOverCurrent()) {
            break;
        }
    }

    calibrationData->stallPressure = i;

    debugPrintln("Final contact PWM " + String(calibrationData->stallPressure) + " at frequency " + String(bowIOConnect->averageFreq()) + "Hz"/* with max current " +
        String(maxCurrentRead)*/, TextInfo);

    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(0);

    return true;
}

bool calibrate::findMinMaxPressure() {
    bool autoCorrectPositionSave = bowIOConnect->stepServoStepper->autoCorrectPosition;
    bowIOConnect->stepServoStepper->autoCorrectPosition = false;

    uint16_t speed = bowIOConnect->stepServoStepper->speedRPM;
    bowIOConnect->stepServoStepper->setSpeed(5);


    bool result = findMinPressure() && findMaxPressure();

    bowIOConnect->stepServoStepper->autoCorrectPosition = autoCorrectPositionSave;
    bowIOConnect->stepServoStepper->setSpeed(speed);
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
/*        for (i=speed;i>0;i--) {
            bowIOConnect->setSpeedPWMSafe(i);
            delayMicroseconds(100);
            if (bowIOConnect->averageFreq() < 1) { goto skiptoend; }
            if (bowIOConnect->averageFreq() < (calibrationData->maxHz - 1)) {
                calibrationData->maxSpeedPWM = i;
                break;
            }
        }*/
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

/*    if (!finished) {
        restore all data here
    }*/

skiptoend:
    bowIOConnect->setSpeedPWMSafe(0);
    bowIOConnect->setTiltPWM(0);

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

    //bowIOConnect->clearTachoData();
    //targetFreq = calibrationData->maxHz;
    //bowControlConnect->setPIDTarget(targetFreq);
    // Testing for stability of initial PID stability
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
    dump += "bis:" + String(calibrationData->minHz) + ",";
    dump += "bxs:" + String(calibrationData->maxHz) + ",";
    dump += "bip:" + String(calibrationData->firstTouchPressure) + ",";
    dump += "bxp:" + String(calibrationData->stallPressure) + ",";
    dump += "brp:" + String(calibrationData->restPosition) + ",";
    dump += "u:" + String(calibrationData->fundamentalFrequency) + ",";
//    dump += "biiw:" + String(calibrationData->minInertialPWM + ",");
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

/*
void calibrate::updateWhileDelayUs(int us) {
//  bowControlConnect->running = 1;
  for (int i=0; i<us; i++) {
    bowControlConnect->updateString();
    delayMicroseconds(1);
  }
}
*/
/** \brief Waits for the bow speed to stabilize, used after setting a new bow speed
 *
 *  Goes through maxIterations number of iterations with a preset delay in between each delay.
 *  If bow speed has not stabilized after the max amount of iterations the function returns false otherwise it will return true
 */
/*bool calibrate::waitForBowToStabilizePID(uint16_t maxIterations = 10) {
  int average;
  int iterations = 0;
  int newAverage = bowIOConnect->averageFreq();
  do {
    average = newAverage;
    updateWhileDelayUs(250);
    newAverage = bowIOConnect->averageFreq();
    iterations++;
    if (iterations > maxIterations) {
      debugPrintln("Bow did not stabilize within the given iterations", Debug);
      return false;
    }
  } while (average != newAverage);
//#ifdef stabilizeDebug
  debugPrintln("Bow settled at ~" + String(newAverage) + "Hz, last value ~" + String(average) + "Hz", Debug);
//#endif
  return true;
}
*/
/*
/// Ramps up the bow speed until a given speed has been reached, uses a preset delay
int calibrate::increaseSpeedPWMToTarget(uint16_t startPWM, float target) {
  do {
    startPWM++;
    bowIOConnect->setSpeedPWMSafe(startPWM);
    delay(1);
    if (startPWM>65535) {
      startPWM = -1;
      break;
    }
  } while (bowIOConnect->averageFreq() < target);
  return startPWM;
}
*/
/** \brief Attempts to reach a target bow speed using a derivative approach
 *
 */
/*
int calibrate::derivativeSpeedPWMToTarget(int startPWM, float target, uint16_t maxIterations = 20) {
  int iterations = 0;
  float averagePWMpHz = (calibrationData->maxSpeedPWM - calibrationData->minSpeedPWM) / (calibrationData->maxHz - calibrationData->minHz);
#ifdef derivativeDebug
  debugPrint("Average PWM / Hz is "); debugPrintln(averagePWMpHz);
#endif
  bowIOConnect->setSpeedPWMSafe(startPWM);
  delay(100);
  int errorCorrectionProportional;
  float multiplierProportional = 0.8; //K
  float tempFreq;
  do {
    tempFreq = bowIOConnect->averageFreq();
#ifdef derivativeDebug
    debugPrintln("PWM is " + String(startPWM) + " and frequency is " + String(tempFreq) + "Hz");
#endif
    float error = target - tempFreq;
    errorCorrectionProportional = error * averagePWMpHz;
#ifdef derivativeDebug
    debugPrintln("Target frequency of " + String(target) + "Hz gives " + String(error) + "Hz of error and we will correct with " + String(errorCorrectionProportional) + "steps", Debug);
#endif
    startPWM += ((double) errorCorrectionProportional * multiplierProportional);
    if (startPWM > 65535) {
      startPWM = -1;
      break;
    }

    bowIOConnect->setSpeedPWMSafe(startPWM);
    if (!waitForBowToStabilize(10)) {
      startPWM = -2;
      break;
    };

    iterations++;
    if (iterations >= maxIterations) {
      startPWM = -2;
#ifdef deriviateDebug
      debugPrint("Couldn't hit target frequency in " + String(maxIterations) + " iterations, exiting", Debug);
#endif
      break;
    }
  } while ((tempFreq - deviation > target) || (tempFreq + deviation < target));

  return startPWM;
}
*/

/*
bool calibrate::findMinMaxSpeedPID() {

  stringControlConnect->running = 1;
  stringControlConnect->setPIDTargetUnsafe(0);
  delay(250);

  if (!waitForBowToStabilize()) { return false; }
  delay(250);

  float j;
  for (j=minTestFreq; j<maxTestFreq; j+=0.1) {
    //stringControlConnect->pidTargetSpeed = j;
    stringControlConnect->setPIDTargetUnsafe(j);
    delayMicroseconds(500);
    if (bowStringConnect->averageFreq() > 0) {
      break;
    }
  }
  stringControlConnect->running = 0;

  return true;
}
*/
/** \brief Attempt at finding the fundamental frequency of the string by recording a spot where the bow speed doesn't noticably change regardless of increase in PWM value fed to the motor controller
 *
 */
/*
bool calibrate::findFundamentalFrequency() {
  bowIOConnect->setTiltPWM(0);
  bowIOConnect->setSpeedPWMSafe(0);
  delay(500);
  if (!waitForBowToStabilize(100)) { return false; }

  debugPrintln("Finding test pressure point", InfoRequest);
  bowIOConnect->setSpeedPWMSafe(calibrationData->minUsableSpeedPWM);
  delay(500);
  uint16_t testPWM = (calibrationData->maxUsablePressure * 0.25  + calibrationData->minUsablePressure * 1.75) / 2;
  do {
    bowIOConnect->setTiltPWM(testPWM);
    derivativeSpeedPWMToTarget(calibrationData->minUsableSpeedPWM, calibrationData->minUsableHz, 100);
    if (bowIOConnect->averageFreq() <= 0) {
      debugPrintln("Bow stalled, decreasing", InfoRequest);
      testPWM -= 500;
    } else {
      debugPrintln("Pressure point found at " + String(testPWM), InfoRequest);
      break;
    }
  } while (testPWM > 500);
  if (testPWM <= calibrationData->minUsablePressure) {
    return false;
  }

  debugPrintln("Spooled up", InfoRequest);
  delay(100);

  #define testBufferSize 650
  #define resonanceDeviation 1
  float testBuffer[testBufferSize], sortBuffer[testBufferSize];
  int testBufferCounter = 0;

  struct _resonantFreqs {
    float freq = -1;
    int instances = 0;
  } resonantFreqs[12];
  int rfCount = 0;

  float currentPeak = 0;

  for (int i=calibrationData->minUsableSpeedPWM; i<calibrationData->maxUsableSpeedPWM; i++) {
    bowIOConnect->setSpeedPWMSafe(i);
    delay(1);

    testBuffer[testBufferCounter] = bowIOConnect->averageFreq();

    debugPrintln("Speed PWM of " + String(i) + " gives a frequency of " + String(testBuffer[testBufferCounter]) + "Hz", InfoRequest);

    testBufferCounter++;
    if (testBufferCounter > testBufferSize) { testBufferCounter = 0; }

    if ((i>testBufferSize) && (testBuffer[testBufferCounter] > 0)) {
      for (int i=0; i<testBufferSize; i++) { sortBuffer[i] = testBuffer[i]; }
      std::sort(sortBuffer, sortBuffer + testBufferSize);
      if (sortBuffer[testBufferSize - 1] - sortBuffer[0] < resonanceDeviation) {
        calibrationData->fundamentalFrequency = testBuffer[testBufferSize / 2];
        calibrationData->fundamentalFrequencySpeedPWM = i - (testBufferSize / 2);
        break;

        currentPeak = testBuffer[testBufferSize / 2];
        debugPrintln("Resonant frequency found at " + String(currentPeak) + "Hz", InfoRequest);
        debugPrintln("Currently at peak " + String(rfCount) + " with instance " + String(resonantFreqs[rfCount].instances), InfoRequest);
        if ((rfCount == 0) && (resonantFreqs[rfCount].instances == 0)) {
          resonantFreqs[rfCount].freq = currentPeak;
          resonantFreqs[rfCount].instances == 1;
          debugPrint("Creating first resonant peak at 0 with frequency " + String(resonantFreqs[rfCount].freq), InfoRequest);
        } else
        if ((currentPeak > resonantFreqs[rfCount].freq  - resonanceDeviation) && (currentPeak < resonantFreqs[rfCount].freq  + resonanceDeviation)) {
          resonantFreqs[rfCount].instances += 1;
          debugPrint("Current peak at " + String(currentPeak) + "Hz is within range of " + String(resonantFreqs[rfCount].freq) + " setting instance to "
            + String(resonantFreqs[rfCount].instances), InfoRequest);
        } else {
          rfCount++;
          resonantFreqs[rfCount].freq = currentPeak;
          resonantFreqs[rfCount].instances == 1;
          debugPrint("New peak at " + String(currentPeak) + "Hz", InfoRequest);
        }
      }
    }
  }
  if (bowIOConnect->averageFreq() <= 0) { return false; }

  debugPrintln("Test done, peak found at " + String(calibrationData->fundamentalFrequency) +"Hz", InfoRequest);

  bowIOConnect->setTiltPWM(0);
  bowIOConnect->setSpeedPWMSafe(0);

  return true;
}
*/
/*
/// Save calibration data into EEPROM
uint16_t calibrate::saveParams() {
//  eeprom_write_block(&calibrationData, 0, sizeof(calibrationData));
  //eeprom_write_block(calibrationData, EEPROM_offset, sizeof(*calibrationData));
  debugPrintln("Saving data at offset " + String(EEPROM_offset), Debug);

  eeprom_write_block(&calibrationData->minHz, EEPROM_offset, 4);
  eeprom_write_block(&calibrationData->minSpeedPWM, EEPROM_offset + 4, 2);
  eeprom_write_block(&calibrationData->maxHz, EEPROM_offset + 6, 4);
  eeprom_write_block(&calibrationData->maxSpeedPWM, EEPROM_offset + 10, 2);
  eeprom_write_block(&calibrationData->firstTouchPressure, EEPROM_offset + 12, 2);
  eeprom_write_block(&calibrationData->stallPressure, EEPROM_offset + 14, 2);
  eeprom_write_block(&calibrationData->fundamentalFrequency, EEPROM_offset + 16, 4);
  eeprom_write_block(&calibrationData->lowerHarmonic, EEPROM_offset + 20, 2);
  eeprom_write_block(&calibrationData->upperHarmonic, EEPROM_offset + 22, 2);
  eeprom_write_block(&calibrationData->restPosition, EEPROM_offset + 24, 2);

  return 26;  // \todo replace with calculated size
}

uint16_t calibrate::loadParams(uint8_t EEPROMVersion) {
  uint16_t size;

  if (EEPROMVersion == 0x02) {
    debugPrintln("Loading data from offset " + String(EEPROM_offset), Debug);

    eeprom_read_block(&calibrationData->minHz, EEPROM_offset, 4);
    eeprom_read_block(&calibrationData->minSpeedPWM, EEPROM_offset + 4, 2);
    eeprom_read_block(&calibrationData->maxHz, EEPROM_offset + 6, 4);
    eeprom_read_block(&calibrationData->maxSpeedPWM, EEPROM_offset + 10, 2);
    eeprom_read_block(&calibrationData->firstTouchPressure, EEPROM_offset + 12, 2);
    eeprom_read_block(&calibrationData->stallPressure, EEPROM_offset + 14, 2);
    eeprom_read_block(&calibrationData->fundamentalFrequency, EEPROM_offset + 16, 4);
    eeprom_read_block(&calibrationData->lowerHarmonic, EEPROM_offset + 20, 2);
    eeprom_read_block(&calibrationData->upperHarmonic, EEPROM_offset + 22, 2);
    eeprom_read_block(&calibrationData->restPosition, EEPROM_offset + 24, 2);

    size = 26;  // \todo replace with calculated size
  }

  //eeprom_read_block(calibrationData, EEPROM_offset, sizeof(*calibrationData));
  //debugPrintln("Data loaded: " + String(calibrationData->maxSpeedPWM), Debug);
  //debugPrintln("Remove the following line, its just for testing ", Error);
  //calibrationData->upperHarmonic = 49;
  //calibrationData->lowerHarmonic = -48;
  return size;
}
*/

/*
/// Print calibration data to serial port
void calibrate::printCalibrationData() {
  debugPrintln("Min/max speed " + String(calibrationData->minHz) + " / " + String(calibrationData->maxHz) + " Hertz", InfoRequest);
//  debugPrintln("Min/max usable speed " + String(calibrationData->minUsableHz) + " / " + String(calibrationData->maxUsableHz) + " Hertz", InfoRequest);
  debugPrintln("Min/max usable pressure " + String(calibrationData->firstTouchPressure) + " / " + String(calibrationData->stallPressure) + " PWM", InfoRequest);
  debugPrintln("Fundamental " + String(calibrationData->fundamentalFrequency), InfoRequest);
}
*/

/// Returns the maximum bow speed minus a preset value \todo arbitrary, replace with more sensible function. Also how is this different than maxUsable
/*float calibrate:: maxSpeed() {
  return calibrationData->maxHz - 30;
}
*/
/*
int calibrate::findLowerHarmonic() {
  float freq = calibrationData->fundamentalFrequency;
  int j = 0;
  while (freq > calibrationData->minHz) {
    freq /= 2;
    j--;
  }
  char i=0;
  for (i=0; i<12; i++) {
    if ((freq * harmonicSeriesList.series[currentHarmonicSeries].frequency[i]) >= calibrationData->minHz) { break; } //justSeries[i]
  }
  calibrationData->lowerHarmonic = (j * 12 + i);
  return (j * 12 + i);
}

int calibrate::findUpperHarmonic() {
  float freq = calibrationData->fundamentalFrequency;
  int j = 0;
  while (freq < calibrationData->maxHz) {
    freq *= 2;
    j++;
  }
  char i=0;
  for (i=11; i>-1; i--) {
    if ((freq - (freq * harmonicSeriesList.series[currentHarmonicSeries].frequency[i])) <= calibrationData->maxHz) { break; } //justSeries[i]
  }

  calibrationData->upperHarmonic = (j * 12 - i);
  return (j * 12 - i);
}
*/
#endif
