#ifndef CONTROLBOXCONTROL_HPP
#define CONTROLBOXCONTROL_HPP

#include "base/module.hpp"
#include "averager.h"

class ControlboxControl : public Module {
public:
    SETMODULEID("controlboxcontrol", "cbc", "Control box control", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(name, ControlboxControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(commands, ControlboxControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(value, ControlboxControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(averages, ControlboxControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(interruptthreshold, ControlboxControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(continuousthreshold, ControlboxControl)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(continuoustimeout, ControlboxControl)

    ControlboxControl(String inName, String inDefaultCommands, bool inTrigger);

    void setDefaults();

    Averager averager;

    String pName;
    String outputCommands;
    uint16_t lastValue;
private:
    String defaultCommands;
};

#endif // BOWACTUATOR_HPP

