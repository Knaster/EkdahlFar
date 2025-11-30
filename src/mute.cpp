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
#ifndef MUTE_C
#define MUTE_C

#include "tmc2209_servostepper.cpp"
#include "mute.hpp"

mute::mute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
    tmc2209ServoStepper = new Tmc2209ServoStepper(stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin); }

bool mute::setupTMC2209() { return tmc2209ServoStepper->setupTMC2209(); }

void mute::getTMC2209Info() { return tmc2209ServoStepper->getTMC2209Info(); }

bool mute::homeMute(bool invert) { return tmc2209ServoStepper->home(invert); }

bool mute::setTilt(uint16_t tilt) {
    uint32_t a = restPosition + ((double)(fullMutePosition - restPosition) / 65535 * ((double)tilt));

    if (a < 0) { a = 0; }
    if (a > 65535) { a = 65535; }
    if (a > fullMutePosition) { a = fullMutePosition; }
    debugPrintln("Setting mute tilt to " + String(a) + ", full mute position " + String(fullMutePosition), debugPrintType::Hardware);

    lastTilt = a;
    mutePosition = emutePosition::mpUndefined;
    hwMutePos = a;
    return true;
}

uint16_t mute::getTilt() { return lastTilt; }

bool mute::rest() {
    hwMutePos = restPosition;
    mutePosition = mpRest;
    return true;
}

bool mute::fullMute() {
    mutePosition = mpFull;
    if (sustain) { return true; }
    hwMutePos = fullMutePosition;
    backOffTimer = 0;
    return true;
}

bool mute::halfMute() {
    hwMutePos = halfMutePosition;
    mutePosition = mpHalf;
    return true;
}

bool mute::setRestPosition(uint16_t inRestPosition) { restPosition = inRestPosition; return true; }

uint16_t mute::getRestPosition() { return restPosition; }

bool mute::setFullMutePosition(uint16_t inFullMutePosition) { fullMutePosition = inFullMutePosition; return true; }

uint16_t mute::getFullMutePosition() { return fullMutePosition; }

bool mute::setHalfMutePosition(uint16_t inHalfMutePosition) { halfMutePosition = inHalfMutePosition; return true; }

uint16_t mute::getHalfMutePosition() { return halfMutePosition; }

bool mute::saveRest() {
    restPosition = lastTilt;
    debugPrintln("Saving mute rest as " + String(restPosition), Debug);
    return true;
}

bool mute::saveFullMute() {
    fullMutePosition = lastTilt;
    debugPrintln("Saving mute full as " + String(fullMutePosition), Debug);
    return true;
}

bool mute::saveHalfMute() {
    halfMutePosition = lastTilt;
    debugPrintln("Saving mute half as " + String(halfMutePosition), Debug);
    return true;
}

bool mute::setSustain(bool inSustain) {
    // If sustain is changed after keys are released, update mute position
    if ((inSustain == false) && (sustain == true)) {
        if (mutePosition == emutePosition::mpFull) {
            hwMutePos = fullMutePosition;
        }
    }

    if ((inSustain == true) && (sustain == false)) {
        hwMutePos = restPosition;
    }

    sustain = inSustain;
    return true;
}

bool mute::getSustain() {
    return sustain;
}

String mute::dumpData() {
    String dump = "";
    dump += "mfmp:" + String(fullMutePosition) + ",";
    dump += "mhmp:" + String(halfMutePosition) + ",";
    dump += "mrp:" + String(restPosition) + ",";
    dump += "mbo:" + String(backOffTime) + ",";
    return dump;
}

void mute::updateMute() {
    if (hwMutePos != hwLastMutePos) {
        tmc2209ServoStepper->stepServoStepper->setPosition(hwMutePos);
        hwLastMutePos = hwMutePos;
    }

    if ((backOffTime > 0) && (mutePosition == mpFull) && (backOffTimer > backOffTime) && (!tmc2209ServoStepper->stepServoStepper->getIsMoving())) {
        debugPrintln("Backing off mute", debugPrintType::Debug);
        rest();
    }
}

#endif
