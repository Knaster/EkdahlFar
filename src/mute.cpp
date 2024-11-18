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

#include "servostepper.h"
#include "mute.h"

mute::mute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
    stepSerialStream = stepSerialPort;
    stepServoStepper = new servoStepper(stepStepPin, stepDirPin, stepHomeSensorPin); //, stepEnPin);
    stepTMC2209Driver = new TMC2209();
    setupTMC2209();
    stepServoStepper->setHomingOffset(6000);
    debugPrintln("Starting home", Debug);
    stepTMC2209Driver->setRunCurrent(stepHomeCurrentPercent);
    stepServoStepper->home(servoStepper::eStepDirection::FORWARD,5,5,servoStepper::eStepDirection::REVERSE);
    if (stepServoStepper->completeTask(5000)) {
        debugPrintln("Homed", Debug);
    } else {
        debugPrintln("Homing FAILED!", Error);
    }
    stepTMC2209Driver->setRunCurrent(stepRunCurrentPercent);
    stepServoStepper->setPosition(0);
    stepServoStepper->completeTask();
    stepServoStepper->setSpeed(15);
}

bool mute::setupTMC2209() {
    if (stepTMC2209Driver == nullptr) { return false; }

    stepTMC2209Driver->setup(*stepSerialStream);
    delay(stepConnectDelay);

    pinMode(stepEnPin, OUTPUT);
    digitalWrite(stepEnPin, 0);

    stepTMC2209Driver->setRunCurrent(stepRunCurrentPercent);
    stepTMC2209Driver->setMicrostepsPerStep(stepMicrostepping);
    stepTMC2209Driver->disableStealthChop();
    stepTMC2209Driver->setHoldCurrent(10);
    stepTMC2209Driver->enable();
    return true;
}


void mute::getTMC2209Info() {
    debugPrintln("*************************", Debug);
    debugPrintln("getSettings()", Debug);
    TMC2209::Settings settings = stepTMC2209Driver->getSettings();
    debugPrintln("settings.is_communicating = " + String(settings.is_communicating), Debug);
    debugPrintln("settings.is_setup = " + String(settings.is_setup), Debug);
    debugPrintln("settings.software_enabled = " + String(settings.software_enabled), Debug);
    debugPrintln("settings.microsteps_per_step = " + String(settings.microsteps_per_step), Debug);
    debugPrintln("settings.inverse_motor_direction_enabled = " + String(settings.inverse_motor_direction_enabled), Debug);
    debugPrintln("settings.stealth_chop_enabled = " + String(settings.stealth_chop_enabled), Debug);
    debugPrintln("settings.standstill_mode = ", Debug);
    switch (settings.standstill_mode)
    {
    case TMC2209::NORMAL:
        debugPrintln("normal", Debug);
        break;
    case TMC2209::FREEWHEELING:
        debugPrintln("freewheeling", Debug);
        break;
    case TMC2209::STRONG_BRAKING:
        debugPrintln("strong_braking", Debug);
        break;
    case TMC2209::BRAKING:
        debugPrintln("braking", Debug);
        break;
    }
    debugPrintln("settings.irun_percent = " + String(settings.irun_percent), Debug);
    debugPrintln("settings.irun_register_value = " + String(settings.irun_register_value), Debug);
    debugPrintln("settings.ihold_percent = " + String(settings.ihold_percent), Debug);
    debugPrintln("settings.ihold_register_value = " + String(settings.ihold_register_value), Debug);
    debugPrintln("settings.iholddelay_percent = " + String(settings.iholddelay_percent), Debug);
    debugPrintln("settings.iholddelay_register_value = " + String(settings.iholddelay_register_value), Debug);
    debugPrintln("settings.automatic_current_scaling_enabled = " + String(settings.automatic_current_scaling_enabled), Debug);
    debugPrintln("settings.automatic_gradient_adaptation_enabled = " + String(settings.automatic_gradient_adaptation_enabled), Debug);
    debugPrintln("settings.pwm_offset = " + String(settings.pwm_offset), Debug);
    debugPrintln("settings.pwm_gradient = " + String(settings.pwm_gradient), Debug);
    debugPrintln("settings.cool_step_enabled = " + String(settings.cool_step_enabled), Debug);
    debugPrintln("settings.analog_current_scaling_enabled = " + String(settings.analog_current_scaling_enabled), Debug);
    debugPrintln("settings.internal_sense_resistors_enabled = " + String(settings.internal_sense_resistors_enabled), Debug);
    debugPrintln("*************************", Debug);
    debugPrintln("", Debug);

    debugPrintln("*************************", Debug);
    debugPrintln("hardwareDisabled()", Debug);
    bool hardware_disabled = stepTMC2209Driver->hardwareDisabled();
    debugPrint("hardware_disabled = " + String(hardware_disabled), Debug);
    debugPrintln("*************************", Debug);
    debugPrintln("", Debug);

    debugPrintln("*************************", Debug);
    debugPrintln("getStatus()", Debug);
    TMC2209::Status status = stepTMC2209Driver->getStatus();
    debugPrintln("status.over_temperature_warning = " + String(status.over_temperature_warning), Debug);
    debugPrintln("status.over_temperature_shutdown = " + String(status.over_temperature_shutdown), Debug);
    debugPrintln("status.short_to_ground_a = " + String(status.short_to_ground_a), Debug);
    debugPrintln("status.short_to_ground_b = " + String(status.short_to_ground_b), Debug);
    debugPrintln("status.low_side_short_a = " + String(status.low_side_short_a), Debug);
    debugPrintln("status.low_side_short_b = " + String(status.low_side_short_b), Debug);
    debugPrintln("status.open_load_a = " + String(status.open_load_a), Debug);
    debugPrintln("status.open_load_b = " + String(status.open_load_b), Debug);
    debugPrintln("status.over_temperature_120c = " + String(status.over_temperature_120c), Debug);
    debugPrintln("status.over_temperature_143c = " + String(status.over_temperature_143c), Debug);
    debugPrintln("status.over_temperature_150c = " + String(status.over_temperature_150c), Debug);
    debugPrintln("status.over_temperature_157c = " + String(status.over_temperature_157c), Debug);
    debugPrintln("status.current_scaling = " + String(status.current_scaling), Debug);
    debugPrintln("status.stealth_chop_mode = " + String(status.stealth_chop_mode), Debug);
    debugPrintln("status.standstill = " + String(status.standstill), Debug);
    debugPrintln("*************************", Debug);
    debugPrintln("", Debug);
    return;
}

bool mute::setTilt(uint16_t tilt) {
    // Edited 2024-07-28
    //int a = tilt;
//    uint32_t a = ((float) (((float) tilt) / (65535 / fullMutePosition))) ;


//    unsigned int tiltPWM = calibrationDataConnect->firstTouchPressure +
//        ((double)(calibrationDataConnect->stallPressure - calibrationDataConnect->firstTouchPressure)
//        / 65535 * ((double)(baselineTiltPWM + modifierTiltPWM)));

    uint32_t a = restPosition + ((double)(fullMutePosition - restPosition) / 65535 * ((double)tilt));


    if (a < 0) { a = 0; }
    if (a > 65535) { a = 65535; }
    if (a > fullMutePosition) { a = fullMutePosition; }
    debugPrintln("Setting mute tilt to " + String(a) + ", full mute position " + String(fullMutePosition), debugPrintType::Hardware);

//  Edited 2024-07-28
//    if ((lastTilt != a) && (tilt <= muteMaxPosition)) {
//    if ((lastTilt != a) && (tilt <= fullMutePosition)) {
        lastTilt = a;
        mutePosition = emutePosition::mpUndefined;
        hwMutePos = a;
    return true;
}

uint16_t mute::getTilt() {
    return lastTilt;
}

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

bool mute::setRestPosition(uint16_t inRestPosition) {
    restPosition = inRestPosition;
    return true;
}

uint16_t mute::getRestPosition() {
    return restPosition;
}

bool mute::setFullMutePosition(uint16_t inFullMutePosition) {
    fullMutePosition = inFullMutePosition;
    return true;
}

uint16_t mute::getFullMutePosition() {
    return fullMutePosition;
}

bool mute::setHalfMutePosition(uint16_t inHalfMutePosition) {
    halfMutePosition = inHalfMutePosition;
    return true;
}

uint16_t mute::getHalfMutePosition() {
    return halfMutePosition;
}

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
        stepServoStepper->setPosition(hwMutePos);
        hwLastMutePos = hwMutePos;
    }

    if ((backOffTime > 0) && (mutePosition == mpFull) && (backOffTimer > backOffTime) && (!stepServoStepper->isMoving)) {
        debugPrintln("Backing off mute", debugPrintType::Debug);
        rest();
    }
    //stepServoStepper->updatePosition();
}


bool mute::homeMute(bool invert = false) {
    debugPrintln("Starting home", Debug);

    stepServoStepper->setHomingOffset(6000);
    stepTMC2209Driver->setRunCurrent(stepHomeCurrentPercent);
    stepServoStepper->eStop();
    if (!invert) {
        stepServoStepper->home(servoStepper::eStepDirection::FORWARD,5,5,servoStepper::eStepDirection::REVERSE); // ,5,5
    } else {
        stepServoStepper->home(servoStepper::eStepDirection::REVERSE,5,5,servoStepper::eStepDirection::FORWARD); // ,5,5
    }

    if (stepServoStepper->completeTask(5000)) {    //2000
        debugPrintln("Homed", Debug);
    } else {
        debugPrintln("Homing FAILED!", Error);
        return false;
    }
//    return true;
    stepTMC2209Driver->setRunCurrent(stepRunCurrentPercent);
//    stepServoStepper->setSpeed(20);
    stepServoStepper->setPosition(0);
    stepServoStepper->completeTask();
    return true;
}

#endif
