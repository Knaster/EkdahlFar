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
#ifndef SOLENOID_C
#define SOLENOID_C

#include "Teensy_PWM.h"
#include "solenoid.hpp"

Solenoid::Solenoid(char _solenoidPin) {
    solenoidPin = _solenoidPin;
    pinMode(solenoidPin, OUTPUT);
    solenoidPWM = new Teensy_PWM(solenoidPin, 20000, 0);
}

eProcessResult Solenoid::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegate, commandList *delegatedCommands) {

    processCommandItems(inCommandItem, serialCommandsSolenoid, sizeof(serialCommandsSolenoid) / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsSolenoid, sizeof(serialCommandsSolenoid) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "solenoidengage") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        solenoidEngage(inCommandItem->argument[0].toInt());
        commandResponses->push_back({"se:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "solenoiddisengage") {
        solenoidDisengage();
        commandResponses->push_back({"sd:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "solenoidengageduration") {
        if (request) {
            commandResponses->push_back({ "sed:" + String(solenoidEngageDuration), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            solenoidEngageDuration = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"sed:" + String(solenoidEngageDuration), InfoRequest});
        }
    } else
    if (inCommandItem->command == "solenoidmaxforce") {
        if (request) {
            commandResponses->push_back({ "sxf:" + String(forceMax), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setSolenoidMax(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"sfx:" + String(forceMax), InfoRequest});
        }
    } else
    if (inCommandItem->command == "solenoidminforce") {
        if (request) {
            commandResponses->push_back({ "sif:" + String(forceMin), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setSolenoidMin(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"sif:" + String(forceMin), InfoRequest});
        }
    } else
    if (inCommandItem->command == "solenoidforcemultiplier") {
        if (request) {
            commandResponses->push_back({ "sfm:" + String(forceMultiplier), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setSolenoidMultiplier(inCommandItem->argument[0].toFloat());
            commandResponses->push_back({"sfm:" + String(forceMultiplier), InfoRequest});
        }
    } else {
        return eProcessResult::NotFound;
    }
    return eProcessResult::Ok;
}

eProcessResult Solenoid::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false, commandList *delegatedCommands = nullptr) {
    return eProcessResult::NotFound;
}


/// Engage solenoid with the given force
void Solenoid::solenoidEngage(int force) {
    if (force <= 0) { return; }
    if (force > 65535) { force = 65535; }

    float forceMultiplied = ((float) force) * forceMultiplier;
    if (forceMultiplied == 0) { return; }

    float actualForce = forceMin + ((float) (forceMax - forceMin)) / 65535 * forceMultiplied;

    solenoidEngageTime = micros();

    analogWrite(solenoidPin, int(actualForce));
    solenoidEngaged = true;
    debugPrintln("Egaging solenoid with force " + String(actualForce), Hardware);
}

/// Engage solenoid with maximum force
void Solenoid::solenoidEngage() {
    solenoidEngage(65535);
}

/// Disengage solenoid
void Solenoid::solenoidDisengage() {
    analogWrite(solenoidPin, 0);
    debugPrintln("Disengaging solenoid", Hardware);
    solenoidEngaged = false;
}

bool Solenoid::setSolenoidMax(uint16_t inMax) {
    if ((inMax <0) || (inMax > 65535)) { return false; }
    forceMax = inMax;
    return true;
}

uint16_t Solenoid::getSolenoidMax() { return forceMax; }

bool Solenoid::setSolenoidMin(uint16_t inMin)  {
    if ((inMin <0) || (inMin > 65535)) { return false; }
    forceMin = inMin;
    return true;
}

uint16_t Solenoid::getSolenoidMin() { return forceMin; }

bool Solenoid::setSolenoidMultiplier(float inMultiplier) {
 {
    if ((inMultiplier < 0) || (inMultiplier > 1)) { return false; }
    forceMultiplier = inMultiplier;
    return true;
}}

float Solenoid::getSolenoidMultiplier() { return forceMultiplier; }

bool Solenoid::setSolenoidDuration(unsigned long inDuration)  {
    if ((inDuration < 0) || (inDuration > 250000)) { return false; }
    solenoidEngageDuration = inDuration;
    return false;
}

unsigned long Solenoid::getSolenoidDuration() { return solenoidEngageDuration; }

bool Solenoid::update() {
    unsigned long currentTime = micros();
    if ((solenoidEngaged) && (currentTime - solenoidEngageTime > solenoidEngageDuration) && (solenoidEngageDuration != 0)) {
        solenoidDisengage();
        return true;
      }
      return false;
}

String Solenoid::dumpData() {
    String dump = "";
    dump += "sxf:" + String(forceMax) + ",";
    dump += "sif:" + String(forceMin) + ",";
    return dump;
}


#endif
