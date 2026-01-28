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

CalibrateMute::CalibrateMute(MuteControl &inMuteControl, BowControl &inBowControl, HarmonicSeriesHandler &inHarmonicSeriesHandler) {
    muteControl = &inMuteControl;
    bowControl = &inBowControl;
    harmonicSeriesHandler = &inHarmonicSeriesHandler;
}

float CalibrateMute::findLevel() {
    float j = 0;
    for (int i=0; i<10; i++) {
        while (!audioPeak->available()) {};
            j += audioPeak->read();
        delay(1);
    }

    j = (j / 10);
    return j;
}

bool CalibrateMute::findLevelSilence() {
    muteControl->home();

    return true;
}

eCalibrationResult CalibrateMute::findMuteLevels() {
    bool debugReport = debugPrintEnabled[debugPrintType::Debug];
    debugPrintEnabled[debugPrintType::Debug] = false;

    debugPrintln("Starting the mute level calibration", debugPrintType::TextInfo);

    // Home mute and set mute and bow position at zero and turn off bow
    muteControl->home();

//    audioFilterBiquad->setLowpass(0, bowControl->calibrationDataConnect->fundamentalFrequency, 0.707);
    bowControl->disableBowMotorPower();
    bowControl->setRun(false);
    bowControl->setBowRest();
    if (!bowControl->waitForPressureToSettle()) { return CR_Err_BowPressure; }

    // Find the pickup output level that is considered silence
    delay(3000);
    levelSilence = findLevel();
    if (levelSilence == 0) { levelSilence = 0.01; }

    debugPrintln("Silence level is set to " + String(levelSilence), debugPrintType::TextInfo);
    if (levelSilence >= (minFundamentalAmplitude / 2)) { return CR_Err_AudioTooHigh; }

    // Enable bow and play the fundamental, set the test pressure to in between max & min pressure

    bowControl->setPIDOn(true);
    bowControl->setRun(true);
    bowControl->enableBowMotorPower();
    harmonicSeriesHandler->setHarmonic(0);

    uint16_t testPressure = bowControl->getEngagePressure() +
        ((bowControl->getStallPressure() - bowControl->getEngagePressure()) / 4);

    debugPrintln("Setting tilt to " + String(testPressure), debugPrintType::TextInfo);
    bowControl->setHardwarePressure(testPressure);
    debugPrintln("Waiting for Pressure setting to finish.", debugPrintType::TextInfo);
    if (!bowControl->waitForPressureToSettle()) { return CR_Err_BowPressure; }
    debugPrintln("Pause for a little..", debugPrintType::TextInfo);
    delay(2500);

    debugPrintln("Starting fundamental level test, need to reach " + String(minFundamentalAmplitude), debugPrintType::TextInfo);
    // Wait for the bow to settle and then reord the fundamental level
    do {
        testPressure += 100;
        bowControl->setHardwarePressure(testPressure);
        levelFundamental = findLevel();
        debugPrintln("Fundamental level " + String(levelFundamental), debugPrintType::TextInfo);
    } while((levelFundamental < minFundamentalAmplitude) && (testPressure < bowControl->getStallPressure()));

    if (levelFundamental < minFundamentalAmplitude) { return CR_Err_AudioTooLow; }

    debugPrintln("Pickup fundamental ampltiude is " + String(levelFundamental), debugPrintType::TextInfo);

    #define levelStepSize 100

    float level = 0;
    uint16_t mutePos = 0;
    muteControl->setHardwarePosition(mutePos);
    if (!muteControl->completeTask(5000)) { return CR_Err_MutePressure; }

    do {
        muteControl->setHardwarePosition(mutePos);
        if (!muteControl->completeTask(100)) { return CR_Err_MutePressure; }

        mutePos += levelStepSize;
        level = findLevel();

        debugPrintln("Level " + String(level) + " at position " + String(mutePos), debugPrintType::TextInfo);
        if (level < minAmplitude) { break; }
        if (level < levelSilence) { break; }
    } while ((mutePos < stallPosition) && ((level >= minAmplitude) || (level >= levelSilence)));

    if (mutePos == levelStepSize) { return CR_Err_Undefined; }

    muteControl->setFullMutePosition(mutePos);
    muteControl->setHalfMutePosition(mutePos / 2);
    int32_t rest = mutePos - 30000;
    if (rest < 0) { rest = 0; }
    muteControl->setRestPosition(rest);
    debugPrintln("Full mute set at " + String(muteControl->getFullMutePosition()), debugPrintType::TextInfo);

    debugPrintln("Finished level set", debugPrintType::TextInfo);

    bowControl->setRun(false);
    bowControl->setBowRest();
    bowControl->disableBowMotorPower();
    if (!bowControl->waitForPressureToSettle()) { return CR_Err_BowPressure; }

    debugPrintEnabled[debugPrintType::Debug] = debugReport;

    return CR_Ok;
}

eCalibrationResult CalibrateMute::findMuteStall() {
    bool debugReport = debugPrintEnabled[debugPrintType::Debug];
    debugPrintEnabled[debugPrintType::Debug] = false;

    debugPrintln("Starting mute stall calibration", debugPrintType::TextInfo);

    muteControl->home();
    bool autoCorrect = muteControl->getAutoCorrect();
    muteControl->setAutoCorrect(false);

    muteControl->setHardwarePosition(maxTestStep);
    if (!muteControl->completeTask()) { return CR_Err_MutePressure; }
    bool initialHomingSensed = muteControl->getHomingSensed();

    debugPrintln("Starting at max with homing sense " + String(initialHomingSensed), debugPrintType::TextInfo);

    uint16_t i = maxTestStep;
    muteControl->setHardwarePosition(i);
    if (!muteControl->completeTask()) { return CR_Err_MutePressure; }

    do {
        i--;
        muteControl->setHardwarePosition(i);
    if (!muteControl->completeTask()) { return CR_Err_MutePressure; }

        if (i == 0) {
            debugPrintln("Edge detection failed with edge " + String(muteControl->getHomingSensed()), debugPrintType::Error);
            return CR_Err_Undefined;
        }
    } while (muteControl->getHomingSensed() == initialHomingSensed);

    debugPrintln("Homing sense changed to " + String(muteControl->getHomingSensed()) + " direction " + String(muteControl->getMoveDirection()) + " step no " +
                 String(muteControl->getCurrentStep()), debugPrintType::TextInfo);

    eEdgeType edge;
    if (initialHomingSensed == homeSenseInactive) { edge = eEdgeType::EDGERISING; } else { edge =eEdgeType::EDGEFALLING; };
    uint16_t homingPoint = muteControl->getHomingPoint(edge, eStepDirection::REVERSE);
    int32_t homingDifference = muteControl->getCurrentStep() - homingPoint;
    float mult = 65535 / (stepsPerRevolution * microSteps);
    int32_t maxPosition = maxTestStep - (homingDifference * mult) - maxStepRetract;

    stallPosition = maxPosition;

    debugPrintln("Homing point is at " + String(homingPoint) + " difference is " + String(homingDifference) + " making the max position " + String(maxPosition), debugPrintType::TextInfo);

    muteControl->setAutoCorrect(autoCorrect);
    debugPrintEnabled[debugPrintType::Debug] = debugReport;
    return CR_Ok;
}

eCalibrationResult CalibrateMute::calibrateAll(){
    eCalibrationResult result = findMuteStall();
    if (result == CR_Ok) { result = findMuteLevels(); }

    muteControl->home();
    return result;
}

#endif
