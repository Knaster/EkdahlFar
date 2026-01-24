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
#ifndef MUTECALIBRATION_HPP
#define MUTECALIBRATION_HPP

class CalibrateMute {
public:
    MuteControl *muteControl;
    BowControl *bowControl;
    HarmonicSeriesHandler *harmonicSeriesHandler;

    uint16_t maxTestStep = 60000;

    #define maxStepRetract 2000
    #define minAmplitude 0.01
    #define minFundamentalAmplitude 0.3

    CalibrateMute(MuteControl &inMuteControl, BowControl &inBowControl, HarmonicSeriesHandler &inHarmonicSeriesHandler);

    eCalibrationResult calibrateAll();

private:
    float levelSilence = 0;
    float levelFundamental = 0;
    uint16_t stallPosition = 0;

    float findLevel();

    bool findLevelSilence();
    eCalibrationResult findMuteLevels();

    eCalibrationResult findMuteStall();
    eCalibrationResult findMuteFirstContact();
    bool findMuteSilence();

};

#endif
