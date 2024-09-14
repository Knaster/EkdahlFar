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
#ifndef MUTECALIBRATION_CPP
#define MUTECALIBRATION_CPP

#include "mutecalibration.hpp"

calibrateMute::calibrateMute(mute &t_Mute, bowIO &t_bowIO, bowControl &t_bowControl) {
    m_muteConnect = &t_Mute;
    m_bowIOConnect = &t_bowIO;
    m_bowControlConnect = &t_bowControl;
//    m_audioAnalyze = audioRMS;
}

float calibrateMute::findLevel() {
    float j = 0;
    for (int i=0; i<10; i++) {
        while (!audioPeak->available()) {};
/*            debugPrintln("Audio peak not avaliable!", debugPrintType::TextInfo);
        } else {*/
            j += audioPeak->read();
//        }

        delay(1);
    }

    j = (j / 10);
    return j;
}

bool calibrateMute::findLevelSilence() {
    m_muteConnect->homeMute();

    return true;
}
/*
bool calibrateMute::findlevelFundamentalPeak() {
    return true;
}
*/
bool calibrateMute::findMuteLevels() {
    bool debugReport = debugPrintEnabled[debugPrintType::Debug];
    debugPrintEnabled[debugPrintType::Debug] = false;

    debugPrintln("Starting the mute level calibration", debugPrintType::TextInfo);

    m_muteConnect->homeMute();
    servoStepper *ss = m_muteConnect->stepServoStepper;

//    audioFilterBiquad->setLowpass(0, m_bowControlConnect->calibrationDataConnect->fundamentalFrequency, 0.707);

    m_bowIOConnect->disableBowPower();
    m_bowControlConnect->run = 0;
    m_bowControlConnect->bowRest(1);
    m_bowIOConnect->stepServoStepper->completeTask();

    delay(1000);
    levelSilence = findLevel();
    if (levelSilence == 0) { levelSilence = 0.01; }

    debugPrintln("Silence level is set to " + String(levelSilence), debugPrintType::TextInfo);

    m_bowControlConnect->PIDon = true;
    m_bowControlConnect->run = 1;
    m_bowIOConnect->enableBowPower();
    m_bowControlConnect->setHarmonic(0);

    uint16_t testPressure = m_bowControlConnect->calibrationDataConnect->firstTouchPressure +
        ((m_bowControlConnect->calibrationDataConnect->stallPressure - m_bowControlConnect->calibrationDataConnect->firstTouchPressure) / 2);
    debugPrintln("Setting tilt to " + String(testPressure), debugPrintType::TextInfo);
    m_bowIOConnect->setTiltPWM(testPressure);
    m_bowIOConnect->waitForTiltToComplete();
    delay(2500);
    levelFundamental = findLevel();
    debugPrintln("Pickup fundamental ampltiude is " + String(levelFundamental), debugPrintType::TextInfo);


    #define levelStepSize 100
//    #define levelArrayCount ((65535 / levelStepSize) + 1)
//    float levelArray[levelArrayCount];
//    uint16_t levelIndex = 0;

    float level = 0;
    uint16_t mutePos = 0;
    do {
        ss->setPosition(mutePos);
        ss->completeTask();
        mutePos += levelStepSize;
        level = findLevel();

//        levelArray[levelIndex] = level;
//        levelIndex++;

        debugPrintln("Level " + String(level) + " at position " + String(mutePos), debugPrintType::TextInfo);
        if (level < minAmplitude) { break; }
        if (level < levelSilence) { break; }
    } while ((mutePos < stallPosition) && ((level >= minAmplitude) || (level >= levelSilence)));

    m_muteConnect->setFullMutePosition(mutePos);
    m_muteConnect->setHalfMutePosition(mutePos / 2);
    debugPrintln("Full mute set at " + String(m_muteConnect->getFullMutePosition()), debugPrintType::TextInfo);
/*
    uint16_t halfMute = 0;

    float maxLevel = 0;
    uint16_t maxLevelStep = 0;

    for (int i = (mutePos / levelStepSize); i > 0; i--) {
        if ((levelArray[i] >= levelFundamental) && (halfMute == 0)) { halfMute = i * levelStepSize; }
        if (maxLevel < levelArray[i]) {
            maxLevel = levelArray[i];
            maxLevelStep = i * levelStepSize;
        }
    }

    m_muteConnect->setHalfMutePosition(halfMute);
    m_muteConnect->setRestPosition(maxLevelStep);
    debugPrintln("Half mute set at " + String(halfMute) + ", rest position set to " + String(maxLevelStep), debugPrintType::TextInfo);
*/
    debugPrintln("Finished level set", debugPrintType::TextInfo);

    m_bowControlConnect->run = 0;
    m_bowControlConnect->bowRest(1);
    m_bowIOConnect->disableBowPower();
    m_bowIOConnect->stepServoStepper->completeTask();

//    audioFilterBiquad->setLowpass(0, noteFreqCutoff, 0.707);
    debugPrintEnabled[debugPrintType::Debug] = debugReport;

    return true;
}

bool calibrateMute::findMuteStall() {
    bool debugReport = debugPrintEnabled[debugPrintType::Debug];
    debugPrintEnabled[debugPrintType::Debug] = false;

    debugPrintln("Starting mute stall calibration", debugPrintType::TextInfo);

    m_muteConnect->homeMute();
    servoStepper *ss = m_muteConnect->stepServoStepper;
/*
    debugPrintln("Mute home positions: rising, forward " +
        String(ss->homingPoint[ss->eEdgeType::EDGERISING][ss->eStepDirection::FORWARD]) +
        ", falling forward " + String(ss->homingPoint[ss->eEdgeType::EDGEFALLING][ss->eStepDirection::FORWARD]) +
        ", rising reverse " + String(ss->homingPoint[ss->eEdgeType::EDGERISING][ss->eStepDirection::REVERSE]) +
        ", falling reverse " + String(ss->homingPoint[ss->eEdgeType::EDGEFALLING][ss->eStepDirection::REVERSE]),
        debugPrintType::TextInfo);
*/
    bool autoCorrect = ss->autoCorrectPosition;
    ss->autoCorrectPosition = false;

    ss->setPosition(maxTestStep);
    if (!ss->completeTask()) {
        debugPrintln("Failed at going to max pressure", debugPrintType::TextInfo);
        return false;
    }
    bool initialHomingSensed = ss->homingSensed;

    debugPrintln("Starting at max with homing sense " + String(initialHomingSensed), debugPrintType::TextInfo);

    uint16_t i = maxTestStep;
    do {
        i--;
        ss->setPosition(i);
        ss->completeTask();
        if (i == 0) {
            debugPrintln("Edge detection failed with edge " + String(ss->homingSensed), debugPrintType::Error);
            return false;
        }
    } while (ss->homingSensed == initialHomingSensed);


    debugPrintln("Homing sense changed to " + String(ss->homingSensed) + " direction " + String(ss->moveDirection) + " step no " + String(ss->currentStep), debugPrintType::TextInfo);

    servoStepper::eEdgeType edge;
    if (initialHomingSensed == homeSenseInactive) { edge = ss->eEdgeType::EDGERISING; } else { edge = ss->eEdgeType::EDGEFALLING; };
    uint16_t homingPoint = ss->homingPoint[edge][ss->eStepDirection::REVERSE];
    int32_t homingDifference = ss->currentStep - homingPoint;
    float mult = 65535 / (stepsPerRevolution * microSteps);
    int32_t maxPosition = maxTestStep - (homingDifference * mult) - maxStepRetract;

    stallPosition = maxPosition;

    debugPrintln("Homing point is at " + String(homingPoint) + " difference is " + String(homingDifference) + " making the max position " + String(maxPosition), debugPrintType::TextInfo);
/*
    debugPrintln("Setting max position for test", debugPrintType::TextInfo);
    ss->setPosition(maxPosition);
    ss->completeTask();
*/
    ss->autoCorrectPosition = autoCorrect;
    debugPrintEnabled[debugPrintType::Debug] = debugReport;
    return true;
}

bool calibrateMute::findMuteFirstContact(){
    return true;
}

bool calibrateMute::findMuteSilence(){
    return true;
}

bool calibrateMute::calibrateAll(){
    //if (stallPosition == 0) {
    if (!findMuteStall()) {
        return false;
    }
    //}

    if (!findMuteLevels()) {
        return false;
    }

    m_muteConnect->homeMute();
    return true;
}

#endif
