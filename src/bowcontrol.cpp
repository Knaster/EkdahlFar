#ifndef BOWCONTROL_C
#define BOWCONTROL_C

#include "elapsedMillis.h"

#include <stdint.h>
#include <sys/types.h>
#include "bowcontrol.h"

#include "calibrate.h"

/// Sets the current bow mode
enum _tiltMode { Rest, Engage, Mute };
enum _speedMode { Automatic, Manual };
//enum _controlMode { Manual, Automatic };

/**
 * @file bowcontrol_c.h
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

class bowControl {
public:
    bowIO *bowIOConnect;                      ///< Pointer to associated bowIO object
    _calibrationData *calibrationDataConnect; ///< Pointer to associated _calibrationData object

    float Kp = 200;                           ///< PID P multiplier 100
    float Ki = 12;                             ///< PID I multiplier 2
    float Kd = 200;                            ///< PID D multiplier 40
    float previousError = 0.0;                ///< Contains the last frequency error calculated by the PID, given in Hertz
    float integratorIgnoreBelow = 0.5;        ///< The PID will ignore any integral errors under this threshold 0.4
    uint16_t pidUpdateInterval = 1000;        ///< The interval at which the PID is being called, given in uS
    float pidMaxError = 50;                   ///< Max PID error per loop, essentially sets acceleration

    float KpTerm = 0;                         ///< Calculated P term
    float KiTerm = 0;                         ///< Calculated I term
    float KdTerm = 0;                         ///< Calculated D term
    float integral = 0.0;                     ///< Contains the integral used by the PID
protected:
    float pidTargetSpeed = 0;                 ///< PID target speed

public:
    elapsedMillis elapsedSinceLastTarget;     ///< Time elapsed since last change in bowing speed or pressure
    char elapsedTimeThreshold = 40;           ///< Time in ms that elapses in between each bow speed stability check
    float recoverRate = 0.05;                 ///< Rate at which the Tilt PWM recovers when it has been decreased in order to avoid bow motor stall and oscillation
    bool inRecovery = false;                  ///< Set to true when bow recovery has started
    bool inRelapse = false;                   ///< Set to true when bow recovery has occurred and it has recovered too much, aka triggered bow instability again.
                                        ///< This prevents the bow from recovering any more, preventing future instabilities
private:
    unsigned long previousTime = 0;           ///< This contains the last time stamp that updateString has been processed
    int motorFault = 0;                       ///< Contains the number of motor faults since the last check
    #define stringUpdateInterval = 10000      ///< Interval at which updateString is being processed, given in uS (10ms)

public:
    int run = 0;                          ///< Set to '1' if the bowing wheel is running
    bool PIDon = true;                        ///< Set to true if the PID is turned on

    uint16_t manualTiltPWM = 0;               ///< Manual change to the current Tilt PWM
    uint16_t manualSpeedPWM = 0;              ///< Manual change to the current bowing speed PWM
    uint16_t lastTiltPWM = 0;                 ///< Last Tilt PWM value set, used as a full unsigned 16-bit value

    uint16_t baselineTiltPWM = 0;             ///< The baseline tilt value used when engaging the bow with the string
    uint16_t modifierTiltPWM = 0;             ///< The modifier tilt value used in conjunction with the baselineTiltPWM when engaing the bow with the string
    _tiltMode tiltMode = Rest;

    elapsedMillis elapsedSinceMute;           ///<
    uint16_t mutePeriod = 350;                ///< How long to perform mute action before going back to rest
    uint16_t muteForce = 3000;                ///< Amount of PWM to add to the maximum allowed tilt PWM to accomplish mute

    elapsedMillis bowShutoffTimer;
    uint16_t bowShutoffTimeout = 1000;
    bool bowShutoffTimedout = false;
    _speedMode speedMode = Automatic;

//    BufferedOutput *slaveSerialOut = nullptr;
//    SafeStringReader *slaveSerialRead = nullptr;
    _harmonicSeriesList harmonicSeriesList;
    uint8_t currentHarmonicSeries = 0;

    uint8_t baseNote = 64;

    float bowSpeedToEngage = 10;
    float bowSpeedWhileEngaged = 10;
    bool reachedEngage = false;

private:
    float currentHarmonicFreq = 0;            ///< Base frequency of current harmonic
    float currentHarmonicShiftFreq = 0;       ///< Calculated frequency with harmonic shift
    float currentHarmonicShift5 = 0;
    int harmonicShiftRange = 12;              ///< Harmonic shift range for one 16-bit integer (full range up and down)
    int harmonicShift = 0;                    ///< Harmonic shift given as one 16-bit integer
    int harmonicShift5 = 0;
    int harmonic = 0;                         ///< Current harmonic

    bool _hold = false;

    float bowCurrentLimit = 1.5;
    float pidPeakError = 0;

public:
    /// Constructor
    bowControl(bowIO &_bowIO, _calibrationData &__calibrationData) {
        bowIOConnect = &_bowIO;
        calibrationDataConnect = &__calibrationData;
    }

#define pidErrorThreshold 5     ///< Threshold that will trigger a bow instability event, given in Hertz

    void setBowCurrentLimit(float inBowCurrentLimit) {
        bowCurrentLimit = inBowCurrentLimit;
    }

    float getBowCurrentLimit() {
        return bowCurrentLimit;
    }

    /// Check if the bow is holding a stable frequency
    int isBowStable() {
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
    void pidReset() {
        integral = 0;
    }

    /// Set PID target speed, check that it doesnt go above maxHz or below minHz
    bool setPIDTarget(float _pidTargetSpeed) {
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
    void setPIDTargetUnsafe(float _pidTargetSpeed) {
        inRecovery = false;
        inRelapse = false;
        bowIOConnect->tiltAdjust = 0;
        pidTargetSpeed = _pidTargetSpeed;
        elapsedSinceLastTarget = 0;
        bowShutoffTimedout = false;
    }

    /// Returns the PID target speed
    float getPIDTarget() {
        return pidTargetSpeed;
    }

    /// PID calculation function to be called at pidUpdateInterval, called by pidInterruptCaller
    /// \todo add Integral injection and other pre-loading parameters to help bow start and change
    /// \todo final check so PWM is never out of range min/max
    /// \todo startup injection of fundamental maybe?
    void pidControl() {
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
//        debugPrintln("PID", Debug);
        bowIOConnect->setSpeedPWM(static_cast<uint16_t>(output));

        // Store the current error for the next iteration
        previousError = error;
    };

    /// This function is to be called every pidUpdateInterval
    void pidInterruptCaller() {
        if (PIDon && (run == 1) && (pidTargetSpeed > 0)) {
            pidControl();
        }
    }

    float getPIDPeakError() {
        float a = pidPeakError;
        pidPeakError = 0;
        return a;
    }

    /// To be called when a motor FAULT interrupt occurs, adds one occurance to motorFault
    void motorFaultDetected() {
        motorFault++;
    }

    /// Returns the number of motor faults that has happend since the last check and zeroes motorFault
    int checkMotorFault() {
        int a = motorFault;
        motorFault = 0;
        return a;
    }

    void setBowPressureSafe(uint16_t tilt) {
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
    bool calculateBaselineModifierPressure() {
/*        unsigned int tiltPWM = calibrationDataConnect->firstTouchPressure - 4000 +
            ((double)(calibrationDataConnect->stallPressure - calibrationDataConnect->firstTouchPressure + 8000)
            / 65535 * ((double)(baselineTiltPWM + modifierTiltPWM)));
*/
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
    void setPressureBaseline(uint16_t baseline) {
        baselineTiltPWM = baseline;
#if EFARMASTER
    //    writeToSlave("spb:" + String(baseline));
#elif EFARSLAVE
        calculateBaselineModifierPressure();
#endif
    }

    uint16_t getPressureBaseline() {
        return baselineTiltPWM;
    }

    void setPressureModifier(uint16_t modifier) {
        modifierTiltPWM = modifier;
        if (tiltMode == Engage) {
            calculateBaselineModifierPressure();
        }
    }

    uint16_t getPressureModifier() {
        return modifierTiltPWM;
    }

    void setBowPower(uint16_t power) {
        manualSpeedPWM = power;
    }

    uint16_t getBowPower() {
        return manualSpeedPWM;
    }

    // set the manual tilt value and send it to BowIO
    bool setManualTilt(uint16_t tilt) {
    /*    unsigned int tiltPWM = calibrationDataConnect->minUsablePressure - 4000 +
      ((double)(calibrationDataConnect->maxUsablePressure - calibrationDataConnect->minUsablePressure + 8000)
      / 65535 * ((double)(manualTiltPWM / 127 * (pressure + 1))));*/
        manualTiltPWM = tilt;
        setBowPressureSafe(tilt);
        debugPrintln("setManualTilt", Debug);
        return true;
    }

    bool bowRest(int enact) {
        if (enact == 0) { return false; }
        tiltMode = Rest;
        if (_hold) { return true; }
        //int restPosition = calibrationDataConnect->firstTouchPressure - 4000;
        //if (restPosition < 0) { restPosition = 0; }
        //if (restPosition > calibrationDataConnect->stallPressure) { restPosition = calibrationDataConnect->stallPressure; }

        reachedEngage = false;
        bowShutoffTimer = 0;
        setBowPressureSafe(calibrationDataConnect->restPosition);
        return true;
    }

    bool setHold(bool hold) {
        _hold = !hold;
        debugPrintln("Hold " + String(hold), Debug);
        if ((_hold == false) && (tiltMode == Rest)) { bowRest(1); }
        return true;
    }

    bool bowEngage(int enact) {
        if (enact == 0) { return false; }
        reachedEngage = false;
        // UGLY 2024-03-10
        bowIOConnect->stepServoStepper->reachedTarget = false;
        tiltMode = Engage;
        return calculateBaselineModifierPressure();
    }

    /// Mute string by turning off the bowing wheel and forcing it into the string by means of setting the Tilt PWM
    bool bowMute(int enact) {
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
    bool calculateHarmonicShift() {
        float freq = currentHarmonicFreq * pow(2, ((float) (((float) harmonicShiftRange) / 12) * harmonicShift / 32768 ));
        float freq5 = freq * pow(2, ((float) (((float) 60) / 12) * harmonicShift5 / 32768 ));
        debugPrintln("Current freq " + String(currentHarmonicFreq) + " shifted freq " + String(freq) + " shifted freq 5 octaves " + String(freq5), Debug);
        currentHarmonicShiftFreq = clamp(freq5, calibrationDataConnect->minHz, calibrationDataConnect->maxHz);
        if (currentHarmonicShiftFreq != freq5) { return false; }
        return true;
    }

    bool setHarmonicShift(int inHarmonicShift) {
        harmonicShift = inHarmonicShift;
        if (!calculateHarmonicShift()) { return false; }
        if (!setPIDTarget(currentHarmonicShiftFreq)) { return false; };
        debugPrintln("New shifted frequency is " + String (currentHarmonicShiftFreq), Debug);
        return true;
    }

    bool setHarmonicShift5(int inHarmonicShift5) {
        harmonicShift5 = inHarmonicShift5;
        if (!calculateHarmonicShift()) { return false; }
        if (!setPIDTarget(currentHarmonicShiftFreq)) { return false; };
        debugPrintln("New shifted frequency is " + String (currentHarmonicShiftFreq), Debug);
        return true;
    }

    int getHarmonicShift() {
        return harmonicShift;
    }

    int getHarmonicShift5() {
        return harmonicShift5;
    }

    bool setHarmonicShiftRange(int inHarmonicShiftRange) {
        harmonicShiftRange = inHarmonicShiftRange;
        return true;
    }

    int getHarmonicShiftRange() {
        return harmonicShiftRange;
    }

    bool setHarmonic(int _harmonic) {
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

    int getHarmonic() {
        return harmonic;
    }

    void updateHarmonicData() {
        setHarmonic(harmonic);
    }

    bool setBaseNote(int inBaseNote) {
        if ((inBaseNote < 0) || (inBaseNote > 127)) { return false; }
        baseNote = uint8_t (inBaseNote);
        return true;
    }

    void measureTimeToTarget(float _pidTargetSpeed) {
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

    String dumpData() {
        String dump = "";
        dump += "ki:" + String(Ki) + ",";
        dump += "kp:" + String(Kp) + ",";
        dump += "kd:" + String(Kd) + ",";
        dump += "ie:" + String(integratorIgnoreBelow) + ",";
        dump += "sbn:" + String(baseNote) + ",";
        dump += "hsr:" + String(harmonicShiftRange) + ",";
        dump += "bes:" + String(bowSpeedToEngage) + ",";
        dump += "bms:" + String(bowSpeedWhileEngaged) + ",";
        return dump;
    }

    /// Handles Tilt PWM updates, zero-speed bow state and motor faults / oscillations
    void updateString() {
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

};
#endif
