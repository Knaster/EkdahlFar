#include <sys/_stdint.h>

#ifndef CALIBRATE_H
#define CALIBRATE_H

struct CalibrationData {
    float minHz = 19.05;                    ///< Minimum bow speed that the motor and controller can handle, in Hertz
    uint16_t minSpeedPWM = 10619;           ///< PWM value used to obtain minHz \todo USED FOR CALIBRATION ONLY, REPLACE WITH LOCAL WHEN NEEDED

    float maxHz = 539.49;                   ///< Maximum bow speed that the motor and controller can handle, in Hertz
    uint16_t maxSpeedPWM = 64564;           ///< PWM value used to obtain maxHz \todo USED FOR CALIBRATION ONLY, REPLACE WITH LOCAL WHEN NEEDED

    uint16_t minInertialPWM = 10619;

    uint16_t firstTouchPressure = 16460;     ///< Minimum "usable" PWM value used for the tilt servo. Calculated as the first point of where the bow speed slows down i.e. touches the string
    uint16_t stallPressure = 51063;     ///< Maximum "usable" PWM value used for the tilt servo. Calculated as the last point before the bow motor stalls

    float fundamentalFrequency = 66;    ///< The fundamental frequency of the string in Hertz

    int16_t lowerHarmonic = -48;
    int16_t upperHarmonic = 49;

    float maxHzPIDFullPressure = -1;

    uint16_t restPosition = 0;
}; // calibrationData;

/** Class for finding out the physical limitations and properties of a bow string unit
  *
  * The proper scheme for using the routines is to
  * - Find the physical minimum and maximum bow speed of the motor using hardware PWM
  * - Find the minimum and maximum bow pressure by
  *   - Setting the bow speed to the minimum speed limit obtained by previous routines and record the first bow pressure where the motor speed goes down, aka where the bow is touching the string
  *   - Setting the bow speed to the maximum speed limit obtained by previous routines and record at what pressure the motor stops completely (stalls)
  */
class calibrate {
public:

    bowIO *bowIOConnect;  ///< bowIO class associated with this calibration class
    bowControl *bowControlConnect;

    #define testDelay 1 ///< Delay used in conjunction with setTiltPWM for various tests
    #define deviation 2 ///< Acceptable bow speed deviation in Hertz for various tests

    #define minTestFreq 1
    #define maxTestFreq 1000

    #define minPermissibleBowSpeedForCalibration 20
    #define maxPowerUseMultiplierDuringCalibration 0.75

//    int EEPROM_offset = 0;

    CalibrationData *calibrationData;

    calibrate(bowIO &_bowIO, CalibrationData &__calibrationData, bowControl &_bowControl);

    void startAudioAnalyzing();
    void stopAudioAnalyzing();

    bool waitForBowToStabilize(uint16_t maxIterations);
private:
    bool findMinPressure();
    bool findMaxPressure();
public:
    uint16_t maxTestPressure = 50000;
    uint16_t pressureTestRetract = 7500;
    bool findMinMaxPressure();
    bool findMinMaxSpeedPWM();
    bool findMinMaxSpeedPID();

    bool calibrateAll();

    String dumpData();
};
#endif
