#ifndef DCMOTORCONTROL_C
#define DCMOTORCONTROL_C

#include "dcmotorcontrol.hpp"

DCMotorControl::DCMotorControl(char inMotorRevPin, char inMotorVoltagePin, char inMotorDCDCEnPin, char inTachoPin, char inCurrentSensePin, char inMotorFaultPin) {
    motorRevPin = inMotorRevPin;
    reflectorInterruptPin = inTachoPin;
    currentSensePin = inCurrentSensePin;
    motorFaultPin = inMotorFaultPin;
    motorDCDCEnPin = inMotorDCDCEnPin;
    motorVoltagePin = inMotorVoltagePin;

    pinMode(motorRevPin, OUTPUT);
    pinMode(reflectorInterruptPin, INPUT);
    pinMode(currentSensePin, INPUT);
    pinMode(motorFaultPin, INPUT);

    motorPWM = new Teensy_PWM(motorRevPin, 20000, 0);
    motorPWM->setResolution(BITDEPTH);

    pinMode(motorDCDCEnPin, OUTPUT);
    disableMotorPower();

    motorVoltagePWM = new Teensy_PWM(motorVoltagePin, 20000, 0);
    motorVoltagePWM->setResolution(16);
    setMotorVoltage(motorVoltage);

    analogWrite(motorRevPin, 1);
    setSpeedPWM(0);

    //attachInterrupt(digitalPinToInterrupt(inTachoPin), ((void*) (&(this->tachometerISRHandler))), CHANGE);
}

eProcessResult DCMotorControl::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsDCMotor, sizeof(serialCommandsDCMotor) / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsDCMotor, sizeof(serialCommandsDCMotor) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "bowmotorvoltage") {
        if (request) {
            commandResponses->push_back({ "bmv:" + String(motorVoltage), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMotorVoltage(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"bmv:" + String(motorVoltage), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorcurrent") {
        if (request) {
            commandResponses->push_back({ "bmc:" + String(getMotorCurrent()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowmotorcurrentlimit") {
        if (request) {
            commandResponses->push_back({ "bmcl:" + String(motorCurrentLimit), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMotorMaxCurrent(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"bmcl" + String(motorCurrentLimit), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorpowerlimit") {
        if (request) {
            commandResponses->push_back({ "bmpl:" + String(motorPowerLimit), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMotorMaxPower(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"bmpl:" + String(motorPowerLimit), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorfrequency") {
        if (request) {
            commandResponses->push_back({ "bmf:" + String(getAverageTachometerFreq()), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowmotoremergencystop") {
        if (request) {
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            emergencyDisable(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"bmes:" + inCommandItem->argument[0], InfoRequest});
            commandResponses->push_back({"EMERGENCY STOP! Cooling down for " + inCommandItem->argument[0] + " ms", Command});
        }
    } else
    if (inCommandItem->command == "bowmotorpwmmin") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMinSpeedPWM(inCommandItem->argument[0].toInt());
        }
        commandResponses->push_back({ "bmpi:" + String(minSpeedPWM), InfoRequest });
    } else
    if (inCommandItem->command == "bowmotorpwmmax") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMaxSpeedPWM(inCommandItem->argument[0].toInt());
        }
        commandResponses->push_back({ "bmpx:" + String(maxSpeedPWM), InfoRequest });
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

eProcessResult DCMotorControl::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {
    return eProcessResult::NotFound;
}

String DCMotorControl::dumpData() {
    String dump = "";
    dump += "bmv:" + String(motorVoltage) + ",";
    dump += "bmcl:" + String(motorCurrentLimit) + ",";
    dump += "bmpl:" + String(motorPowerLimit) + ",";
    dump += "bmpi:" + String(minSpeedPWM) + ",";
    dump += "bmpx:" + String(maxSpeedPWM);
    return dump;
}

// *** POWER SUPPLY FUNCTIONS

bool DCMotorControl::setMotorVoltage(float voltage) {
    motorVoltage = voltage;
    voltage -= DCDCLOWERBOUND;
    float t = (65536 / (DCDCUPPERBOUND - DCDCLOWERBOUND)) * voltage;
    uint16_t pwm = 65536 - ((uint16_t) t);
    analogWrite(motorVoltagePin, pwm);
    return true;
}

float DCMotorControl::getMotorVoltage() {
    return motorVoltage;
}

bool DCMotorControl::enableMotorPower() {
    if (emergencyCoolDown) {
        debugPrintln("Cooldown event at " + String(emergencyCoolDownEvent) + " with duration " + String(emergencyCoolDownPeriod) + ", current time " + String(millis()), Debug);
        if (millis() > (emergencyCoolDownEvent + emergencyCoolDownPeriod)) {
            debugPrintln("Cool down event passed, resetting", Debug);
            emergencyCoolDown = false;
        } else {
            debugPrintln("Cool down event in effect, ignoring", Debug);
            return false;
        }
    }
    digitalWrite(motorDCDCEnPin, 1);
    return true;
}

bool DCMotorControl::disableMotorPower() {
    digitalWrite(motorDCDCEnPin, 0);
    return true;
}

/** \brief Adds a tachometer value to the tachometer buffer, given in Hertz
 *
 *  Advances the tachoFreq index counter tachoFreqIndex and increases tachFreqCount if it has not yet reached tachoFreqLength.
 *  Values outside of the range defined by tachoFreqPermissibleMAX and tachoFreqPermissibleMIN are set to 0
 */

void DCMotorControl::addTachoFreq(float freq) {
    if ((freq >= tachoFreqPerimssibleMAX) || (freq <= tachoFreqPerimssibleMIN)) {
        freq = 0;
    }

    tachoFreq[tachoFreqIndex] = freq;
    tachoFreqIndex++;
    if (tachoFreqCount < tachoFreqLength) { tachoFreqCount++; }
    if (tachoFreqIndex >= tachoFreqLength) { tachoFreqIndex = 0; }
}

/// Returns the last added tachometer value
float DCMotorControl::getLastTachometerFreq() {
    if (tachoFreqIndex < 1) {
        return tachoFreq[tachoFreqLength - 1];
    } else {
        return tachoFreq[tachoFreqIndex - 1];
    }
}

void DCMotorControl::clearTachometerData() {
    tachoFreqCount = 0;
    tachoFreqIndex = 0;
}

/** \brief Returns the average bow speed frequency
 *
 *  - Calculates the average bow speed frequency using the values stored in the tachometer buffer by
 *    -# finding the median of the buffer
 *    -# recalculating the average skipping values straying from the median by a frequency given by permissibleFreqDeviation
 *
 */

float DCMotorControl::getAverageTachometerFreq() {

    if (tachoFreqCount < tachoFreqLength) {
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

    return average;
}

/// Secondary callback handler for tachometer events
void DCMotorControl::tachometerISRHandler() {
    asm("nop");
    uint8_t state = digitalRead(reflectorInterruptPin);
    if (state == lastReflectorISRState) { return; }
    if (state == 0) {
        reflectorCyclePeriod = reflectorCounter;
        reflectorCounter = 0;
    }
    addTachoFreq(((double) 1000000 / reflectorCyclePeriod));
    asm("dsb");
    lastReflectorISRState = state;
}

/// Ramps the bow speed PWM from the last PWM value given to the value given in speed, blocking, only use for testing purposes
void DCMotorControl::setSpeedPWMSafe(uint16_t speed) {
    int a = speed;
    // ramp the speed if going up, driver doesn't like sudden fast changes
    analogWrite(motorRevPin, 65535 - a);
    if (lastMotorPWM < a) {
        for (int i=lastMotorPWM; i<a; i++) {
            delayMicroseconds(5); //5
            analogWrite(motorRevPin, 65535 - i);
        };
    } else {
        for (int i=lastMotorPWM; i>a; i--) {
            delayMicroseconds(5); //5
            analogWrite(motorRevPin, 65535 - i);
        };
    }

    lastMotorPWM = a;
}

/// Sets the bow speed PWM using a unsigned 16-bit int (RAW)
void DCMotorControl::setSpeedPWM(uint16_t speed) {
    int a = speed;
    motorPWM->setPWM_manual(motorRevPin, (65535 - a) / BITDIV);
    lastMotorPWM = a;
}

uint16_t DCMotorControl::getSpeedPWM() {
    return lastMotorPWM;
}

/// Check whether bow has timed out and add a value of 0 Hertz if so
bool DCMotorControl::checkTachometerTimeout() {
    if (reflectorCounter > reflectorZeroTimeoutValue) {
        addTachoFreq(0);
        return true;
    } else {
        return false;
    }
}

void DCMotorControl::setMinSpeedPWM(uint16_t inSpeed) { minSpeedPWM = inSpeed; }

uint16_t DCMotorControl::getMinSpeedPWM() { return minSpeedPWM; };

void DCMotorControl::setMaxSpeedPWM(uint16_t inSpeed) { maxSpeedPWM = inSpeed; }

uint16_t DCMotorControl::getMaxSpeedPWM() { return maxSpeedPWM; }

void DCMotorControl::setMotorMaxPower(float watts) { motorPowerLimit = watts; }

float DCMotorControl::getMotorMaxPower() { return motorPowerLimit; }

//**** STATISTICS AND FAULT HANDLING

float DCMotorControl::getMotorCurrent() {
      return analogRead(currentSensePin) * 3.3 / 4096;
}

bool DCMotorControl::isOverCurrent() {
    if (getMotorCurrent() >= motorCurrentLimit) {
        return true;
    } else {
        return false;
    }
}

bool DCMotorControl::isOverPower() {
    if (getMotorCurrent() >= (motorPowerLimit / motorVoltage)) {
        // If we have already set the internal over-power flag
        if (transientOverPower) {
            // And that was set more than the allowed time-span ago, aka the over-power event has been going on for X ms
            if (lastOverPowerEvent > overPowerDuration) {
                // Signal the external over power flag
                overPowerFlag = true;
            }
        } else {
            // If this is a new over-power event set the flag and clear the event-time
            transientOverPower = true;
            lastOverPowerEvent = 0;
        }
        return true;
    } else {
        // Clear everything if we no longer have an over power event
        overPowerFlag = false;
        transientOverPower = false;
        return false;
    }
}

// Emergency disable of power with a required cooldown period before bow is allowed to be started again
bool DCMotorControl::emergencyDisable(uint16_t coolDown) {
    if (emergencyCoolDown) { return false; }
    emergencyCoolDownEvent = millis();
    emergencyCoolDown = true;
    debugPrintln("Emergency disable! Setting cool down to " + String(emergencyCoolDownEvent), Debug);
    disableMotorPower();
    return true;
}

bool DCMotorControl::getMotorFault() {
    if (digitalRead(motorFaultPin) == 0) {    // changed 2024-05-07
        return  true;
    } else {
        return false;
    }
}

void DCMotorControl::setMotorMaxCurrent(float amps) {
    motorCurrentLimit = amps;
}

float DCMotorControl::getMotorMaxCurrent() { return motorCurrentLimit; };

#endif

