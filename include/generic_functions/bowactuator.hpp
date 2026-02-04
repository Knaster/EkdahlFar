#ifndef BOWACTUATOR_HPP
#define BOWACTUATOR_HPP

#include "base/module.hpp"

class BowActuator : public Module {
public:
    SETMODULEID("actuator", "ac", "Actuator data handler", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(name, BowActuator)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(data, BowActuator)

    BowActuator();

    uint16_t firstTouchPressure = 2000;
    uint16_t stallPressure = 65535;
    uint16_t restPosition = 0;
    String id;
};

#endif // BOWACTUATOR_HPP
