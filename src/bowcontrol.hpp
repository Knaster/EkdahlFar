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

#include "bowpressure.cpp"
#include "pidcontroller.cpp"

/***** NEW CLASS *****/

serialCommandItem serialCommandsBowControl[] = {
    { "bowcontrolspeedmode", "bcsm", "0|1", "Bow motor speed mode, 0 = Automatic and 1 = Manual" },
    { "bowmotortimeout", "bmt", "ms(0-65535)", "Bow motor shutdown timeout after bow having been put into the rest position" },
    { "bowmotordirectpwm", "bmdp", "0-65535", "Bow motor direct power in 16-bit PWM values, requires that the PID is turned off" },
    { "bowpid", "bpid", "1|0", "Sets the bow PID on/off" },
    { "bowmotorrun", "bmr", "1|0", "Set bow motor run on/off" }
};

enum eSpeedMode { Automatic, Manual };

class BowControl {
private:
    DCMotorControl *dcMotorControl = nullptr;

    PIDController *pidController = nullptr;

    BowPressure *bowPressure = nullptr;

public:
#ifndef EXTERNAL_PRESSURE_CONTROLLER
    BowControl(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin,
                       char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin);
#endif // EXTERNAL_PRESSURE_CONTROLLER

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);
    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
    String dumpData();
private:
    uint16_t pidUpdateInterval = 1000;          ///< The interval at which the PID is being called, given in uS

    uint32_t lastUpdate = 0;                    ///< Time of last update() call

    elapsedMillis bowShutoffTimer;

    uint16_t bowShutoffTimeout = 1000;

    bool bowShutoffTimedout = false;

    bool bowShutoffMotorDisabled = false;

    eSpeedMode speedMode = eSpeedMode::Automatic;

    uint16_t manualMotorPWM = 0;

    bool run = false;                          ///< Set to true if the bowing wheel is running

    bool PIDon = true;                        ///< Set to true if the PID is turned on

    String commandsOverPowerCurrent = "";

    String commandsMotorFault = "";

public:
/***** Module specific commands mirroring serially attainable commands *****/
    void setBowSpeedPWM(uint16_t speed);

    uint16_t getBowSpeedPWM() { return dcMotorControl->getSpeedPWM(); }

    void setBowSpeedHZ(float speed) { pidController->setPIDTarget(speed); }

#ifndef EXTERNAL_PRESSURE_CONTROLLER
    bool home() { return bowPressure->home(); }
#endif

    bool enableBowMotorPower() { return dcMotorControl->enableMotorPower(); }

/***** Hidden commands for modular use *****/
    void setCommandsOverPowerCurrent(String inCommands) { commandsOverPowerCurrent = inCommands; }

    String getCommandsOverPowerCurrent() { return commandsOverPowerCurrent; }

    void setCommandsMotorFault(String inCommands) { commandsMotorFault = inCommands; }

    String getCommandsMotorFault() { return commandsMotorFault; }

    bool getBowOverPowerFlag() { return dcMotorControl->isOverPower(); }

    bool getBowMotorFaultFlag() { return dcMotorControl->getMotorFault(); }

#ifndef EXTERNAL_PRESSURE_CONTROLLER
    void getTMC2209Info() { bowPressure->getTMC2209Info(); }
#endif // EXTERNAL_PRESSURE_CONTROLLER

/***** Internal commands for stand-alone and semi-modular use *****/
    // Update function to be periodically called in order for backoff and other things to work
    void update();

    void updatePID();

    void updateTachometer() { dcMotorControl->tachometerISRHandler(); }

    uint16_t getPIDUpdateInterval() { return pidUpdateInterval; }

    void* getBowPressureReference() { return bowPressure; }

#ifndef EXTERNAL_PRESSURE_CONTROLLER
    // Handles time-critical timing of stepper motor update - automatically called through stepInervalCallback - DO NOT MANUALLY CALL THIS
    void updateServo() { bowPressure->updateServo(); }

    void setStepIntervalCallback(void *stepIntervalCallback) { bowPressure->setStepIntervalCallback(stepIntervalCallback); }
#endif // EXTERNAL_PRESSURE_CONTROLLER
/***** Internal commands for debugging use, most likely to be removed *****/
#ifndef EXTERNAL_PRESSURE_CONTROLLER
    void setStepperID(uint16_t stepperID);
#endif // EXTERNAL_PRESSURE_CONTROLLER
private:
    void updateMotorAutoShutdown();
    void updateRun_PID();
    void updateMotorStatus();
};

#endif
