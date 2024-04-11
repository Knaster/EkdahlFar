#ifndef MUTE_C
#define MUTE_C

#include "servostepper.h"
//#include <TMC2209.cpp>

class mute {
private:
    uint16_t lastTilt;

    uint16_t restPosition = 0;
    uint16_t fullMutePosition = 0;
    uint16_t halfMutePosition = 0;

    bool sustain = false;

    uint16_t muteMaxPosition = 59000;

public:
//    uint8_t muteChannel;
//    Adafruit_PWMServoDriver *pwmShield;
//
//    uint16_t servoMin = 65;
//    uint16_t servoMax = 370;
//
//    uint16_t EEPROM_offset = 0;

    uint16_t hwMutePos = 0;
    uint16_t hwLastMutePos = 0;

    enum emutePosition {mpUndefined, mpRest, mpFull, mpHalf };
    emutePosition mutePosition = mpUndefined;

    servoStepper *stepServoStepper = nullptr;
private:
    HardwareSerial *stepSerialStream;       ///< Serial port for tilt stepper
    const long stepBaudRate = 115200;       ///< Baud rate for tilt stepper serial
    const int stepConnectDelay = 200;       ///< Delay in between connecting to stepper and issuing commandss
    const uint8_t stepRunCurrentPercent = 30;   ///< Stepper current during run phase
    const uint8_t stepHomeCurrentPercent = 10;
    const char stepMicrostepping = 32;      ///< Tilt stepper micro stepping
    char stepEnPin;                         ///< Pin for tilt stepper driver enable
    char stepperDirPin;                     ///< Pin for tilt stepper driver direction
    char stepStepPin;                       ///< Pin for tilt stepper driver step
    TMC2209 *stepTMC2209Driver = nullptr;   ///< Tilt stepper driver class pointer

public:
    mute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
        stepSerialStream = stepSerialPort;
        stepServoStepper = new servoStepper(stepStepPin, stepDirPin, stepHomeSensorPin); //, stepEnPin);
        stepTMC2209Driver = new TMC2209();
        setupTMC2209();
        stepServoStepper->setHomingOffset(6000);
//        stepServoStepper->setHomingOffset(22500);
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

    bool setupTMC2209() {
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


    void getTMC2209Info() {
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

    bool setTilt(uint16_t tilt) {
/*        int a = servoMin + (((float) (servoMax - servoMin) * (65535 - tilt) / 65535));
        if (lastTilt != a) {
            lastTilt = a;
            //elapsedMicros pop;
            //pwmShield->setPWM(muteChannel, 0, a);
            hwMutePos = a;
            // debugPrintln("Took " + String(pop) + " us", Debug);
        }*/
        int a = tilt;
        if ((lastTilt != a) && (tilt <= muteMaxPosition)) {
            lastTilt = a;
            mutePosition = emutePosition::mpUndefined;
            hwMutePos = a;
        }
        return true;
    }

    uint16_t getTilt() {
        return lastTilt;
    }

    bool rest() {
        //debugPrintln("Mute rest", Hardware);
        //    pwmShield->setPWM(muteChannel, 0, restPosition);
        hwMutePos = restPosition;
        mutePosition = mpRest;
        return true;
    }

    bool fullMute() {
        mutePosition = mpFull;
        if (sustain) { return true; }
        //debugPrintln("Mute full mute", Hardware);
        //    pwmShield->setPWM(muteChannel, 0, fullMutePosition);
        hwMutePos = fullMutePosition;
        return true;
    }

    bool halfMute() {
        //debugPrintln("Mute half mute", Hardware);
        //    pwmShield->setPWM(muteChannel, 0, halfMutePosition);
        hwMutePos = halfMutePosition;
        mutePosition = mpHalf;
        return true;
    }

    bool setRestPosition(uint16_t inRestPosition) {
        restPosition = inRestPosition;
        return true;
    }

    uint16_t getRestPosition() {
        return restPosition;
    }

    bool setFullMutePosition(uint16_t inFullMutePosition) {
        fullMutePosition = inFullMutePosition;
        return true;
    }

    uint16_t getFullMutePosition() {
        return fullMutePosition;
    }

    bool setHalfMutePosition(uint16_t inHalfMutePosition) {
        halfMutePosition = inHalfMutePosition;
        return true;
    }

    uint16_t getHalfMutePosition() {
        return halfMutePosition;
    }

    bool saveRest() {
        restPosition = lastTilt;
        debugPrintln("Saving mute rest as " + String(restPosition), Debug);
        return true;
    }

    bool saveFullMute() {
        fullMutePosition = lastTilt;
        debugPrintln("Saving mute full as " + String(fullMutePosition), Debug);
        return true;
    }

    bool saveHalfMute() {
        halfMutePosition = lastTilt;
        debugPrintln("Saving mute half as " + String(halfMutePosition), Debug);
        return true;
    }

    bool setSustain(bool inSustain) {
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

    bool getSustain() {
        return sustain;
    }

/*
    uint16_t saveParams() {
        eeprom_write_word(EEPROM_offset, restPosition);
        eeprom_write_word(EEPROM_offset + 2, fullMutePosition);
        eeprom_write_word(EEPROM_offset + 4, halfMutePosition);

        debugPrintln("Saving parameters starting att offset " + String(EEPROM_offset) + " mute rest " + String(restPosition) + " full " + String(fullMutePosition) + " half " + String(halfMutePosition), Debug);
        return 6;
    }

    uint16_t loadParams() {
        restPosition = eeprom_read_word(EEPROM_offset);
        fullMutePosition = eeprom_read_word(EEPROM_offset + 2);
        halfMutePosition = eeprom_read_word(EEPROM_offset + 4);

        debugPrintln("Loading parameters starting att offset " + String(EEPROM_offset) + " mute rest " + String(restPosition) + " full " + String(fullMutePosition) + " half " + String(halfMutePosition), Debug);
        return 6;
    }
*/
    String dumpData() {
        String dump = "";
        dump += "mfmp:" + String(fullMutePosition) + ",";
        dump += "mhmp:" + String(halfMutePosition) + ",";
        dump += "mrp:" + String(restPosition) + ",";
        return dump;
    }

    void updateMute() {
#ifdef EFARMASTER
  pwmShield->setPWM(muteChannel, 0, hwMutePos);
#endif
        if (hwMutePos != hwLastMutePos) {
            stepServoStepper->setPosition(hwMutePos);
            hwLastMutePos = hwMutePos;
        }
        stepServoStepper->updatePosition();
    }
};

#endif
