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
#ifndef BOWACTUATORS_H
#define BOWACTUATORS_H

#include "base/modulehandler.hpp"
#include "generic_functions/bowactuator.hpp"

class BowActuators : public ModuleHandler
{
public:
    SETMODULEID("actuatorhandler", "ah", "Bow actuator handler", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_INDEX_CALLBACK_FWD(actuatorIndexChanged, BowActuators);

    CREATE_MODULE_COMMAND_FUNCTION_FWD(add, BowActuators)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(remove, BowActuators)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(count, BowActuators)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(clear, BowActuators)

    void dumpData(std::vector<commandResponse> *dataDump) override;

    BowActuators();

private:
/***** Module specific commands mirroring serially attainable commands *****/
    BowActuator* addBowActuator(String name = "default", uint16_t rest = 0, uint16_t engage = 2000, uint16_t stall = 65535);

    bool removeBowActuator(uint8_t t_actuator);


//    uint8_t setBowActuator(uint8_t t_actuator);

//    uint8_t getBowActuator();

//    uint8_t getBowActuatorCount();
/*
    bool loadBowActuator();

    bool loadBowActuator(uint8_t t_actuator);

    bool saveBowActuator(uint16_t actuator, String name);
    */
/*
    bool setBowActuatorFirstTouchPressure(uint16_t t_firstTouchPressure);

    bool setBowActuatorStallPressure(uint16_t t_stallPressure);

    bool setBowActuatorRestPosition(uint16_t t_restPosition);

    bool setBowActuatorID(String t_id);
*/
/*
    uint16_t getBowActuatorFirstTouchPressure(int8_t t_bowActuatorIndex = -1);

    uint16_t getBowActuatorStallPressure(int8_t t_bowActuatorIndex = -1);

    uint16_t getBowActuatorRestPosition(int8_t t_bowActuatorIndex = -1);
    */
/*
    String getBowActuatorID(int8_t t_bowActuatorIndex = -1);

    bool setBowActuatorData(int8_t t_bowActuatorIndex, uint16_t t_firstTouchPressure, uint16_t t_stallPressure, uint16_t t_restPosition, String t_id);*/
};

#endif // BOWACTUATORS_H
