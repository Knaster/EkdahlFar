#ifndef DCMOTORCONTROL_C
#define DCMOTORCONTROL_C

#include "generic_functions/dcmotorcontrol.hpp"

const ModuleCommandDeclaration DCMotorControl::moduleCommands[] = {
    { "run", "ru", "1|0", "Set bow motor run on/off", false, false, &s_run, eCommandType_data::ectSimpleBool | eCommandType_function::ectParameter },
    { "pwm", "pw", "0-65535", "Bow motor direct power in 16-bit PWM values, requires that the PID is turned off", false, false, &s_pwm,
        eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectParameter },
    { "voltage", "vo", "float", "Bow motor voltage", false, true, &s_voltage, eCommandType_data::ectSimpleFloat | eCommandType_function::ectSetting },
    { "current", "cu", "float", "Bow motor reported current use", false, false, &s_current, eCommandType_data::ectSimpleFloat | eCommandType_access::ectRequest },
    { "currentlimit", "cl", "float", "Bow motor current limit (A)- !WARNING! Can ruin your instrument if changed", false, true, &s_currentLimit,
        eCommandType_data::ectSimpleFloat | eCommandType_function::ectVolatileSetting },
    { "powerlimit", "pl", "float", "Bow motor power limit (W) - !WARNING! Can ruin your instrument if changed", false, true, &s_powerLimit,
        eCommandType_data::ectSimpleFloat | eCommandType_function::ectVolatileSetting },
    { "frequency", "fq", "-", "Bow motor reported frequency", false, false, &s_frequency, eCommandType_data::ectSimpleFloat | eCommandType_function::ectParameter },
    { "emergencystop", "es", "ms (0-65535)", "Immediately stops the bowing motor and doesn't allow it to start again until the cool down period given in the first argument has lapsed (milliseconds)",
        false, false, &s_emergencyStop, eCommandType_data::ectOutputAssignment | eCommandType_function::ectVolatileSetting },
    { "minpwm", "ip", "-", "Bow motor minimum PWM (for calibration)", false, true, &s_minPWM, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectSetting },
    { "maxpwm", "xp", "-", "Bow motor maximum PWM (for calibration)", false, true, &s_maxPWM, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectSetting }
};

getModuleCount(DCMotorControl)

DCMotorControl::DCMotorControl(char inMotorRevPin, char inMotorVoltagePin, char inMotorDCDCEnPin, char inTachoPin, char inCurrentSensePin, char inMotorFaultPin) {
//    moduleID = new ModuleID("dcmotor", "dcm", "DC Motor controller v1.0", eModuleType::hardware);

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

CREATE_MODULE_COMMAND_FUNCTION(run, DCMotorControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pRun), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setBowMotorRun(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(pRun), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(pwm, DCMotorControl) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setSpeedPWM(inCommandItem->argument[0].toInt());
    }
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(getSpeedPWM()), debugPrintType::InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(voltage, DCMotorControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(motorVoltage), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setMotorVoltage(inCommandItem->argument[0].toFloat());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(motorVoltage), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(current, DCMotorControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getMotorCurrent()), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(currentLimit, DCMotorControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(motorCurrentLimit), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setMotorMaxCurrent(inCommandItem->argument[0].toFloat());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(motorCurrentLimit), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(powerLimit, DCMotorControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(motorPowerLimit), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setMotorMaxPower(inCommandItem->argument[0].toFloat());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(motorPowerLimit), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(frequency, DCMotorControl) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getAverageTachometerFreq()), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(emergencyStop, DCMotorControl) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        emergencyDisable(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + inCommandItem->argument[0], InfoRequest});
        inCommandResponses->push_back({"EMERGENCY STOP! Cooling down for " + inCommandItem->argument[0] + " ms", debugPrintType::Error});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(minPWM, DCMotorControl) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setMinSpeedPWM(inCommandItem->argument[0].toInt());
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(minSpeedPWM), InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(maxPWM, DCMotorControl) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setMaxSpeedPWM(inCommandItem->argument[0].toInt());
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(maxSpeedPWM), InfoRequest });
    return eProcessResult::Ok;
};

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

void DCMotorControl::setBowMotorRun(bool inRun) {
    pRun = inRun;
    if (pRun) {
        enableMotorPower();
    } else {
        disableMotorPower();
    }
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

