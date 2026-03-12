#ifndef SOLENOID_HPP
#define SOLENOID_HPP

#include <base/module.hpp>

#ifdef ARDUINO_TEENSY40
#include <Teensy_PWM.h>
#endif // ARDUINO_TEENSY40

class Solenoid : public Module {
public:
    SETMODULEID("solenoid", "so", "Solenoid controller v1.0", eModuleType::hardware, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(engage, Solenoid)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(disengage, Solenoid)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(maxForce, Solenoid)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(minForce, Solenoid)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(forceMultiplier, Solenoid)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(engageDuration, Solenoid)

    Solenoid(char _solenoidPin);

    void update();

private:
    char solenoidPin;     ///< Pin for solenoid activation
    //Teensy_PWM *solenoidPWM;

    uint16_t forceMax = 65535;
    uint16_t forceMin = 0;
    //float fForceMultiplier = 1;
    uint16_t pForceMultiplier = 65535;

    bool solenoidEngaged = false; ///< Current state of the pick solenoid
    unsigned long solenoidEngageTime = 0; ///< Time at which the solenoid was engaged last
    unsigned long solenoidEngageDuration = 15000;  ///< Duration of the solenoid engagement, given in uS

    void solenoidEngage(int force);
    void solenoidEngage();
    void solenoidDisengage();

    bool setSolenoidMax(uint16_t inMax);
    bool setSolenoidMin(uint16_t inMin);
    //bool setSolenoidMultiplier(float inMultiplier);
    bool setSolenoidMultiplier(uint16_t inMultiplier);
    bool setSolenoidDuration(unsigned long inDuration);
};

#endif // SOLENOID_HPP
