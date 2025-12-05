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

calibrateMute::calibrateMute(MuteControl &t_Mute, bowIO &t_bowIO, bowControl &t_bowControl) {
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
//    m_muteConnect->homeMute();
    m_muteConnect->home();


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

    // Home mute and set mute and bow position at zero and turn off bow
    //m_muteConnect->homeMute();
    m_muteConnect->home();
    //servoStepper *ss = m_muteConnect->tmc2209ServoStepper->stepServoStepper;

//    audioFilterBiquad->setLowpass(0, m_bowControlConnect->calibrationDataConnect->fundamentalFrequency, 0.707);

    m_bowIOConnect->dcMotorControl->disableMotorPower();
    m_bowControlConnect->run = 0;
    //m_bowControlConnect->bowRest(1);
    //m_bowIOConnect->tmc2209ServoStepper->stepServoStepper->completeTask();
    m_bowIOConnect->bowPressure->rest(1);
    if (!m_bowIOConnect->bowPressure->completeTask()) {
        debugPrintln("Error setting pressure", debugPrintType::Error);
        return false;
    }

    // Find the pickup output level that is considered silence

    delay(3000);
    levelSilence = findLevel();
    if (levelSilence == 0) { levelSilence = 0.01; }

    debugPrintln("Silence level is set to " + String(levelSilence), debugPrintType::TextInfo);
    if (levelSilence >= (minFundamentalAmplitude / 2)) {
        debugPrintln("Silence level is too high!", debugPrintType::Error);
        return false;
    }

    // Enable bow and play the fundamental, set the test pressure to in between max & min pressure

    m_bowControlConnect->PIDon = true;
    m_bowControlConnect->run = 1;
//    m_bowIOConnect->enableBowPower();
    m_bowIOConnect->dcMotorControl->enableMotorPower();
    m_bowControlConnect->setHarmonic(0);

//    uint16_t testPressure = m_bowControlConnect->calibrationDataConnect->firstTouchPressure +
//        ((m_bowControlConnect->calibrationDataConnect->stallPressure - m_bowControlConnect->calibrationDataConnect->firstTouchPressure) / 4);
    uint16_t testPressure = m_bowIOConnect->bowPressure->getEngagePressure() +
        ((m_bowIOConnect->bowPressure->getMaxPressure() - m_bowIOConnect->bowPressure->getEngagePressure()) / 4);

    debugPrintln("Setting tilt to " + String(testPressure), debugPrintType::TextInfo);
    //m_bowIOConnect->setTiltPWM(testPressure);
    m_bowIOConnect->bowPressure->setHardwarePressure(testPressure);
    debugPrintln("Waiting for Pressure setting to finish.", debugPrintType::TextInfo);
    //m_bowIOConnect->waitForTiltToComplete();
    if (!m_bowIOConnect->bowPressure->completeTask()) {
        debugPrintln("Error setting pressure", debugPrintType::Error);
        return false;
    }
    debugPrintln("Pause for a little..", debugPrintType::TextInfo);
    delay(2500);

    debugPrintln("Starting fundamental level test, need to reach " + String(minFundamentalAmplitude), debugPrintType::TextInfo);
    // Wait for the bow to settle and then reord the fundamental level
    do {
        testPressure += 100;
//        m_bowIOConnect->setTiltPWM(testPressure);
        m_bowIOConnect->bowPressure->setHardwarePressure(testPressure);
        levelFundamental = findLevel();
        debugPrintln("Fundamental level " + String(levelFundamental), debugPrintType::TextInfo);
//    } while((levelFundamental < minFundamentalAmplitude) && (testPressure < m_bowControlConnect->calibrationDataConnect->stallPressure));
    } while((levelFundamental < minFundamentalAmplitude) && (testPressure < m_bowIOConnect->bowPressure->getMaxPressure()));

    if (levelFundamental < minFundamentalAmplitude) {
        debugPrintln("Pickup volume too low!", debugPrintType::Error);
        return false;
    }

    debugPrintln("Pickup fundamental ampltiude is " + String(levelFundamental), debugPrintType::TextInfo);

    #define levelStepSize 100

    float level = 0;
    uint16_t mutePos = 0;
    m_muteConnect->setHardwarePosition(mutePos);
    if (!m_muteConnect->completeTask(5000)) {
        debugPrintln("Error moving to position", debugPrintType::Error);
        return false;
    }

    do {
        //ss->setPosition(mutePos);
        m_muteConnect->setHardwarePosition(mutePos);
        //ss->completeTask();
        if (!m_muteConnect->completeTask(100)) {
            debugPrintln("Error moving to position", debugPrintType::Error);
            return false;
        }

        mutePos += levelStepSize;
        level = findLevel();

        debugPrintln("Level " + String(level) + " at position " + String(mutePos), debugPrintType::TextInfo);
        if (level < minAmplitude) { break; }
        if (level < levelSilence) { break; }
    } while ((mutePos < stallPosition) && ((level >= minAmplitude) || (level >= levelSilence)));

    if (mutePos == levelStepSize) {
        debugPrintln("Error finding mute level", debugPrintType::Error);
        return false;
    }

    m_muteConnect->setFullMutePosition(mutePos);
    m_muteConnect->setHalfMutePosition(mutePos / 2);
    debugPrintln("Full mute set at " + String(m_muteConnect->getFullMutePosition()), debugPrintType::TextInfo);

    debugPrintln("Finished level set", debugPrintType::TextInfo);

    m_bowControlConnect->run = 0;
    //m_bowControlConnect->bowRest(1);
    //m_bowIOConnect->disableBowPower();
    //m_bowIOConnect->tmc2209ServoStepper->stepServoStepper->completeTask();
    m_bowIOConnect->bowPressure->rest(1);
    m_bowIOConnect->dcMotorControl->disableMotorPower();
    if (!m_bowIOConnect->bowPressure->completeTask()) {
        debugPrintln("Error setting pressure", debugPrintType::Error);
        return false;
    }

//    audioFilterBiquad->setLowpass(0, noteFreqCutoff, 0.707);
    debugPrintEnabled[debugPrintType::Debug] = debugReport;

    return true;
}

bool calibrateMute::findMuteStall() {
    bool debugReport = debugPrintEnabled[debugPrintType::Debug];
    debugPrintEnabled[debugPrintType::Debug] = false;

    debugPrintln("Starting mute stall calibration", debugPrintType::TextInfo);

//    m_muteConnect->homeMute();
    m_muteConnect->home();
//    servoStepper *ss = m_muteConnect->tmc2209ServoStepper->stepServoStepper;

/*
    debugPrintln("Mute home positions: rising, forward " +
        String(ss->homingPoint[ss->eEdgeType::EDGERISING][ss->eStepDirection::FORWARD]) +
        ", falling forward " + String(ss->homingPoint[ss->eEdgeType::EDGEFALLING][ss->eStepDirection::FORWARD]) +
        ", rising reverse " + String(ss->homingPoint[ss->eEdgeType::EDGERISING][ss->eStepDirection::REVERSE]) +
        ", falling reverse " + String(ss->homingPoint[ss->eEdgeType::EDGEFALLING][ss->eStepDirection::REVERSE]),
        debugPrintType::TextInfo);
*/
//    bool autoCorrect = ss->autoCorrectPosition;
//    ss->autoCorrectPosition = false;
    bool autoCorrect = m_muteConnect->getAutoCorrect();
    m_muteConnect->setAutoCorrect(false);

    //ss->setPosition(maxTestStep);
    m_muteConnect->setHardwarePosition(maxTestStep);
    //if (!ss->completeTask()) {
    if (!m_muteConnect->completeTask()) {
        debugPrintln("Failed at going to max pressure", debugPrintType::TextInfo);
        return false;
    }
//    bool initialHomingSensed = ss->getHomingSensed();
    bool initialHomingSensed = m_muteConnect->getHomingSensed();

    debugPrintln("Starting at max with homing sense " + String(initialHomingSensed), debugPrintType::TextInfo);

    uint16_t i = maxTestStep;
    m_muteConnect->setHardwarePosition(i);
    if (!m_muteConnect->completeTask(3000)) {
        debugPrintln("Failed setting position", debugPrintType::Error);
        return false;
    }

    do {
        i--;
//        ss->setPosition(i);
//        ss->completeTask();
        m_muteConnect->setHardwarePosition(i);
        if (!m_muteConnect->completeTask(100)) {
            debugPrintln("Failed setting position", debugPrintType::Error);
            return false;
        }

        if (i == 0) {
//            debugPrintln("Edge detection failed with edge " + String(ss->getHomingSensed()), debugPrintType::Error);
            debugPrintln("Edge detection failed with edge " + String(m_muteConnect->getHomingSensed()), debugPrintType::Error);
            return false;
        }
//    } while (ss->getHomingSensed() == initialHomingSensed);
    } while (m_muteConnect->getHomingSensed() == initialHomingSensed);


//    debugPrintln("Homing sense changed to " + String(ss->getHomingSensed()) + " direction " + String(ss->getMoveDirection()) + " step no " + String(ss->getCurrentStep()), debugPrintType::TextInfo);
    debugPrintln("Homing sense changed to " + String(m_muteConnect->getHomingSensed()) + " direction " + String(m_muteConnect->getMoveDirection()) + " step no " +
                 String(m_muteConnect->getCurrentStep()), debugPrintType::TextInfo);

    eEdgeType edge;
    if (initialHomingSensed == homeSenseInactive) { edge = eEdgeType::EDGERISING; } else { edge =eEdgeType::EDGEFALLING; };
//    uint16_t homingPoint = ss->getHomingPoint(edge, ss->eStepDirection::REVERSE);
    uint16_t homingPoint = m_muteConnect->getHomingPoint(edge, eStepDirection::REVERSE);
//    int32_t homingDifference = ss->getCurrentStep() - homingPoint;
    int32_t homingDifference = m_muteConnect->getCurrentStep() - homingPoint;
    float mult = 65535 / (stepsPerRevolution * microSteps);
    int32_t maxPosition = maxTestStep - (homingDifference * mult) - maxStepRetract;

    stallPosition = maxPosition;

    debugPrintln("Homing point is at " + String(homingPoint) + " difference is " + String(homingDifference) + " making the max position " + String(maxPosition), debugPrintType::TextInfo);
/*
    debugPrintln("Setting max position for test", debugPrintType::TextInfo);
    ss->setPosition(maxPosition);
    ss->completeTask();
*/
    //ss->autoCorrectPosition = autoCorrect;
    m_muteConnect->setAutoCorrect(autoCorrect);
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
    if (!findMuteStall()) {
        return false;
    }

    if (!findMuteLevels()) {
        return false;
    }

    m_muteConnect->home();
    return true;
}

#endif
