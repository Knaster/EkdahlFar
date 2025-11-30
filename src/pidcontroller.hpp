#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

class PIDController {
public:
    bowIO *bowIOConnect;                      ///< Pointer to associated bowIO object
    CalibrationData *calibrationDataConnect; ///< Pointer to associated _calibrationData object

    float Kp = 500;                           ///< PID P multiplier 100
    float Ki = 7;                             ///< PID I multiplier 2
    float Kd = 200;                            ///< PID D multiplier 40
    float previousError = 0.0;                ///< Contains the last frequency error calculated by the PID, given in Hertz
    float integratorIgnoreBelow = 0.1;        ///< The PID will ignore any integral errors under this threshold 0.4
//    uint16_t pidUpdateInterval = 1000;        ///< The interval at which the PID is being called, given in uS
    float pidMaxError = 50;                   ///< Max PID error per loop, essentially sets acceleration

    float KpTerm = 0;                         ///< Calculated P term
    float KiTerm = 0;                         ///< Calculated I term
    float KdTerm = 0;                         ///< Calculated D term
    float integral = 0.0;                     ///< Contains the integral used by the PID

    float pidTargetSpeed = 0;                 ///< PID target speed

private:
    float *maxHz = nullptr;
    float *minHz = nullptr;
    float (*getFrequency)();
    void (*setFrequency)(uint16_t);

    float pidPeakError = 0;
public:
    PIDController(CalibrationData &inCalibrationData, bowIO &inBowIO);

    void pidReset();
    bool setPIDTarget(float _pidTargetSpeed);
    void setPIDTargetUnsafe(float _pidTargetSpeed);
    float getPIDTarget();
    void pidControl();
//    void pidInterruptCaller();
    float getPIDPeakError();
    void measureTimeToTarget(float _pidTargetSpeed);
    String dumpData();
};

#endif // PIDCONTROLLER_H
