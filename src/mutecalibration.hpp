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

class calibrateMute {
public:
    mute *m_muteConnect;
    bowIO *m_bowIOConnect;
    bowControl *m_bowControlConnect;

    uint16_t maxTestStep = 50000;

    #define maxStepRetract 2000
    #define minAmplitude 0.01

    calibrateMute(mute &t_Mute, bowIO &t_bowIO, bowControl &t_bowControl);

    bool calibrateAll();

private:
    float levelSilence = 0;
    float levelFundamental = 0;
//    float levelFundamentalRMS = 0;
    uint16_t stallPosition = 0;

    float findLevel();

    bool findLevelSilence();
//    bool findlevelFundamentalPeak();
//    bool findLevelFundamentalRMS();
    bool findMuteLevels();

    bool findMuteStall();
    bool findMuteFirstContact();
    bool findMuteSilence();

};

#endif
