#ifndef PIDCONTROLLER_C
#define PIDCONTROLLER_C

#include "pidcontroller.hpp"

const ModuleCommandDeclaration PIDController::moduleCommands[] = {
    { "targetfrequency", "tf", "float", "Sets the PID target frequency", false, false, &s_targetFreq, },
    { "ki", "ki", "float", "Sets the Ki parameter of the PID of the selected bow", false, true, &s_ki },
    { "kp", "kp", "float", "Sets the Kp parameter of the PID of the selected bow", false, true, &s_kp },
    { "kd", "kd", "float", "Sets the Kd parameter of the PID of the selected bow", false, true, &s_kd },
    { "integratorerror", "ie", "float", "Sets the lower threshold of error values for the PID integrator to ignore of the selected bow", false, true, &s_integratorError },
    { "reset", "re", "-", "Resets the PID of the selected bow", false, false, &s_reset },
    { "maxerror", "xe", "float", "Maximum error to correct in each PID loop, essentially sets acceleration", false, true, &s_maxError },
    { "peakerror", "pe", "float", "Get latest PID peak error", false, false, &s_peakError },
    { "motorspeedmax", "msx", "float", "Bow motor maximum speed limit", false, true, &s_motorSpeedMax },
    { "motorspeedmin", "msi", "float", "Bow motor minimum speed limit", false, true, &s_motorSpeedMin },
    { "measuretimetotarget", "mtt", "float", "Measure the time it takes to change from the current frequency to the target frequency", false, false, &s_measureTimeToTarget }
};

getModuleCount(PIDController)

PIDController::PIDController(DCMotorControl &inDCMotorControl) {
    moduleID = new ModuleID("pid", "pid", "PID controller v1.0", ModuleID::software);
    dcMotorControl = &inDCMotorControl;
}

CREATE_MODULE_COMMAND_FUNCTION(targetFreq, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pidTargetSpeed), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setPIDTarget(inCommandItem->argument[0].toFloat());
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pidTargetSpeed), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(ki, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pKi), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pKi = inCommandItem->argument[0].toFloat();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(pKi), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(kp, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pKp), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pKp = inCommandItem->argument[0].toFloat();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(pKp), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(kd, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pKd), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pKd = inCommandItem->argument[0].toFloat();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(pKd), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(integratorError, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(integratorIgnoreBelow), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        integratorIgnoreBelow = inCommandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(integratorIgnoreBelow), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(reset, PIDController) {
    if (!request) {
        inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
        previousError = 0;
        pidReset();
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(maxError, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pidMaxError), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pidMaxError = inCommandItem->argument[0].toInt();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(pidMaxError), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(peakError, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pidPeakError), InfoRequest });
        pidPeakError = 0;
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(motorSpeedMax, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(maxSpeedHz), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setMaxSpeedHz(inCommandItem->argument[0].toFloat()); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(maxSpeedHz), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(motorSpeedMin, PIDController) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(minSpeedHz), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setMinSpeedHz(inCommandItem->argument[0].toFloat());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(minSpeedHz), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(measureTimeToTarget, PIDController) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        float startSpeed = pidTargetSpeed;
        float endSpeed = inCommandItem->argument[0].toFloat();
        float time = measureTimeToTarget(endSpeed);
        if (time == -1) {
            inCommandResponses->push_back({thisItem.shortCommand + ":0", InfoRequest});
            return eProcessResult::CommandFailed;
        } else {
            inCommandResponses->push_back({thisItem.shortCommand + ":" + String(time) + ":" + String(startSpeed) + ":" + String(endSpeed), InfoRequest});
        }
    }
    return eProcessResult::Ok;
};
/// Set PID target speed, check that it doesnt go above maxHz or below minHz
bool PIDController::setPIDTarget(float inPIDTargetSpeed) {
    if (((inPIDTargetSpeed > maxSpeedHz) || (inPIDTargetSpeed < minSpeedHz)) && inPIDTargetSpeed != 0) {
        debugPrintln("PID Target out of range!", Hardware);
        return false;
    }
    debugPrintln("Setting PID target to " + String(inPIDTargetSpeed), Hardware);
    setPIDTargetUnsafe(inPIDTargetSpeed);
    if (inPIDTargetSpeed == 0) { pidReset(); }
    return true;
}

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
    KpTerm = pKp * error;
    KiTerm = pKi * integral;
    KdTerm = pKd * (error - previousError);

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
float PIDController::measureTimeToTarget(float inPIDTargetSpeed) {
    elapsedMillis timeToTarget;
    elapsedMicros overshootTime;
//    float startSpeed = getPIDTarget();

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
            //debugPrintln("Couldn't get to target speed in time (reached " + String(dcMotorControl->getAverageTachometerFreq()) + " Hertz)", Error);
            return -1;
        }
    }

    //debugPrint("Going from " + String(startSpeed) + " -> " + String(inPIDTargetSpeed) + " in " + String(timeToTarget) + "ms", InfoRequest);
    //if (inPIDTargetSpeed > startSpeed) { debugPrintln(", overshoot " + String(overshoot), InfoRequest); }
    //if (inPIDTargetSpeed < startSpeed) { debugPrintln(", undershoot " + String(undershoot), InfoRequest); }
    return timeToTarget;
}

#endif // PIDCONTROLLER_C
