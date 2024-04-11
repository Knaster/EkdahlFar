/** \brief Constructor for the bowIO class
 *
 *  + Sets up all hardware pins associated with the string unit
 *  + Creates a new Teensy_PWM object for the bow motor driver
 *  + Associates a channel on the Arduino PWM shield for the tilt servo motor
 */

#ifndef BOWIO_C
#define BOWIO_C

#include "bowio.h"

#define BITDEPTH 16
#define BITDIV 1 // = 2^(16-BITDEPTH)

#define DCDCLOWERBOUND 1.2
#define DCDCUPPERBOUND 9

bool bowIO::setBowMotorVoltage(float voltage) {
    bowMotorVoltage = voltage;
    voltage -= DCDCLOWERBOUND;
    float t = (65536 / (DCDCUPPERBOUND - DCDCLOWERBOUND)) * voltage;
    uint16_t pwm = 65536 - ((uint16_t) t);
//    debugPrintln("Voltage PWM " + String(pwm), Debug);
    analogWrite(bowMotorVoltagePin, pwm);
    return true;
}

float bowIO::getBowMotorVoltage() {
    return bowMotorVoltage;
}


bool bowIO::disableBowPower() {
    digitalWrite(bowMotorDCDCEnPin, 0);
    return true;
}

bool bowIO::enableBowPower() {
    digitalWrite(bowMotorDCDCEnPin, 1);
    return true;
}

bowIO::bowIO(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin, char stepCorrectionSensorPin) {
    bowMotorRevPin = motorRevPin;
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
    setSpeedPWM(0);
    setTiltPWM(0);

    stepSerialStream = stepSerialPort;
    stepServoStepper = new servoStepper(stepStepPin, stepDirPin, stepHomeSensorPin); //, stepEnPin);
    stepTMC2209Driver = new TMC2209();
    setupTMC2209();
    stepServoStepper->setHomingOffset(6000);
//    stepServoStepper->setHomingOffset(22500);
    debugPrintln("Starting home", Debug);
    stepTMC2209Driver->setRunCurrent(stepHomeCurrentPercent);
    stepServoStepper->home(servoStepper::eStepDirection::FORWARD,5,5,servoStepper::eStepDirection::REVERSE); // ,5,5
    if (stepServoStepper->completeTask(5000)) {    //2000
        debugPrintln("Homed", Debug);
    } else {
        debugPrintln("Homing FAILED!", Error);
    }
    stepTMC2209Driver->setRunCurrent(stepRunCurrentPercent);
    stepServoStepper->setSpeed(20);
    stepServoStepper->setPosition(0);
    stepServoStepper->completeTask();
    //stepServoStepper->setSpeed(20);
}

/** \brief Adds a tachometer value to the tachometer buffer, given in Hertz
 *
 *  Advances the tachoFreq index counter tachoFreqIndex and increases tachFreqCount if it has not yet reached tachoFreqLength.
 *  Values outside of the range defined by tachoFreqPermissibleMAX and tachoFreqPermissibleMIN are set to 0
 */
void bowIO::addTachoFreq(float freq) {
    if ((freq >= tachoFreqPerimssibleMAX) || (freq <= tachoFreqPerimssibleMIN)) {
        freq = 0;
    }

    tachoFreq[tachoFreqIndex] = freq;
    tachoFreqIndex++;
    if (tachoFreqCount < tachoFreqLength) { tachoFreqCount++; }
    if (tachoFreqIndex >= tachoFreqLength) { tachoFreqIndex = 0; }
}

/// Returns the last added tachometer value
float bowIO::getLastTachoFreq() {
    if (tachoFreqIndex < 1) {
        return tachoFreq[tachoFreqLength - 1];
    } else {
        return tachoFreq[tachoFreqIndex - 1];
    }
}

void bowIO::clearTachoData() {
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
float bowIO::averageFreq() {
    if (tachoFreqCount < tachoFreqLength) {
    //return -1;
        if (tachoFreq[tachoFreqIndex] == 0) {
            return 0;
        } else {
            debugPrintln("AverageFreq: error, not enough point stored.", Priority);
            //return tachoOldAverage;
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

/// Ramps the bow speed PWM from the last PWM value given to the value given in speed, blocking, only use for testing purposes
void bowIO::setSpeedPWMSafe(uint16_t speed) {
    int a = speed;
    //  digitalWrite(ZXBMFwdPin, 1);
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

    lastBowMotorPWM = a;
}


/// Sets the bow speed PWM using a unsigned 16-bit int (RAW)
void bowIO::setSpeedPWM(uint16_t speed) {
/*    if (speed != lastBowMotorPWM) {
        debugPrintln("Setting bow motor PWM to " + String(speed), Hardware);
    }
*/
    int a = speed;
    //debugPrintln("Changing pin " + String(((int) ZXBMRevPin)) + " to " + String(65535 - a), Hardware);
    //analogWrite(ZXBMRevPin, (65535 - a) / BITDIV);
    bowMotorPWM->setPWM_manual(bowMotorRevPin, (65535 - a) / BITDIV);
    lastBowMotorPWM = a;
}

/** \brief Sets the tilt servo PWM value using a unsigned 16-bit int (RECALCULATED)
 *
 *  Recalculated internally to use SERVO_MIN and SERVO_MAX as lower and upper bound respectively
 */
void bowIO::setTiltPWM(uint16_t tilt) {
/*    int a = SERVO_MIN + (((float) (SERVO_MAX - SERVO_MIN) * (65535 - tilt) / 65535));
    a -= tiltAdjust;
    if (lastTilt != a) {
        lastTilt = a;
        debugPrintln("Tilt " + String(a) + " (" + String(tilt) + ")", Hardware);
        //pwmShield->setPWM(tiltPWMChannel, 0, a);
        hwTiltPos = a;
    }*/
    /*
    if (stepper != NULL) {
    stepper->setPosition(tilt);
    }*/
    //  Serial1.print(tilt);
    //  Serial1.write(25);

#ifdef EFARMASTER
/*    if (delegateTilt) {
        debugPrintln("Delegating value " + String(tilt), Debug);
        stepSerialOut.write(tilt >> 8);
        stepSerialOut.write(tilt & 0xFF);
    }*/
#elif EFARSLAVE
    if (stepServoStepper != nullptr) {
        debugPrintln("Setting stepper tilt to " + String(tilt - tiltAdjust), Hardware);
        stepServoStepper->setPosition(tilt);
        lastTilt = tilt;
    }
#endif
}

bool bowIO::waitForTiltToComplete(uint16_t timeout) {
    return stepServoStepper->completeTask(timeout);
}


/// Secondary callback handler for tachometer events
void bowIO::tachoISRHandler() {
    asm("nop");
    //char state = digitalRead(reflectorInterruptPin);
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

/// Check whether bow has timed out and add a value of 0 Hertz if so
bool bowIO::checkTimeout() {
    if (reflectorCounter > reflectorZeroTimeoutValue) {
        addTachoFreq(0);
        return true;
    } else {
        return false;
    }
}

void bowIO::updateBow() {
/*
    if (stepServoStepper != nullptr) {
        stepServoStepper->updatePosition();
    }
    */
    if (bowOverPower()) {

    }
}
/*
bool bowIO::createStepper(byte _pinStep, byte _pinDir, byte _pinHome) {
    stepper = new farStepper(_pinStep, _pinDir, _pinHome);
    return true;
}
*/
bool bowIO::setupTMC2209() {
    if (stepTMC2209Driver == nullptr) { return false; }

    stepTMC2209Driver->setup(*stepSerialStream);
    delay(stepConnectDelay);

    pinMode(stepEnPin, OUTPUT);
    digitalWrite(stepEnPin, 0);

    stepTMC2209Driver->setRunCurrent(stepRunCurrentPercent);
    stepTMC2209Driver->setMicrostepsPerStep(stepMicrostepping);
    stepTMC2209Driver->disableStealthChop();
    stepTMC2209Driver->setHoldCurrent(1);
    stepTMC2209Driver->enable();
    return true;
}


void bowIO::getTMC2209Info() {
    debugPrintln("*************************", InfoRequest);
    debugPrintln("getSettings()", InfoRequest);
    TMC2209::Settings settings = stepTMC2209Driver->getSettings();
    debugPrintln("settings.is_communicating = " + String(settings.is_communicating), InfoRequest);
    debugPrintln("settings.is_setup = " + String(settings.is_setup), InfoRequest);
    debugPrintln("settings.software_enabled = " + String(settings.software_enabled), InfoRequest);
    debugPrintln("settings.microsteps_per_step = " + String(settings.microsteps_per_step), InfoRequest);
    debugPrintln("settings.inverse_motor_direction_enabled = " + String(settings.inverse_motor_direction_enabled), InfoRequest);
    debugPrintln("settings.stealth_chop_enabled = " + String(settings.stealth_chop_enabled), InfoRequest);
    debugPrintln("settings.standstill_mode = ", InfoRequest);
    switch (settings.standstill_mode)
    {
    case TMC2209::NORMAL:
        debugPrintln("normal", InfoRequest);
        break;
    case TMC2209::FREEWHEELING:
        debugPrintln("freewheeling", InfoRequest);
        break;
    case TMC2209::STRONG_BRAKING:
        debugPrintln("strong_braking", InfoRequest);
        break;
    case TMC2209::BRAKING:
        debugPrintln("braking", InfoRequest);
        break;
    }
    debugPrintln("settings.irun_percent = " + String(settings.irun_percent), InfoRequest);
    debugPrintln("settings.irun_register_value = " + String(settings.irun_register_value), InfoRequest);
    debugPrintln("settings.ihold_percent = " + String(settings.ihold_percent), InfoRequest);
    debugPrintln("settings.ihold_register_value = " + String(settings.ihold_register_value), InfoRequest);
    debugPrintln("settings.iholddelay_percent = " + String(settings.iholddelay_percent), InfoRequest);
    debugPrintln("settings.iholddelay_register_value = " + String(settings.iholddelay_register_value), InfoRequest);
    debugPrintln("settings.automatic_current_scaling_enabled = " + String(settings.automatic_current_scaling_enabled), InfoRequest);
    debugPrintln("settings.automatic_gradient_adaptation_enabled = " + String(settings.automatic_gradient_adaptation_enabled), InfoRequest);
    debugPrintln("settings.pwm_offset = " + String(settings.pwm_offset), InfoRequest);
    debugPrintln("settings.pwm_gradient = " + String(settings.pwm_gradient), InfoRequest);
    debugPrintln("settings.cool_step_enabled = " + String(settings.cool_step_enabled), InfoRequest);
    debugPrintln("settings.analog_current_scaling_enabled = " + String(settings.analog_current_scaling_enabled), InfoRequest);
    debugPrintln("settings.internal_sense_resistors_enabled = " + String(settings.internal_sense_resistors_enabled), InfoRequest);
    debugPrintln("*************************", InfoRequest);
    debugPrintln("", InfoRequest);

    debugPrintln("*************************", InfoRequest);
    debugPrintln("hardwareDisabled()", InfoRequest);
    bool hardware_disabled = stepTMC2209Driver->hardwareDisabled();
    debugPrint("hardware_disabled = " + String(hardware_disabled), InfoRequest);
    debugPrintln("*************************", InfoRequest);
    debugPrintln("", InfoRequest);

    debugPrintln("*************************", InfoRequest);
    debugPrintln("getStatus()", InfoRequest);
    TMC2209::Status status = stepTMC2209Driver->getStatus();
    debugPrintln("status.over_temperature_warning = " + String(status.over_temperature_warning), InfoRequest);
    debugPrintln("status.over_temperature_shutdown = " + String(status.over_temperature_shutdown), InfoRequest);
    debugPrintln("status.short_to_ground_a = " + String(status.short_to_ground_a), InfoRequest);
    debugPrintln("status.short_to_ground_b = " + String(status.short_to_ground_b), InfoRequest);
    debugPrintln("status.low_side_short_a = " + String(status.low_side_short_a), InfoRequest);
    debugPrintln("status.low_side_short_b = " + String(status.low_side_short_b), InfoRequest);
    debugPrintln("status.open_load_a = " + String(status.open_load_a), InfoRequest);
    debugPrintln("status.open_load_b = " + String(status.open_load_b), InfoRequest);
    debugPrintln("status.over_temperature_120c = " + String(status.over_temperature_120c), InfoRequest);
    debugPrintln("status.over_temperature_143c = " + String(status.over_temperature_143c), InfoRequest);
    debugPrintln("status.over_temperature_150c = " + String(status.over_temperature_150c), InfoRequest);
    debugPrintln("status.over_temperature_157c = " + String(status.over_temperature_157c), InfoRequest);
    debugPrintln("status.current_scaling = " + String(status.current_scaling), InfoRequest);
    debugPrintln("status.stealth_chop_mode = " + String(status.stealth_chop_mode), InfoRequest);
    debugPrintln("status.standstill = " + String(status.standstill), InfoRequest);
    debugPrintln("*************************", InfoRequest);
    debugPrintln("", InfoRequest);
    return;
}

float bowIO::getBowCurrent() {
      return analogRead(bowCurrentSensePin) * 3.3 / 4096;
}

bool bowIO::bowOverCurrent() {
    if (getBowCurrent() >= bowMotorCurrentLimit) {
        return true;
    } else {
        return false;
    }
}

bool bowIO::bowOverPower() {
    if (getBowCurrent() >= (bowMotorWattage / bowMotorVoltage)) {
        bowOverPowerFlag = true;
        lastBowOverPowerEvent = 0;
        return true;
    } else {
        bowOverPowerFlag = false;
        return false;
    }
}

bool bowIO::getMotorFault() {
    if (digitalRead(bowMotorFaultPin) == 1) {
        return  true;
    } else {
        return false;
    }
}

String bowIO::dumpData() {
    String dump;
    dump = "bv:" + String(bowMotorVoltage) + ",";
    return dump;
}
#endif
