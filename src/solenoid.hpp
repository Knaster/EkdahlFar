#ifndef SOLENOID_HPP
#define SOLENOID_HPP

const serialCommandItem serialCommandsSolenoid[] = {
    { "solenoidengage", "se", "0-65535", "Sets the solenoid engagement of the current bow using the 1st argument as hardness" },
    { "solenoiddisengage", "sd", "0-1", "Sets the solenoid disengagement of the current bow, an argument of '0' will leave the solenoid at its current state while any other value will disengage the solenoid" },
    { "solenoidmaxforce", "sxf", "0 - 65535", "Set solenoid maximum usable force" },
    { "solenoidminforce", "sif", "0 - 65535", "Set solenoid minimum usable force" },
    { "solenoidforcemultiplier", "sfm", "0 - 1", "Set solenoid force multiplier" },
    { "solenoidengageduration", "sed", "uS", "Sets the duration of the solenoid hit in uS, if a value of 0 is set the solenoid will not disengage until a solenoiddisengage command has been given"}
};
/*
const serialCommandItem serialCommandsSolenoidHidden[] = {

}
*/
class Solenoid {
private:
    char solenoidPin;     ///< Pin for solenoid activation
    Teensy_PWM *solenoidPWM;

    uint16_t forceMax = 65535;
    uint16_t forceMin = 0;
    float forceMultiplier = 1;

    bool solenoidEngaged = false; ///< Current state of the pick solenoid
    unsigned long solenoidEngageTime = 0; ///< Time at which the solenoid was engaged last
    unsigned long solenoidEngageDuration = 15000;  ///< Duration of the solenoid engagement, given in uS

public:
    Solenoid(char _solenoidPin);
    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);
    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
    String dumpData();

/***** Module specific commands mirroring serially attainable commands *****/

/***** Hidden commands for modular use *****/
/***** Internal commands for stand-alone and semi-modular use *****/
    bool update();
/***** Internal commands for debugging use, most likely to be removed *****/

private:
    void solenoidEngage(int force);
    void solenoidEngage();
    void solenoidDisengage();

    bool setSolenoidMax(uint16_t inMax);
    bool setSolenoidMin(uint16_t inMin);
    bool setSolenoidMultiplier(float inMultiplier);
    bool setSolenoidDuration(unsigned long inDuration);
};

#endif // SOLENOID_HPP
