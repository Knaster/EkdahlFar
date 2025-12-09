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

BowControl::BowControl(char motorRevPin, char motorVoltagePin, char motorDCDCEnPin, char tachoPin, char currentSensePin, char motorFaultPin,
                       char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {

    dcMotorControl = new DCMotorControl(motorRevPin, motorVoltagePin, motorDCDCEnPin, tachoPin, currentSensePin, motorFaultPin);
    pidController = new PIDController(*dcMotorControl);
    bowPressure = new BowPressure(stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
//    bowActuators = new BowActuators(bowPressure);
}

eProcessResult BowControl::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    eProcessResult processResult = eProcessResult::Ok;

    processCommandItems(inCommandItem, serialCommandsBowControl, sizeof(serialCommandsBowControl)  / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsBowControl, sizeof(serialCommandsBowControl) / sizeof(serialCommandItem), commandResponses,"");
    }

    if (((inCommandItem->command == "bowpressurerest") || (inCommandItem->command == "bpr")) && !request) {
        debugPrintln("Second rest", debugPrintType::Debug);
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() == 1) { rest(); }
    }

    if (inCommandItem->command == "bowmotorrun") {
        if (request) {
            commandResponses->push_back({ "bmr:" + String(run), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (inCommandItem->argument[0].toInt() > 0) {
                run = 1;
                dcMotorControl->enableMotorPower();
            } else {
                run = 0;
                dcMotorControl->disableMotorPower();
            }
            commandResponses->push_back({"bmr:" + String(run), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpid") {
        if (request) {
            commandResponses->push_back({ "bpid:" + String(PIDon), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (inCommandItem->argument[0].toInt() > 0) { PIDon = true; } else { PIDon = false;}
            commandResponses->push_back({"bpid:" + String(PIDon), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowcontrolspeedmode") {
        if (request) {
            commandResponses->push_back({"bcsm:" + String(int(speedMode)), InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (!validateNumber(inCommandItem->argument[0].toInt(), 0, 1)) { return eProcessResult::WrongArgumentValue; }
            speedMode = (eSpeedMode) (inCommandItem->argument[0].toInt());
            commandResponses->push_back({"bcsm:" + String(int(speedMode)), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotordirectpwm") {
        if (request) {
            commandResponses->push_back({ "bmdp:" + String(manualMotorPWM), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            manualMotorPWM = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bmdp:" + String(manualMotorPWM), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotortimeout") {
        if (request) {
            commandResponses->push_back({ "bmt:" + String(bowShutoffTimeout), InfoRequest });
        } else {
            bowShutoffTimeout = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bmt:" + String(bowShutoffTimeout), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowmotorfaultcommands") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            commandsMotorFault = delimitExpression(inCommandItem->argument[0], true);
        }
        commandResponses->push_back({ "bmfc:" + String(commandsMotorFault), InfoRequest });
    } else
    if (inCommandItem->command == "bowmotoroverpowercommands") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            commandsOverPowerCurrent = delimitExpression(inCommandItem->argument[0], true);
        }
        commandResponses->push_back({ "bmopc:" + String(commandsOverPowerCurrent), InfoRequest });
    } else {
        processResult = bowPressure->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = dcMotorControl->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
        if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

        processResult = pidController->processSerialCommand(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    }

    return processResult;
}

eProcessResult BowControl::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    eProcessResult processResult = eProcessResult::NotFound;

    processResult = bowPressure->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

    processResult = dcMotorControl->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }

    processResult = pidController->processSerialCommandHidden(inCommandItem, commandResponses, request, delegate, delegatedCommands);
    if ((processResult != eProcessResult::NotFound) && (processResult != eProcessResult::PassThrough)) { return processResult; }


    return eProcessResult::NotFound;
}
/*
void BowControl::setBowSpeedPWM(uint16_t speed) {
    pidController->setPIDTarget(speed);
}
*/
String BowControl::dumpData() {
    String dump = "";
    dump += "bmt:" + String(bowShutoffTimeout) + ",";
    dump += "bmfc:" + commandsMotorFault + ",";
    dump += "bmopc:" + commandsOverPowerCurrent + ",";
    dump += dcMotorControl->dumpData() + ",";
    dump += bowPressure->dumpData() + ",";
    dump += pidController->dumpData() + ",";
    return dump;
}

void BowControl::rest() {
    bowShutoffTimer = 0;
    bowShutoffTimedout = false;         // added 2024-06-27
    bowShutoffMotorDisabled = false;    // added 2024-06-27
}

void BowControl::updateMotorAutoShutdown() {
    if ((speedMode == eSpeedMode::Automatic) && (bowPressure->getHold() == false)  && (bowShutoffTimer >= bowShutoffTimeout)) {
        if ((bowPressure->getPressureMode() == ePressureMode::Rest) && (bowShutoffTimedout == false)) {
            run = false;
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
    if (run) {
        if (PIDon) {
            if (pidController->getPIDTarget() == 0) { dcMotorControl->setSpeedPWM(0); }
        } else {
            pidController->pidReset();
            dcMotorControl->setSpeedPWM(manualMotorPWM);
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
            commands->addCommands(commandsOverPowerCurrent);
        }
        if (dcMotorControl->getMotorFault()) {
            debugPrintln("Bow motor fault!", debugPrintType::Error);
            commands->addCommands(commandsMotorFault);
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
    if (PIDon && (run == 1) && (pidController->getPIDTarget() > 0)) {
        pidController->pidControl();
    }
}

#endif
