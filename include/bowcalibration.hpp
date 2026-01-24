/*
 * This file is part of The Ekdahl FAR firmware.
 *
 * The Ekdahl FAR firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Ekdahl FAR firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Ekdahl FAR firmware. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2024 Karl Ekdahl
 */
#include <sys/_stdint.h>

#ifndef CALIBRATE_H
#define CALIBRATE_H

#include "../src/debugprint.hpp"
#include "../src/calibrationhelpers.hpp"

/** Class for finding out the physical limitations and properties of a bow string unit
  *
  * The proper scheme for using the routines is to
  * - Find the physical minimum and maximum bow speed of the motor using hardware PWM
  * - Find the minimum and maximum bow pressure by
  *   - Setting the bow speed to the minimum speed limit obtained by previous routines and record the first bow pressure where the motor speed goes down, aka where the bow is touching the string
  *   - Setting the bow speed to the maximum speed limit obtained by previous routines and record at what pressure the motor stops completely (stalls)
  */

class CalibrateBow {
public:
    BowControl *bowControl;

    #define testDelay 1 ///< Delay used in conjunction with setTiltPWM for various tests
    #define deviation 2 ///< Acceptable bow speed deviation in Hertz for various tests

    #define minTestFreq 1
    #define maxTestFreq 1000

    #define minPermissibleBowSpeedForCalibration 20
    #define maxPowerUseMultiplierDuringCalibration 0.75

    CalibrateBow(BowControl *inBowControl);

    bool waitForBowToStabilize(uint16_t maxIterations);
private:
    eCalibrationResult findMinPressure();
    eCalibrationResult findMaxPressure();
    eCalibrationResult safeExitWithResult(eCalibrationResult result, bool pidState = true);
public:

    uint16_t maxTestPressure = 60000;
    uint16_t pressureTestRetract = 7500;
    eCalibrationResult findMinMaxPressure();
    eCalibrationResult findMinMaxSpeedPWM();
    //eCalibrationResult findMinMaxSpeedPID();

    bool calibrateAll();

//    String dumpData();
};
#endif
