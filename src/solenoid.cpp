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
    }

    /// Engage solenoid with maximum force
    void solenoidEngage() {
        solenoidEngage(255);
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
        if ((solenoidEngaged) && (currentTime - solenoidEngageTime > solenoidEngageDuration)) {
            solenoidDisengage();
            return true;
          }
          return false;
    }

    String dumpData() {
        String dump = "";
        dump += "ssxf:" + String(forceMax) + ",";
        dump += "ssif:" + String(forceMin) + ",";
        return dump;
    }

};

#endif
