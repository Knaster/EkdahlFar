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
#ifndef BOWCONTROL_C
#define BOWCONTROL_C

#include "bowcontrol.hpp"

/**
 * @file bowcontrol.cpp
 *
 * High level bow control functions
 *
 * Need to have different modes for automatic or manual pressure and speed
 *
 * Current behavior:
 * if run
 *    if pid ON
 *      do nothing as the interrupt driven PID function will set speed to pidTargetSpeed
 *    else
 *      reset PID
 *      setTilt manualTiltPWM
 * else
 *    reset PID
 *    set bow speed to 0
 *
 * tilt is set immediately by either setManualTilt or setPressureBaseline
 * \todo THIS NEEDS TO CHANGE, values should be updated internally but only outputted depending on manual / automatic control
 */

/***** NEW CLASS *****/

const ModuleCommandDeclaration BowControl::moduleCommands[] = {
    { "speedmode", "sm", "0|1", "Bow motor speed mode, 0 = Automatic and 1 = Manual", false, false, &s_speedMode },
    { "motortimeout", "mt", "ms(0-65535)", "Bow motor shutdown timeout after bow having been put into the rest position", false, true, &s_motorTimeout },
    { "pidenable", "pe", "1|0", "Sets the bow PID on/off", false, false, &s_pidEnable },
    { "motorfaultcommands", "mfc", "command list", "Commands to execute when a motor fault is tripped - !WARNING! Can ruin your instrument if changed", false, true, &s_motorFaultCommands },
    { "motoroverpowercommands", "moc", "command list", "Commands to execute when motor is over the power limit - !WARNING! Can ruin your instrument if changed", false, true, &s_motorOverPowerCommands },
};

getModuleCount(BowControl)

BowControl::BowControl(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin,
                       char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {

    moduleID = new ModuleID("bowingwheel", "bw", "Bowing wheel controller v1.0", eModuleType::hardware);

    dcMotorControl = new DCMotorControl(motorRevPin, motorVoltagePin, motorDCDCEnPin, tachoPin, currentSensePin, motorFaultPin);
    pidController = new PIDController(*dcMotorControl);
    bowPressure = new BowPressure(stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);

    commandsMotorFault = "bw.dcm.ru:0,bw.bp.rs:1";
    commandsOverPowerCurrent = "bw.dcm.ru:0,bw.bp.rs:1,bw.dcm.es:1000";

    harmonicSeriesHandler = new HarmonicSeriesHandler();

    addModule(dcMotorControl);
    addModule(pidController);
    addModule(bowPressure);

    addModule(harmonicSeriesHandler);
}

CREATE_MODULE_COMMAND_FUNCTION(speedMode, BowControl) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (!validateNumber(inCommandItem->argument[0].toInt(), 0, 1)) { return eProcessResult::WrongArgumentValue; }
        pSpeedMode = (eSpeedMode) (inCommandItem->argument[0].toInt());
    }
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(int(pSpeedMode)), InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(motorTimeout, BowControl) {
    if (!request) {
        bowShutoffTimeout = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(bowShutoffTimeout), InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(pidEnable, BowControl) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() > 0) { PIDon = true; } else { PIDon = false;}
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(PIDon), InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(motorFaultCommands, BowControl) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        commandsMotorFault = stripQuotes(inCommandItem->argument[0]);
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(commandsMotorFault, true), InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(motorOverPowerCommands, BowControl) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        commandsOverPowerCurrent = stripQuotes(inCommandItem->argument[0]);
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(commandsOverPowerCurrent, true), InfoRequest });
    return eProcessResult::Ok;
};

void BowControl::rest() {
    bowShutoffTimer = 0;
    bowShutoffTimedout = false;         // added 2024-06-27
    bowShutoffMotorDisabled = false;    // added 2024-06-27
    debugPrintln("rest signal ", debugPrintType::Debug);
}

void BowControl::updateMotorAutoShutdown() {
    if (((BowPressure*) getGroup("bowpressure")->modules[0])->getRestSignal()) { rest(); }

    if ((pSpeedMode == eSpeedMode::Automatic) && (bowPressure->getHold() == false)  && (bowShutoffTimer >= bowShutoffTimeout)) {
        if ((bowPressure->getPressureMode() == ePressureMode::Rest) && (bowShutoffTimedout == false)) {
            dcMotorControl->setBowMotorRun(false);
            bowShutoffTimedout = true;
            debugPrintln("Auto shutdown of motor", debugPrintType::Debug);
        } else
        if ((bowShutoffTimedout == true) && (bowShutoffMotorDisabled == false) && (dcMotorControl->getAverageTachometerFreq() == 0)) {
            dcMotorControl->disableMotorPower();
            bowShutoffMotorDisabled = true;
            debugPrintln("Auto shutdown of motor dc/dc converter", debugPrintType::Debug);
        }
    }
}

void BowControl::updateRun_PID() {
    //if (run) {
    if (dcMotorControl->getBowMotorRun()) {
        if (PIDon) {
            if (pidController->getPIDTarget() == 0) { dcMotorControl->setSpeedPWM(0); }
        } else {
            pidController->pidReset();
            //dcMotorControl->setSpeedPWM(manualMotorPWM);
        }
    }  else {
        pidController->pidReset();
        dcMotorControl->setSpeedPWM(0);
    }
}

void BowControl::updateMotorStatus() {
    if (dcMotorControl->getSpeedPWM() != 0) {
        if (dcMotorControl->isOverPower() || dcMotorControl->isOverCurrent()) {
            debugPrintln("Bow over power!", debugPrintType::Error);
            globalResponseCommands.addCommands(commandsOverPowerCurrent);
        }
        if (dcMotorControl->getMotorFault()) {
            debugPrintln("Bow motor fault!", debugPrintType::Error);
            globalResponseCommands.addCommands(commandsMotorFault);
        }
    }
}

void BowControl::update() {
    uint32_t currentTime = micros();

    if (currentTime - lastUpdate >= pidUpdateInterval) {
        lastUpdate = currentTime;

        updateMotorAutoShutdown();
        updateRun_PID();
        updateMotorStatus();
        bowPressure->update();
        // Check if the bowing wheel speed is zero
        dcMotorControl->checkTachometerTimeout();
    }

    bowPressure->update();
}

void BowControl::updatePID() {
    if (PIDon && (dcMotorControl->getBowMotorRun() == 1) && (pidController->getPIDTarget() > 0)) {
//    if (PIDon && (run == 1) && (pidController->getPIDTarget() > 0)) {
        pidController->pidControl();
    }
}

#endif
