#ifndef CALIBRATIONHELPERS_H
#define CALIBRATIONHELPERS_H

enum eCalibrationResult { CR_Err_Undefined = 0, CR_Ok = 1, CR_Err_BowPressure = 2, CR_Err_BowStabilize = 3, CR_Err_Timeout = 4, CR_Err_MotorFault = 5, CR_Err_MotorFrequencyReading = 6,
                          CR_Err_AudioTooHigh = 7, CR_Err_AudioTooLow = 8, CR_Err_MutePressure = 9, CR_Err_MotorNotCalibrated = 10 };


bool exitWithError(eCalibrationResult error);

#endif // CALIBRATIONHELPERS_H
