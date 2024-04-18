#ifndef BOWCONTROL_C
#define BOWCONTROL_C

#include "elapsedMillis.h"

#include <stdint.h>
#include <sys/types.h>

#include "calibrate.h"
#include "bowcontrol.h"

/**
 * @file bowcontrol.cpp
 *
 * High level bow control functions
 *
 * Need to have different modes for automatic or manual pressure and speed
 *
 * Current behavior:
 * if run
 *    if pid ON
 *      do nothing as the interrupt driven PID function will set speed to pidTargetSpeed
 *    else
 *      reset PID
 *      setTilt manualTiltPWM
 * else
 *    reset PID
 *    set bow speed to 0
 *
 * tilt is set immediately by either setManualTilt or setPressureBaseline
 * \todo THIS NEEDS TO CHANGE, values should be updated internally but only outputted depending on manual / automatic control
 */



/// Constructor
bowControl::bowControl(bowIO &_bowIO, _calibrationData &__calibrationData) {
    bowIOConnect = &_bowIO;
    calibrationDataConnect = &__calibrationData;
    bowActuators = new BowActuators(&__calibrationData);
}

#define pidErrorThreshold 5     ///< Threshold that will trigger a bow instability event, given in Hertz

void bowControl::setBowCurrentLimit(float inBowCurrentLimit) {
    bowCurrentLimit = inBowCurrentLimit;
}

float bowControl::getBowCurrentLimit() {
    return bowCurrentLimit;
}

/// Check if the bow is holding a stable frequency
int bowControl::isBowStable() {
    if (elapsedSinceLastTarget < elapsedTimeThreshold) { return -1; }
    if (bowIOConnect->averageFreq() == 0) { return 1; }
    if (previousError >= pidErrorThreshold) {
        elapsedSinceLastTarget = 0;
        return 0;
    } else {
        return 1;
    }
}

    /// Resets the PID integral
void bowControl::pidReset() {
    integral = 0;
}

/// Set PID target speed, check that it doesnt go above maxHz or below minHz
bool bowControl::setPIDTarget(float _pidTargetSpeed) {
    if (((_pidTargetSpeed > calibrationDataConnect->maxHz) || (_pidTargetSpeed < calibrationDataConnect->minHz)) && _pidTargetSpeed != 0) {
        debugPrintln("PID Target out of range!", Hardware);
        return false;
    }
    debugPrintln("Setting PID target to " + String(_pidTargetSpeed), Hardware);
    setPIDTargetUnsafe(_pidTargetSpeed);
    if (_pidTargetSpeed == 0) { pidReset(); }
    return true;
}

/// Set PID target speed without any safety checks, used by setPIDTarget
void bowControl::setPIDTargetUnsafe(float _pidTargetSpeed) {
    inRecovery = false;
    inRelapse = false;
    bowIOConnect->tiltAdjust = 0;
    pidTargetSpeed = _pidTargetSpeed;
    elapsedSinceLastTarget = 0;
    bowShutoffTimedout = false;
}

/// Returns the PID target speed
float bowControl::getPIDTarget() {
    return pidTargetSpeed;
}

/// PID calculation function to be called at pidUpdateInterval, called by pidInterruptCaller
/// \todo add Integral injection and other pre-loading parameters to help bow start and change
/// \todo final check so PWM is never out of range min/max
/// \todo startup injection of fundamental maybe?
void bowControl::pidControl() {
    // Calculate the error between the target speed and the current speed
    float currentSpeed = bowIOConnect->getLastTachoFreq();
    float error = pidTargetSpeed - currentSpeed;

    if (abs(error) > pidPeakError) { pidPeakError = abs(error); }

    if (error > pidMaxError) { error = pidMaxError; }
    if (error < -pidMaxError) { error = -pidMaxError; }

    // Update the integral term
    if ((error >= integratorIgnoreBelow) || (error <= -integratorIgnoreBelow)) { integral += error; }

    // Calculate the PID control output
    KpTerm = Kp * error;
    KiTerm = Ki * integral;
    KdTerm = Kd * (error - previousError);

    // At incorrect Ki values the integral starts building,this doesn't really matter since
    // the type cast makes the output wrap but it might cause trouble in the future so added this
    if (KiTerm > 65535) { integral -= 65535;}

    float output = KpTerm + KiTerm + KdTerm;

    if (output < 0) { output = 0; }
    // This attempt at setting lowest speed doesn't work, makes you not be able to go beyond a certain key
/*        if ((pidTargetSpeed > 0) && (output < calibrationDataConnect->minInertialPWM)) {
        output = calibrationDataConnect->minInertialPWM;
    }
*/
    // If problem, check the following line - added 2023-10-22
    if (output > 65535) { output = 65535; }

    // Set the motor speed using PWM
    bowIOConnect->setSpeedPWM(static_cast<uint16_t>(output));

    // Store the current error for the next iteration
    previousError = error;
};

/// This function is to be called every pidUpdateInterval
void bowControl::pidInterruptCaller() {
    if (PIDon && (run == 1) && (pidTargetSpeed > 0)) {
        pidControl();
    }
}

float bowControl::getPIDPeakError() {
    float a = pidPeakError;
    pidPeakError = 0;
    return a;
}

/// To be called when a motor FAULT interrupt occurs, adds one occurance to motorFault
void bowControl::motorFaultDetected() {
    motorFault++;
}

/// Returns the number of motor faults that has happend since the last check and zeroes motorFault
int bowControl::checkMotorFault() {
    int a = motorFault;
    motorFault = 0;
    return a;
}

void bowControl::setBowPressureSafe(uint16_t tilt) {
    if (tilt > calibrationDataConnect->stallPressure) { tilt = calibrationDataConnect->stallPressure; }

    if ((tiltMode == Engage) && (bowIOConnect->stepServoStepper->reachedTarget))  {
        reachedEngage = true;
        debugPrintln("Reached engage", debugPrintType::Debug);
    }

    if ((tiltMode == Engage) && (reachedEngage)) {
        bowIOConnect->stepServoStepper->setSpeed(bowSpeedWhileEngaged);
        debugPrintln("Setting pressure speed to slow", debugPrintType::Debug);
    } else {
        bowIOConnect->stepServoStepper->setSpeed(bowSpeedToEngage);
        debugPrintln("Setting pressure speed to high", debugPrintType::Debug);
    }

    bowIOConnect->setTiltPWM(tilt);
    return;
}

/// Calculate the tilt PWM value using baselineTiltPWM and modfierTiltPWM and send it to BowIO
bool bowControl::calculateBaselineModifierPressure() {
    unsigned int tiltPWM = calibrationDataConnect->firstTouchPressure +
        ((double)(calibrationDataConnect->stallPressure - calibrationDataConnect->firstTouchPressure)
        / 65535 * ((double)(baselineTiltPWM + modifierTiltPWM)));

    setBowPressureSafe(tiltPWM);
    return true;
}

/*
    void writeToSlave(String command) {
        if (slaveSerialOut != nullptr) {
            slaveSerialOut->println(command.c_str());
        } else {
            debugPrintln("Slave serial device is NULL", Error);
        }
    }
*/
void bowControl::setPressureBaseline(uint16_t baseline) {
    baselineTiltPWM = baseline;
#if EFARMASTER
//    writeToSlave("spb:" + String(baseline));
#elif EFARSLAVE
    calculateBaselineModifierPressure();
#endif
}

uint16_t bowControl::getPressureBaseline() {
    return baselineTiltPWM;
}

void bowControl::setPressureModifier(uint16_t modifier) {
    modifierTiltPWM = modifier;
    if (tiltMode == Engage) {
        calculateBaselineModifierPressure();
    }
}

uint16_t bowControl::getPressureModifier() {
    return modifierTiltPWM;
}

void bowControl::setBowPower(uint16_t power) {
    manualSpeedPWM = power;
}

uint16_t bowControl::getBowPower() {
    return manualSpeedPWM;
}

// set the manual tilt value and send it to BowIO
bool bowControl::setManualTilt(uint16_t tilt) {
    manualTiltPWM = tilt;
    setBowPressureSafe(tilt);
    debugPrintln("setManualTilt", Debug);
    return true;
}

bool bowControl::bowRest(int enact) {
    if (enact == 0) { return false; }
    tiltMode = Rest;
    if (_hold) { return true; }

    reachedEngage = false;
    bowShutoffTimer = 0;
    setBowPressureSafe(calibrationDataConnect->restPosition);
    return true;
}

bool bowControl::setHold(bool hold) {
    _hold = !hold;
    debugPrintln("Hold " + String(hold), Debug);
    if ((_hold == false) && (tiltMode == Rest)) { bowRest(1); }
    return true;
}

bool bowControl::bowEngage(int enact) {
    if (enact == 0) { return false; }
    reachedEngage = false;
    // UGLY 2024-03-10
    bowIOConnect->stepServoStepper->reachedTarget = false;
    tiltMode = Engage;
    return calculateBaselineModifierPressure();
}

/// Mute string by turning off the bowing wheel and forcing it into the string by means of setting the Tilt PWM
bool bowControl::bowMute(int enact) {
    if (enact == 0) { return false; }
    pidTargetSpeed = 0;
    bowIOConnect->setSpeedPWM(0);
    setBowPressureSafe(calibrationDataConnect->firstTouchPressure + muteForce);
    debugPrintln("bowMute", Debug);
    tiltMode = Mute;
    elapsedSinceMute = 0;
    return true;
}

/// Calculate the final frequency using currentHarmonicFreq and currentHarmonicShiftFreq
bool bowControl::calculateHarmonicShift() {
    float freq = currentHarmonicFreq * pow(2, ((float) (((float) harmonicShiftRange) / 12) * harmonicShift / 32768 ));
    float freq5 = freq * pow(2, ((float) (((float) 60) / 12) * harmonicShift5 / 32768 ));
    debugPrintln("Current freq " + String(currentHarmonicFreq) + " shifted freq " + String(freq) + " shifted freq 5 octaves " + String(freq5), Debug);
    currentHarmonicShiftFreq = clamp(freq5, calibrationDataConnect->minHz, calibrationDataConnect->maxHz);
    if (currentHarmonicShiftFreq != freq5) { return false; }
    return true;
}

bool bowControl::setHarmonicShift(int inHarmonicShift) {
    harmonicShift = inHarmonicShift;
    if (!calculateHarmonicShift()) { return false; }
    if (!setPIDTarget(currentHarmonicShiftFreq)) { return false; };
    debugPrintln("New shifted frequency is " + String (currentHarmonicShiftFreq), Debug);
    return true;
}

bool bowControl::setHarmonicShift5(int inHarmonicShift5) {
    harmonicShift5 = inHarmonicShift5;
    if (!calculateHarmonicShift()) { return false; }
    if (!setPIDTarget(currentHarmonicShiftFreq)) { return false; };
    debugPrintln("New shifted frequency is " + String (currentHarmonicShiftFreq), Debug);
    return true;
}

int bowControl::getHarmonicShift() {
    return harmonicShift;
}

int bowControl::getHarmonicShift5() {
    return harmonicShift5;
}

bool bowControl::setHarmonicShiftRange(int inHarmonicShiftRange) {
    harmonicShiftRange = inHarmonicShiftRange;
    return true;
}

int bowControl::getHarmonicShiftRange() {
    return harmonicShiftRange;
}

bool bowControl::setHarmonic(int _harmonic) {
    //    debugPrintln("Remove the following line, its just for testing ", Error)
    //    calibrationDataConnect->upperHarmonic = 49;
    /// \todo do we return if we're out of bounds or do lowest/highest possible?
    int __harmonic = clamp(_harmonic, calibrationDataConnect->lowerHarmonic, calibrationDataConnect->upperHarmonic);
    //    debugPrintln("LH " + String(calibrationDataConnect->lowerHarmonic) + " UH " + String(calibrationDataConnect->upperHarmonic), Debug);
    if (__harmonic != _harmonic) { return false; }
    harmonic = __harmonic;
    int harmonicCount = harmonicSeriesList.series[currentHarmonicSeries].frequency.size();
    // Calculate where in the series the current harmonic resides (0-11)
    /*    int series = harmonic % 12;
    if (series < 0) { series = 12 + series; }*/
    int series = harmonic % harmonicCount;
    if (series < 0) { series = harmonicCount + series; }
    // If harmonic is below 0 we need to reduce for the truncation to work properly
    /*    if (harmonic < 0) { harmonic -= 11; }
    int octave = trunc(harmonic / 12);*/
    if (harmonic < 0) { harmonic -= (harmonicCount - 1); }
    int octave = trunc(harmonic / harmonicCount);

    //    float freq = calibrationDataConnect->fundamentalFrequency * pow(2, octave) * justSeries[series]; // - 0.4;
    float freq = calibrationDataConnect->fundamentalFrequency * pow(2, octave) * harmonicSeriesList.series[currentHarmonicSeries].frequency[series]; // - 0.4;

    debugPrintln("Setting harmonic to " + String(series) + " @ frequency " + String(freq), Debug);

    if (freq > calibrationDataConnect->maxHz) { debugPrintln("ERROR! Over max speed!!", Error); return false; };
    if (freq < calibrationDataConnect->minHz) { debugPrintln("Harmonic under limit @ " + String(freq) + " Hertz", Error); return false; }

    currentHarmonicFreq = freq;
    calculateHarmonicShift();
    if (!setPIDTarget(currentHarmonicShiftFreq)) { return false; };

    return true;
}

int bowControl::getHarmonic() {
    return harmonic;
}

void bowControl::updateHarmonicData() {
    setHarmonic(harmonic);
}

bool bowControl::setBaseNote(int inBaseNote) {
    if ((inBaseNote < 0) || (inBaseNote > 127)) { return false; }
    baseNote = uint8_t (inBaseNote);
    return true;
}

void bowControl::measureTimeToTarget(float _pidTargetSpeed) {
    elapsedMillis timeToTarget;
    elapsedMicros overshootTime;
    float startSpeed = getPIDTarget();

    timeToTarget = 0;
    float overshoot = 0;
    float undershoot = -1;
    float freq;
    int iterations = 0;

    setPIDTarget(_pidTargetSpeed);
    while(iterations < 10) {
        if (round(bowIOConnect->averageFreq()) == round(_pidTargetSpeed)) { iterations++; }

        overshootTime = 0;
        while (overshootTime < 10) {
            freq = bowIOConnect->getLastTachoFreq();
            if (freq > overshoot) { overshoot = freq; }
            if ((freq < undershoot) || (undershoot = -1 )) { undershoot = freq; }
        }
        if (timeToTarget > 1000) {
            debugPrintln("Couldn't get to target speed in time (reached " + String(bowIOConnect->averageFreq()) + " Hertz)", Error);
            return;
        }
    }

    debugPrint("Going from " + String(startSpeed) + " -> " + String(_pidTargetSpeed) + " in " + String(timeToTarget) + "ms", InfoRequest);
    if (_pidTargetSpeed > startSpeed) { debugPrintln(", overshoot " + String(overshoot), InfoRequest); }
    if (_pidTargetSpeed < startSpeed) { debugPrintln(", undershoot " + String(undershoot), InfoRequest); }
    //, max freq " + String(overshoot) + " Hz, min freq " + String(undershoot) + "Hz", InfoRequest);
}

String bowControl::dumpData() {
    String dump = "";
    dump += "bpki:" + String(Ki) + ",";
    dump += "bpkp:" + String(Kp) + ",";
    dump += "bpkd:" + String(Kd) + ",";
    dump += "bpie:" + String(integratorIgnoreBelow) + ",";
    dump += "bchbn:" + String(baseNote) + ",";
    dump += "bchsr:" + String(harmonicShiftRange) + ",";
    dump += "bpes:" + String(bowSpeedToEngage) + ",";
    dump += "bpms:" + String(bowSpeedWhileEngaged) + ",";
    dump += "bmt:" + String(bowShutoffTimeout) + ",";
    return dump;
}

/// Handles Tilt PWM updates, zero-speed bow state and motor faults / oscillations
void bowControl::updateString() {
    unsigned long currentTime = micros();

    bowIOConnect->updateBow();

    // Process only if at the pidUpdateInterval
    if (currentTime - previousTime >= pidUpdateInterval) {
        previousTime = currentTime;

        // Calculate the manual Tilt PWM
        //unsigned int tiltPWM = calibrationDataConnect->minUsablePressure - 8000 +
        //  ((double) (calibrationDataConnect->maxUsablePressure - calibrationDataConnect->minUsablePressure + 16000) / 65535 * manualTiltPWM);

        /// \todo Replace with a proper motor fault
        if (checkMotorFault()) {
            debugPrintln("Motor driver Fault!", Error);
        }

        if ((tiltMode == Rest) && (speedMode == Automatic) &&  (_hold == false) && (bowShutoffTimer >= bowShutoffTimeout) && (bowShutoffTimedout == false)) {
            run = 0;
            bowShutoffTimedout = true;
            debugPrintln("Auto shutdown of motor", Debug);
        }

        if (run) {
            if (PIDon) {
                if (pidTargetSpeed == 0) {
                    bowIOConnect->setSpeedPWM(0);
                }
            } else {
                pidReset();
                bowIOConnect->setSpeedPWM(manualSpeedPWM);
            }
        }  else {
        // If the string module is not running, turn off the bowing wheel
            pidReset();
            bowIOConnect->setSpeedPWM(0);
        }

        // Check if the bowing wheel speed is zero
        bowIOConnect->checkTimeout();
        if (((tiltMode == Mute) && (mutePeriod > 0)) && (elapsedSinceMute >= mutePeriod)) {
            bowRest(1);
        }
    }
};

#endif
