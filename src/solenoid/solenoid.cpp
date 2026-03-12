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

#ifdef ARDUINO_TEENSY40
#include "Teensy_PWM.h"
#else
#include "Arduino.h"
#endif

#include "solenoid/solenoid.hpp"

const ModuleCommandDeclaration Solenoid::moduleCommands[] = {
    { "engage", "en", "0-65535", "Engages the solenoid using the 1st argument as hardness", false, false, &s_engage, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectParameter },
    { "rest", "rs", "0-1", "Forces the solenoid to its rest position, an argument of 0 will leave the solenoid at its current state while any other value will disengage the solenoid",
        false, false, &s_disengage, eCommandType_data::ectConditional | eCommandType_function::ectParameter },
    { "maxforce", "xf", "0-65535", "Set solenoid maximum usable force", false, true, &s_maxForce, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectSetting },
    { "minforce", "if", "0-65535", "Set solenoid minimum usable force", false, true, &s_minForce, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectSetting },
    { "forcemultiplier", "fm", "0-65535", "Set solenoid force multiplier", false, true, &s_forceMultiplier, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectParameter },
    { "engageduration", "ed", "uS", "Sets the duration of the solenoid hit in uS, if a value of 0 is set the solenoid will not disengage until a solenoiddisengage command has been given. WARNING!! HAVING THE SOLENOID ON FOR A SPAN OF SECONDS COULD DESTROY THE CIRCUITRY!!",
        false, true, &s_engageDuration, eCommandType_data::ectMicroseconds | eCommandType_function::ectVolatileSetting }
};

getModuleCount(Solenoid)

Solenoid::Solenoid(char _solenoidPin) {
    solenoidPin = _solenoidPin;
    pinMode(solenoidPin, OUTPUT);
}

CREATE_MODULE_COMMAND_FUNCTION(engage, Solenoid) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    solenoidEngage(inCommandItem->argument[0].toInt());
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(disengage, Solenoid) {
    solenoidDisengage();
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(maxForce, Solenoid) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(forceMax), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setSolenoidMax(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(forceMax), InfoRequest});
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(minForce, Solenoid) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(forceMin), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setSolenoidMin(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(forceMin), InfoRequest});
    }
    return eProcessResult::Ok;
}
/*
CREATE_MODULE_COMMAND_FUNCTION(forceMultiplier, Solenoid) {
    if (request) {
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(fForceMultiplier), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setSolenoidMultiplier(inCommandItem->argument[0].toFloat());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(fForceMultiplier), InfoRequest});
    }
    return eProcessResult::Ok;
}
*/

CREATE_MODULE_COMMAND_FUNCTION(forceMultiplier, Solenoid) {
    if (request) {
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(pForceMultiplier), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setSolenoidMultiplier(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(pForceMultiplier), InfoRequest});
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(engageDuration, Solenoid) {
    if (request) {
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(solenoidEngageDuration), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setSolenoidDuration(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(solenoidEngageDuration), InfoRequest});
    }
    return eProcessResult::Ok;
}

/// Engage solenoid with the given force
void Solenoid::solenoidEngage(int force) {
    if (force <= 0) { return; }
    if (force > 65535) { force = 65535; }
/*
    float forceMultiplied = ((float) force) * fForceMultiplier;
    if (forceMultiplied == 0) { return; }

    float actualForce = forceMin + ((float) (forceMax - forceMin)) / 65535 * forceMultiplied;
*/
    //uint32_t multd = ((uint32_t) (forceMax - forceMin) * ((uint32_t) ((force * pForceMultiplier) / 65535)));
    //debugPrintln("Multd " + String(multd), debugPrintType::Debug);
    //uint32_t wlimits = ((uint32_t) (forceMax - forceMin) * multd);
//    debugPrintln("Wlimits " + String(wlimits), debugPrintType::Debug);
    //int32_t actualForce = forceMin + (((forceMax - forceMin) * ((force * pForceMultiplier) / 65535)) / 65535);
    uint32_t actualForce = ((uint32_t) (forceMax - forceMin) * ((uint32_t) ((force * pForceMultiplier) / 65535))) / 65535;
    if (actualForce < 0) { actualForce = 0; }
    if (actualForce > 65535) { actualForce = 65535; }

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
   // debugPrintln("Disengaging solenoid", Hardware);
    solenoidEngaged = false;
}

bool Solenoid::setSolenoidMax(uint16_t inMax) {
    if ((inMax <0) || (inMax > 65535)) { return false; }
    forceMax = inMax;
    return true;
}

bool Solenoid::setSolenoidMin(uint16_t inMin)  {
    if ((inMin <0) || (inMin > 65535)) { return false; }
    forceMin = inMin;
    return true;
}
/*
bool Solenoid::setSolenoidMultiplier(float inMultiplier) {
    if ((inMultiplier < 0) || (inMultiplier > 1)) { return false; }
    fForceMultiplier = inMultiplier;
    return true;
}
*/
bool Solenoid::setSolenoidMultiplier(uint16_t inMultiplier) {
    pForceMultiplier = inMultiplier;
    return true;
}

bool Solenoid::setSolenoidDuration(unsigned long inDuration)  {
    if ((inDuration < 0) || (inDuration > 250000)) { return false; }
    solenoidEngageDuration = inDuration;
    return false;
}

void Solenoid::update() {
    unsigned long currentTime = micros();
    if ((solenoidEngaged) && (currentTime - solenoidEngageTime > solenoidEngageDuration) && (solenoidEngageDuration != 0)) {
        solenoidDisengage();
      }
}

#endif
