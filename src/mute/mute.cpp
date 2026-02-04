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

#include <base/arduinorequired.hpp>

#include "generic_functions/tmc2209_servostepper.hpp"
#include "mute/mute.hpp"

Mute::Mute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
    tmc2209ServoStepper = new Tmc2209ServoStepper(stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin); }

bool Mute::setupTMC2209() { return tmc2209ServoStepper->setupTMC2209(); }

void Mute::getTMC2209Info() { return tmc2209ServoStepper->getTMC2209Info(); }

bool Mute::homeMute(bool invert) { return tmc2209ServoStepper->home(invert); }

bool Mute::setPosition(uint16_t position) {
    uint32_t a = restPosition + ((double)(fullMutePosition - restPosition) / 65535 * ((double)position));

    if (a < 0) { a = 0; }
    if (a > 65535) { a = 65535; }
    if (a > fullMutePosition) { a = fullMutePosition; }
    debugPrintln("Setting mute tilt to " + String(a) + ", full mute position " + String(fullMutePosition), debugPrintType::Hardware);

    lastPosition = a;
    mutePosition = emutePosition::mpUndefined;
    hwMutePos = a;
    return true;
}

uint16_t Mute::getPosition() { return lastPosition; }

bool Mute::rest() {
    hwMutePos = restPosition;
    mutePosition = mpRest;
    return true;
}

bool Mute::fullMute() {
    mutePosition = mpFull;
    if (sustain) { return true; }
    hwMutePos = fullMutePosition;
    backOffTimer = 0;
    return true;
}

bool Mute::halfMute() {
    hwMutePos = halfMutePosition;
    mutePosition = mpHalf;
    return true;
}

bool Mute::setRestPosition(uint16_t inRestPosition) { restPosition = inRestPosition; return true; }

uint16_t Mute::getRestPosition() { return restPosition; }

bool Mute::setFullMutePosition(uint16_t inFullMutePosition) { fullMutePosition = inFullMutePosition; return true; }

uint16_t Mute::getFullMutePosition() { return fullMutePosition; }

bool Mute::setHalfMutePosition(uint16_t inHalfMutePosition) { halfMutePosition = inHalfMutePosition; return true; }

uint16_t Mute::getHalfMutePosition() { return halfMutePosition; }

bool Mute::setBackOffTime(uint16_t inBackOffTime) { backOffTime = inBackOffTime; return true; };

uint16_t Mute::getBackOffTime() { return backOffTime; };


bool Mute::saveRest() {
    restPosition = lastPosition;
    debugPrintln("Saving mute rest as " + String(restPosition), Debug);
    return true;
}

bool Mute::saveFullMute() {
    fullMutePosition = lastPosition;
    debugPrintln("Saving mute full as " + String(fullMutePosition), Debug);
    return true;
}

bool Mute::saveHalfMute() {
    halfMutePosition = lastPosition;
    debugPrintln("Saving mute half as " + String(halfMutePosition), Debug);
    return true;
}

bool Mute::setSustain(bool inSustain) {
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

bool Mute::getSustain() {
    return sustain;
}

void Mute::updateMute() {
    if (hwMutePos != hwLastMutePos) {
        tmc2209ServoStepper->stepServoStepper->setPosition(hwMutePos);
        hwLastMutePos = hwMutePos;
    }

    if ((backOffTime > 0) && (mutePosition == mpFull) && (backOffTimer > backOffTime) && (!tmc2209ServoStepper->stepServoStepper->getIsMoving())) {
        debugPrintln("Backing off mute", debugPrintType::Debug);
        rest();
    }

    if (tmc2209ServoStepper->stepServoStepper->intDrivenMsgFlag) {
        debugRaw(tmc2209ServoStepper->stepServoStepper->intDrivenMsg);
        tmc2209ServoStepper->stepServoStepper->intDrivenMsgFlag = false;
    }
}

#endif
