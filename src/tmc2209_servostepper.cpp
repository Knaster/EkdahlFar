#ifndef TMC2209_SERVOSTEPPER_C
#define TMC2209_SERVOSTEPPER_C

#include "servostepper.h    pp"
#include "tmc2209_servostepper.hpp"

Tmc2209ServoStepper::Tmc2209ServoStepper(char inStepDirPin, char inStepStepPin, HardwareSerial *inStepSerialPort, char inStepHomeSensorPin) {
    stepSerialStream = inStepSerialPort;
    stepServoStepper = new servoStepper(inStepStepPin, inStepDirPin, inStepHomeSensorPin); //, stepEnPin);
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

bool Tmc2209ServoStepper::setupTMC2209() {
    if (stepTMC2209Driver == nullptr) { return false; }

    stepTMC2209Driver->setup(*stepSerialStream);
    delay(stepConnectDelay);

    stepTMC2209Driver->setRunCurrent(stepRunCurrentPercent);
    stepTMC2209Driver->setMicrostepsPerStep(stepMicrostepping);
    stepTMC2209Driver->disableStealthChop();
    stepTMC2209Driver->setHoldCurrent(10);
    stepTMC2209Driver->enable();
    return true;
}


void Tmc2209ServoStepper::getTMC2209Info() {
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

bool Tmc2209ServoStepper::home(bool invert) {
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
    stepTMC2209Driver->setRunCurrent(stepRunCurrentPercent);
    stepServoStepper->setPosition(0);
    stepServoStepper->completeTask();
    return true;
}

#endif
