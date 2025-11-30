#ifndef PIDCONTROLLER_C
#define PIDCONTROLLER_C

#include "pidcontroller.hpp"

PIDController::PIDController(CalibrationData &inCalibrationData, bowIO &inBowIO) {
    calibrationDataConnect = &inCalibrationData;
    bowIOConnect = &inBowIO;
}

/// Resets the PID integral
void PIDController::pidReset() {
    integral = 0;
}

/// Set PID target speed, check that it doesnt go above maxHz or below minHz
bool PIDController::setPIDTarget(float _pidTargetSpeed) {
    if (((_pidTargetSpeed > calibrationDataConnect->maxHz) || (_pidTargetSpeed < calibrationDataConnect->minHz)) && _pidTargetSpeed != 0) {
    //if (((_pidTargetSpeed > *maxHz) || (_pidTargetSpeed < *minHz)) && _pidTargetSpeed != 0) {
        debugPrintln("PID Target out of range!", Hardware);
        return false;
    }
    debugPrintln("Setting PID target to " + String(_pidTargetSpeed), Hardware);
    setPIDTargetUnsafe(_pidTargetSpeed);
    if (_pidTargetSpeed == 0) { pidReset(); }
    return true;
}

/// Set PID target speed without any safety checks, used by setPIDTarget
void PIDController::setPIDTargetUnsafe(float _pidTargetSpeed) {
    pidTargetSpeed = _pidTargetSpeed;
//    bowShutoffTimedout = false;
//    bowShutoffMotorDisabled = false;
}

/// Returns the PID target speed
float PIDController::getPIDTarget() {
    return pidTargetSpeed;
}

/// PID calculation function to be called at pidUpdateInterval, called by pidInterruptCaller
/// \todo add Integral injection and other pre-loading parameters to help bow start and change
/// \todo final check so PWM is never out of range min/max
/// \todo startup injection of fundamental maybe?
void PIDController::pidControl() {
    // Calculate the error between the target speed and the current speed
    float currentSpeed = bowIOConnect->getLastTachoFreq();
//    float currentSpeed = getFrequency();

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
    // If problem, check the following line - added 2023-10-22
    if (output > 65535) { output = 65535; }

    // Set the motor speed using PWM
    bowIOConnect->setSpeedPWM(static_cast<uint16_t>(output));
    //setFrequency(static_cast<uint16_t>(output));

    // Store the current error for the next iteration
    previousError = error;
};
/*
/// This function is to be called every pidUpdateInterval
void PIDController::pidInterruptCaller() {
    if (PIDon && (run == 1) && (pidTargetSpeed > 0)) {
        pidControl();
    }
}
*/
float PIDController::getPIDPeakError() {
    float a = pidPeakError;
    pidPeakError = 0;
    return a;
}

void PIDController::measureTimeToTarget(float _pidTargetSpeed) {
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
}


String PIDController::dumpData() {
    String dump = "";
    dump += "bpki:" + String(Ki) + ",";
    dump += "bpkp:" + String(Kp) + ",";
    dump += "bpkd:" + String(Kd) + ",";
    dump += "bpie:" + String(integratorIgnoreBelow) + ",";

    return dump;
}

#endif // PIDCONTROLLER_C
