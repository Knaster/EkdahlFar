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

class solenoid {
public:

    char solenoidPin;     ///< Pin for solenoid activation

    Teensy_PWM *solenoidPWM;

    solenoid(char _solenoidPin) {
    solenoidPin = _solenoidPin;
    pinMode(solenoidPin, OUTPUT);
    //digitalWrite(solenoidPin, 1);
    solenoidPWM = new Teensy_PWM(solenoidPin, 20000, 0);
    }

    bool solenoidEngaged = false; ///< Current state of the pick solenoid
    unsigned long solenoidEngageTime = 0; ///< Time at which the solenoid was engaged last
    unsigned long solenoidEngageDuration = 15000;  ///< Duration of the solenoid engagement, given in uS

    uint16_t forceMax = 65535;
    uint16_t forceMin = 0;
    float forceMultiplier = 1;

    /// Engage solenoid with the given force
    void solenoidEngage(int force) {
        if (force <= 0) { return; }
        if (force > 65535) { force = 65535; }

        float forceMultiplied = ((float) force) * forceMultiplier;
//        debugPrintln("forceMultiplied " + String(forceMultiplied) + " forceMultiplier " + String(forceMultiplier) + " force " + String(force), Debug);
        if (forceMultiplied == 0) { return; }

        float actualForce = forceMin + ((float) (forceMax - forceMin)) / 65535 * forceMultiplied;

        solenoidEngageTime = micros();

        //    analogWrite(solenoidPin, force);
        analogWrite(solenoidPin, int(actualForce));
        solenoidEngaged = true;
        debugPrintln("Egaging solenoid with force " + String(actualForce), Hardware);
    }

    /// Engage solenoid with maximum force
    void solenoidEngage() {
        solenoidEngage(65535);
    }

    /// Disengage solenoid
    void solenoidDisengage() {
        analogWrite(solenoidPin, 0);
        debugPrintln("Disengaging solenoid", Hardware);
        solenoidEngaged = false;
    }

    void solenoidDisengage(int value) {
        if (value > 0 ) { solenoidDisengage(); }
    }

    bool updateSolenoid() {
        unsigned long currentTime = micros();
        if ((solenoidEngaged) && (currentTime - solenoidEngageTime > solenoidEngageDuration) && (solenoidEngageDuration != 0)) {
            solenoidDisengage();
            return true;
          }
          return false;
    }

    String dumpData() {
        String dump = "";
        dump += "sxf:" + String(forceMax) + ",";
        dump += "sif:" + String(forceMin) + ",";
        return dump;
    }

};

#endif
