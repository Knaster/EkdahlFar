#ifndef CALIBRATIONHELPERS_H
#define CALIBRATIONHELPERS_H

enum eCalibrationResult { CR_Err_Undefined = 0, CR_Ok = 1, CR_Err_BowPressure = 2, CR_Err_BowStabilize = 3, CR_Err_Timeout = 4, CR_Err_MotorFault = 5, CR_Err_MotorFrequencyReading = 6,
                          CR_Err_AudioTooHigh = 7, CR_Err_AudioTooLow = 8, CR_Err_MutePressure = 9, CR_Err_MotorNotCalibrated = 10 };


bool exitWithError(eCalibrationResult error) {
    switch(error) {
    case CR_Err_BowPressure:
        debugPrintln("Error setting bow pressure", debugPrintType::Error);
        break;
    case CR_Err_BowStabilize:
        debugPrintln("Error getting bowing wheel speed to stabilize", debugPrintType::Error);
        break;
    case CR_Err_MotorFault:
        debugPrintln("Error; motor fault", debugPrintType::Error);
        break;
    case CR_Err_MotorFrequencyReading:
        debugPrintln("Error reading bowing wheel frequency", debugPrintType::Error);
        break;
    case CR_Err_Timeout:
        debugPrintln("Error; timeout", debugPrintType::Error);
        break;
    case CR_Err_Undefined:
        debugPrintln("Error; undefined problem", debugPrintType::Error);
        break;
    case CR_Err_AudioTooHigh:
        debugPrintln("Error; audio too loud", debugPrintType::Error);
        break;
    case CR_Err_AudioTooLow:
        debugPrintln("Error; audio too low", debugPrintType::Error);
        break;
    case CR_Err_MutePressure:
        debugPrintln("Error setting mute pressure", debugPrintType::Error);
        break;
    case CR_Err_MotorNotCalibrated:
        debugPrintln("Error; motor hasn't been calibrated yet", debugPrintType::Error);
        break;
    case CR_Ok:
        break;
    }
    return false;
}

#endif // CALIBRATIONHELPERS_H
