#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include "dcmotorcontrol.cpp"

const serialCommandItem serialCommandsPID[] = {
    { "bowpidtargetfreq", "bptf", "float", "Sets the PID target frequency" },
    { "bowpidki", "bpki", "float", "Sets the Ki parameter of the PID of the selected bow" },
    { "bowpidkp", "bpkp", "float", "Sets the Kp parameter of the PID of the selected bow" },
    { "bowpidkd", "bpkd", "float", "Sets the Kd parameter of the PID of the selected bow" },
    { "bowpidintegratorerror", "bpie", "float", "Sets the lower threshold of error values for the PID integrator to ignore of the selected bow" },
    { "bowpidr", "bpir", "-", "Resets the PID of the selected bow" },
    { "bowpidmaxerror", "bpme", "float", "Maximum error to correct in each PID loop, essentially sets acceleration" },
    { "bowpidpeakerror", "bpperr", "float", "Get latest PID peak error"},

    { "bowmotorspeedmax", "bmsx", "float", "Bow motor maximum speed limit" },
    { "bowmotorspeedmin", "bmsi", "float", "Bow motor minimum speed limit"},

    { "bowpidtargetfreq", "bcf", "float", "Legacy command conversion" },
    { "bowpidtargetfreq", "bowcontrolfrequency", "float", "Legacy command conversion" }
};

const serialCommandItem serialCommandsPIDHidden[] =  {
    { "bowmeasuretimetotarget", "bmtt", "float", "Measure the time it takes to change from the current frequency to the target frequency" }
};

class PIDController {
public:
    PIDController(DCMotorControl &DCMotorControl);

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
    String dumpData();
public:
//    bowIO *bowIOConnect;                      ///< Pointer to associated bowIO object
//    CalibrationData *calibrationDataConnect; ///< Pointer to associated _calibrationData object

private:
    float Kp = 500;                           ///< PID P multiplier 100
    float Ki = 7;                             ///< PID I multiplier 2
    float Kd = 200;                            ///< PID D multiplier 40
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
