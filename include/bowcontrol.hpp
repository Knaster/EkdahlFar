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
#ifndef BOWCONTROL_H
#define BOWCONTROL_H

#include "../src/bowActuators.cpp"
#include "../src/bowpressure.cpp"
#include "../src/pidcontroller.cpp"
#include "../src/harmonicserieshandler.cpp"


/***** NEW CLASS *****/

enum eSpeedMode { Automatic, Manual };

class BowControl : public ModuleHandler {
public:
    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(speedMode, BowControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(motorTimeout, BowControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(pidEnable, BowControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(motorFaultCommands, BowControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(motorOverPowerCommands, BowControl)

private:
    DCMotorControl *dcMotorControl = nullptr;
    PIDController *pidController = nullptr;
    BowPressure *bowPressure = nullptr;

public:
    HarmonicSeriesHandler *harmonicSeriesHandler = nullptr;

    const String prefix[2] = { "bow", "b" };
    const String identifier = "Bowing wheel controller v1.0";

#ifndef EXTERNAL_PRESSURE_CONTROLLER
    BowControl(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin,
                       char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin);
#endif // EXTERNAL_PRESSURE_CONTROLLER

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);
    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
private:
    uint16_t pidUpdateInterval = 1000;          ///< The interval at which the PID is being called, given in uS

    uint32_t lastUpdate = 0;                    ///< Time of last update() call

    elapsedMillis bowShutoffTimer;

    uint16_t bowShutoffTimeout = 1000;

    bool bowShutoffTimedout = false;

    bool bowShutoffMotorDisabled = false;

    eSpeedMode pSpeedMode = eSpeedMode::Automatic;

//    uint16_t manualMotorPWM = 0;

    //bool run = false;                          ///< Set to true if the bowing wheel is running

    bool PIDon = true;                        ///< Set to true if the PID is turned on

    String commandsOverPowerCurrent = "";

    String commandsMotorFault = "";

public:
/***** Module specific commands mirroring serially attainable commands *****/
    //void setBowSpeedPWM(uint16_t speed);

    //uint16_t getBowSpeedPWM() { return dcMotorControl->getSpeedPWM(); }

    void setBowMotorDirectPWM(uint16_t speed) { dcMotorControl->setSpeedPWMSafe(speed); }

    void setBowSpeedHZ(float speed) { pidController->setPIDTarget(speed); }

#ifndef EXTERNAL_PRESSURE_CONTROLLER
    bool home() { return bowPressure->home(); }
#endif

    bool enableBowMotorPower() { return dcMotorControl->enableMotorPower(); }

    bool disableBowMotorPower() { return dcMotorControl->disableMotorPower(); }

/***** Hidden commands for modular use *****/
    String getCommandsOverPowerCurrent() { return commandsOverPowerCurrent; }

    String getCommandsMotorFault() { return commandsMotorFault; }

    bool getBowOverPowerFlag() { return dcMotorControl->isOverPower(); }

    bool getBowMotorFaultFlag() { return dcMotorControl->getMotorFault(); }

    float getAverageTachometerFreq() { return dcMotorControl->getAverageTachometerFreq(); }

    bool checkTachometerTimeout() { return dcMotorControl->checkTachometerTimeout(); }

    void setHardwarePressure(uint16_t speed) { bowPressure->setHardwarePressure(speed);}

    bool waitForPressureToSettle(uint16_t timeout = 3000) { return bowPressure->completeTask(timeout); }

    void setEngagePressure(uint16_t pressure) { bowPressure->setEngagePressure(pressure); }

    void setRestPressure(uint16_t pressure) { bowPressure->setRestPressure(pressure); }

    void setStallPressure(uint16_t pressure) { bowPressure->setStallPressure(pressure); }

    uint16_t getEngagePressure() { return bowPressure->getEngagePressure(); }

    uint16_t getRestPressure() { return bowPressure->getRestPressure(); }

    uint16_t getStallPressure() { return bowPressure->getStallPressure(); }

    bool bowMotorIsOverCurrent() { return dcMotorControl->isOverCurrent(); }

    bool bowMotorIsOverPower() { return dcMotorControl->isOverPower(); }

    bool getPressureAutocorrect() { return bowPressure->getAutoCorrect(); }

    void setPressureAutocorrect(bool correct) { bowPressure->setAutoCorrect(correct); }

    bool getPIDOn() { return PIDon; }

    void setPIDOn(bool inPIDon) { PIDon = inPIDon;}

    void setPressureMotorSpeed(float speed) { bowPressure->setSpeed(speed); }

    float getPressureMotorSpeed() { return bowPressure->getSpeed(); }

    void setRun(bool inRun) { dcMotorControl->setBowMotorRun(inRun); }

    bool getRun() { return dcMotorControl->getBowMotorRun(); }

    uint16_t getBowMotorMaxPower() { return dcMotorControl->getMotorMaxPower(); }

    void setBowMotorMaxPower(uint16_t power) { dcMotorControl->setMotorMaxPower(power); }

    uint16_t getBowMotorMinPWM() { return dcMotorControl->getMinSpeedPWM(); }

    void setBowMotorMinPWM(uint16_t pwm) { dcMotorControl->setMinSpeedPWM(pwm); }

    uint16_t getBowMotorMaxPWM() { return dcMotorControl->getMaxSpeedPWM(); }

    void setBowMotorMaxPWM(uint16_t pwm) { dcMotorControl->setMaxSpeedPWM(pwm); }

    uint16_t getBowMotorMaxHz() { return pidController->getMaxSpeedHz(); }

    void setBowMotorMaxHz(uint16_t frequency) { pidController->setMaxSpeedHz(frequency); }

    uint16_t getBowMotorMinHz() { return pidController->getMinSpeedHz(); }

    void setBowMotorMinHz(uint16_t frequency) { pidController->setMinSpeedHz(frequency); }

    float getBowMotorPower() { return dcMotorControl->getMotorCurrent() * dcMotorControl->getMotorVoltage(); }

    void setBowRest() { bowPressure->rest(true); }

    void setBowEngage() { bowPressure->engage(true); }

    void setSpeedMode(eSpeedMode inSpeedMode) { pSpeedMode = inSpeedMode; }

    void rest();
#ifndef EXTERNAL_PRESSURE_CONTROLLER
    void getTMC2209Info() { bowPressure->getTMC2209Info(); }
#endif // EXTERNAL_PRESSURE_CONTROLLER

/***** Internal commands for stand-alone and semi-modular use *****/
    // Update function to be periodically called in order for backoff and other things to work
    void update();

    void updatePID();

    void updateTachometer() { dcMotorControl->tachometerISRHandler(); }

    uint16_t getPIDUpdateInterval() { return pidUpdateInterval; }

    BowPressure* getBowPressureReference() { return bowPressure; }

#ifndef EXTERNAL_PRESSURE_CONTROLLER
    // Handles time-critical timing of stepper motor update - automatically called through stepInervalCallback - DO NOT MANUALLY CALL THIS
    void updateServo() { bowPressure->updateServo(); }

    void setStepIntervalCallback(void *stepIntervalCallback) { bowPressure->setStepIntervalCallback(stepIntervalCallback); }
#endif // EXTERNAL_PRESSURE_CONTROLLER
/***** Internal commands for debugging use, most likely to be removed *****/
#ifndef EXTERNAL_PRESSURE_CONTROLLER
//    void setStepperID(uint16_t stepperID);
#endif // EXTERNAL_PRESSURE_CONTROLLER
private:
    void updateMotorAutoShutdown();
    void updateRun_PID();
    void updateMotorStatus();
};

#endif
