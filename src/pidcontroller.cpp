#ifndef PIDCONTROLLER_C
#define PIDCONTROLLER_C

#include "pidcontroller.hpp"

eProcessResult PIDController::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                           commandList *delegatedCommands = nullptr) {

    processCommandItems(inCommandItem, serialCommandsPID, sizeof(serialCommandsPID)  / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsPID, sizeof(serialCommandsPID) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "bowpidtargetfreq") {
        if (request) {
            commandResponses->push_back({ "bptf:" + String(pidTargetSpeed), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setPIDTarget(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({ "bptf:" + String(pidTargetSpeed), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowpidki") {
        if (request) {
            commandResponses->push_back({ "bpki:" + String(Ki), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            Ki = inCommandItem->argument[0].toFloat();
            commandResponses->push_back({"bpki:" + String(Ki), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidkp") {
        if (request) {
            commandResponses->push_back({ "bpkp:" + String(Kp), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            Kp = inCommandItem->argument[0].toFloat();
            commandResponses->push_back({"bpkp:" + String(Kp), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidkd") {
        if (request) {
            commandResponses->push_back({ "bpkd:" + String(Kd), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            Kd = inCommandItem->argument[0].toFloat();
            commandResponses->push_back({"bpkd:" + String(Kd), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidr") {
        if (!request) {
            commandResponses->push_back({"bowpidr:1", InfoRequest});
            previousError = 0;
            pidReset();
        }
    }  else
    if (inCommandItem->command == "bowpidintegratorerror") {
        if (request) {
            commandResponses->push_back({ "bpie:" + String(integratorIgnoreBelow), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            integratorIgnoreBelow = inCommandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            commandResponses->push_back({"bpie:" + String(integratorIgnoreBelow), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidmaxerror") {
        if (request) {
            commandResponses->push_back({ "bpme:" + String(pidMaxError), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            pidMaxError = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bpme:" + String(pidMaxError), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpidpeakerror") {
        if (request) {
            commandResponses->push_back({ "bpperr:" + String(pidPeakError), InfoRequest });
            pidPeakError = 0;
        }
    } else
    if (inCommandItem->command == "bowmotorspeedmax") {
        if (request) {
            commandResponses->push_back({ "bmsx:" + String(maxSpeedHz), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMaxSpeedHz(inCommandItem->argument[0].toFloat()); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"bmsx:" + String(maxSpeedHz), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorspeedmin") {
        if (request) {
            commandResponses->push_back({ "bmsi:" + String(minSpeedHz), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setMinSpeedHz(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"bmsi:" + String(minSpeedHz), InfoRequest});
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

/*
PIDController::PIDController(CalibrationData &inCalibrationData, bowIO &inBowIO) {
    calibrationDataConnect = &inCalibrationData;
    bowIOConnect = &inBowIO;
}
*/
PIDController::PIDController(DCMotorControl &inDCMotorControl) {
    dcMotorControl = &inDCMotorControl;
}
/*
/// Resets the PID integral
void PIDController::pidReset() {
    integral = 0;
}
*/
/// Set PID target speed, check that it doesnt go above maxHz or below minHz
bool PIDController::setPIDTarget(float inPIDTargetSpeed) {
//    if (((_pidTargetSpeed > calibrationDataConnect->maxHz) || (_pidTargetSpeed < calibrationDataConnect->minHz)) && _pidTargetSpeed != 0) {
    if (((inPIDTargetSpeed > maxSpeedHz) || (inPIDTargetSpeed < minSpeedHz)) && inPIDTargetSpeed != 0) {
    //if (((_pidTargetSpeed > *maxHz) || (_pidTargetSpeed < *minHz)) && _pidTargetSpeed != 0) {
        debugPrintln("PID Target out of range!", Hardware);
        return false;
    }
    debugPrintln("Setting PID target to " + String(inPIDTargetSpeed), Hardware);
    setPIDTargetUnsafe(inPIDTargetSpeed);
    if (inPIDTargetSpeed == 0) { pidReset(); }
    return true;
}
/*
/// Set PID target speed without any safety checks, used by setPIDTarget
void PIDController::setPIDTargetUnsafe(float inPIDTargetSpeed) {
    pidTargetSpeed = inPIDTargetSpeed;
}
*/

/*
/// Returns the PID target speed
float PIDController::getPIDTarget() {
    return pidTargetSpeed;
}
*/
/// PID calculation function to be called at pidUpdateInterval, called by pidInterruptCaller
/// \todo add Integral injection and other pre-loading parameters to help bow start and change
/// \todo final check so PWM is never out of range min/max
/// \todo startup injection of fundamental maybe?
void PIDController::pidControl() {
    // Calculate the error between the target speed and the current speed
//    float currentSpeed = bowIOConnect->dcMotorControl->getLastTachoFreq();
    float currentSpeed = dcMotorControl->getLastTachometerFreq();

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
//    bowIOConnect->dcMotorControl->setSpeedPWM(static_cast<uint16_t>(output));
    dcMotorControl->setSpeedPWM(static_cast<uint16_t>(output));

    // Store the current error for the next iteration
    previousError = error;
};
/*
float PIDController::getPIDPeakError() {
    float a = pidPeakError;
    pidPeakError = 0;
    return a;
}
*/
void PIDController::measureTimeToTarget(float inPIDTargetSpeed) {
    elapsedMillis timeToTarget;
    elapsedMicros overshootTime;
    float startSpeed = getPIDTarget();

    timeToTarget = 0;
    float overshoot = 0;
    float undershoot = -1;
    float freq;
    int iterations = 0;

    setPIDTarget(inPIDTargetSpeed);
    while(iterations < 10) {
        if (round(dcMotorControl->getAverageTachometerFreq()) == round(inPIDTargetSpeed)) { iterations++; }

        overshootTime = 0;
        while (overshootTime < 10) {
            freq = dcMotorControl->getLastTachometerFreq();
            if (freq > overshoot) { overshoot = freq; }
            if ((freq < undershoot) || (undershoot = -1 )) { undershoot = freq; }
        }
        if (timeToTarget > 1000) {
            debugPrintln("Couldn't get to target speed in time (reached " + String(dcMotorControl->getAverageTachometerFreq()) + " Hertz)", Error);
            return;
        }
    }

    debugPrint("Going from " + String(startSpeed) + " -> " + String(inPIDTargetSpeed) + " in " + String(timeToTarget) + "ms", InfoRequest);
    if (inPIDTargetSpeed > startSpeed) { debugPrintln(", overshoot " + String(overshoot), InfoRequest); }
    if (inPIDTargetSpeed < startSpeed) { debugPrintln(", undershoot " + String(undershoot), InfoRequest); }
}


String PIDController::dumpData() {
    String dump = "";
    dump += "bpki:" + String(Ki) + ",";
    dump += "bpkp:" + String(Kp) + ",";
    dump += "bpkd:" + String(Kd) + ",";
    dump += "bpie:" + String(integratorIgnoreBelow) + ",";
    dump += "bpme:" + String(pidMaxError) + ",";
    dump += "bmsx:" + String(maxSpeedHz) + ",";
    dump += "bmsi:" + String(minSpeedHz);

    return dump;
}

#endif // PIDCONTROLLER_C
