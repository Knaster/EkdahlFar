#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include "generic_functions/dcmotorcontrol.hpp"

class PIDController : public Module {
public:
    SETMODULEID("pid", "pid", "PID controller v1.0", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(targetFreq, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ki, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(kp, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(kd, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(integratorError, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(reset, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(maxError, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(peakError, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(motorSpeedMax, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(motorSpeedMin, PIDController)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(measureTimeToTarget, PIDController)

    PIDController(DCMotorControl &DCMotorControl);

private:
    float pKp = 500;                           ///< PID P multiplier 100
    float pKi = 7;                             ///< PID I multiplier 2
    float pKd = 200;                            ///< PID D multiplier 40
    float previousError = 0.0;                ///< Contains the last frequency error calculated by the PID, given in Hertz
    float integratorIgnoreBelow = 0.1;        ///< The PID will ignore any integral errors under this threshold 0.4
    float pidMaxError = 50;                   ///< Max PID error per loop, essentially sets acceleration

    float KpTerm = 0;                         ///< Calculated P term
    float KiTerm = 0;                         ///< Calculated I term
    float KdTerm = 0;                         ///< Calculated D term
    float integral = 0.0;                     ///< Contains the integral used by the PID

    DCMotorControl *dcMotorControl;

    float pidTargetSpeed = 0;                 ///< PID target speed
    float pidPeakError = 0;

    float minSpeedHz = 20;
    float maxSpeedHz = 550;

public:
    //PIDController(CalibrationData &inCalibrationData, bowIO &inBowIO);
 /***** Module specific commands mirroring serially attainable commands *****/
    void pidReset() { integral = 0; };

    bool setPIDTarget(float inPIDTargetSpeed);

    float getPIDTarget() { return pidTargetSpeed; };

    float getPIDPeakError();

    float measureTimeToTarget(float _pidTargetSpeed);

    void setMinSpeedHz(float inSpeed) { minSpeedHz = inSpeed; }

    float getMinSpeedHz() { return minSpeedHz; };

    void setMaxSpeedHz(float inSpeed) { maxSpeedHz = inSpeed; }

    float getMaxSpeedHz() { return maxSpeedHz; }

/***** Hidden serial commands *****/
    void setPIDTargetUnsafe(float inPIDTargetSpeed) { pidTargetSpeed = inPIDTargetSpeed; };

/***** Internal commands for stand-alone and semi-modular use *****/
    void pidControl();

};

#endif // PIDCONTROLLER_H
