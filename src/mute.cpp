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
    debugPrintln("*************************", InfoRequest);
    debugPrintln("getSettings()", InfoRequest);
    TMC2209::Settings settings = stepTMC2209Driver->getSettings();
    debugPrintln("settings.is_communicating = " + String(settings.is_communicating), InfoRequest);
    debugPrintln("settings.is_setup = " + String(settings.is_setup), InfoRequest);
    debugPrintln("settings.software_enabled = " + String(settings.software_enabled), InfoRequest);
    debugPrintln("settings.microsteps_per_step = " + String(settings.microsteps_per_step), InfoRequest);
    debugPrintln("settings.inverse_motor_direction_enabled = " + String(settings.inverse_motor_direction_enabled), InfoRequest);
    debugPrintln("settings.stealth_chop_enabled = " + String(settings.stealth_chop_enabled), InfoRequest);
    debugPrintln("settings.standstill_mode = ", InfoRequest);
    switch (settings.standstill_mode)
    {
    case TMC2209::NORMAL:
        debugPrintln("normal", InfoRequest);
        break;
    case TMC2209::FREEWHEELING:
        debugPrintln("freewheeling", InfoRequest);
        break;
    case TMC2209::STRONG_BRAKING:
        debugPrintln("strong_braking", InfoRequest);
        break;
    case TMC2209::BRAKING:
        debugPrintln("braking", InfoRequest);
        break;
    }
    debugPrintln("settings.irun_percent = " + String(settings.irun_percent), InfoRequest);
    debugPrintln("settings.irun_register_value = " + String(settings.irun_register_value), InfoRequest);
    debugPrintln("settings.ihold_percent = " + String(settings.ihold_percent), InfoRequest);
    debugPrintln("settings.ihold_register_value = " + String(settings.ihold_register_value), InfoRequest);
    debugPrintln("settings.iholddelay_percent = " + String(settings.iholddelay_percent), InfoRequest);
    debugPrintln("settings.iholddelay_register_value = " + String(settings.iholddelay_register_value), InfoRequest);
    debugPrintln("settings.automatic_current_scaling_enabled = " + String(settings.automatic_current_scaling_enabled), InfoRequest);
    debugPrintln("settings.automatic_gradient_adaptation_enabled = " + String(settings.automatic_gradient_adaptation_enabled), InfoRequest);
    debugPrintln("settings.pwm_offset = " + String(settings.pwm_offset), InfoRequest);
    debugPrintln("settings.pwm_gradient = " + String(settings.pwm_gradient), InfoRequest);
    debugPrintln("settings.cool_step_enabled = " + String(settings.cool_step_enabled), InfoRequest);
    debugPrintln("settings.analog_current_scaling_enabled = " + String(settings.analog_current_scaling_enabled), InfoRequest);
    debugPrintln("settings.internal_sense_resistors_enabled = " + String(settings.internal_sense_resistors_enabled), InfoRequest);
    debugPrintln("*************************", InfoRequest);
    debugPrintln("", InfoRequest);

    debugPrintln("*************************", InfoRequest);
    debugPrintln("hardwareDisabled()", InfoRequest);
    bool hardware_disabled = stepTMC2209Driver->hardwareDisabled();
    debugPrint("hardware_disabled = " + String(hardware_disabled), InfoRequest);
    debugPrintln("*************************", InfoRequest);
    debugPrintln("", InfoRequest);

    debugPrintln("*************************", InfoRequest);
    debugPrintln("getStatus()", InfoRequest);
    TMC2209::Status status = stepTMC2209Driver->getStatus();
    debugPrintln("status.over_temperature_warning = " + String(status.over_temperature_warning), InfoRequest);
    debugPrintln("status.over_temperature_shutdown = " + String(status.over_temperature_shutdown), InfoRequest);
    debugPrintln("status.short_to_ground_a = " + String(status.short_to_ground_a), InfoRequest);
    debugPrintln("status.short_to_ground_b = " + String(status.short_to_ground_b), InfoRequest);
    debugPrintln("status.low_side_short_a = " + String(status.low_side_short_a), InfoRequest);
    debugPrintln("status.low_side_short_b = " + String(status.low_side_short_b), InfoRequest);
    debugPrintln("status.open_load_a = " + String(status.open_load_a), InfoRequest);
    debugPrintln("status.open_load_b = " + String(status.open_load_b), InfoRequest);
    debugPrintln("status.over_temperature_120c = " + String(status.over_temperature_120c), InfoRequest);
    debugPrintln("status.over_temperature_143c = " + String(status.over_temperature_143c), InfoRequest);
    debugPrintln("status.over_temperature_150c = " + String(status.over_temperature_150c), InfoRequest);
    debugPrintln("status.over_temperature_157c = " + String(status.over_temperature_157c), InfoRequest);
    debugPrintln("status.current_scaling = " + String(status.current_scaling), InfoRequest);
    debugPrintln("status.stealth_chop_mode = " + String(status.stealth_chop_mode), InfoRequest);
    debugPrintln("status.standstill = " + String(status.standstill), InfoRequest);
    debugPrintln("*************************", InfoRequest);
    debugPrintln("", InfoRequest);
    return;
}

bool mute::setTilt(uint16_t tilt) {
    int a = tilt;
    if ((lastTilt != a) && (tilt <= muteMaxPosition)) {
        lastTilt = a;
        mutePosition = emutePosition::mpUndefined;
        hwMutePos = a;
    }
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

#endif
