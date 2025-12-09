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

/*
    Conforming to STD C++ naming conventions with the exception of where external non-conforming items are referenced
*/

const serialCommandItem serialCommandsBowActuators[] = {
    { "bowactuator", "ba", "actuator", "Sets and loads the actuator given"},
    { "bowactuatorremove", "bar", "actuator", "Remove bow actuator"},
    { "bowactuatorsave", "bas", "-", "Save current bow parameters into currently selected bow actuator" },
    { "bowactuatordata", "bad", "actuator:engagepressure:stallpressure:restposition:id", "Set data of bow actuator [actuator]" },
    { "bowactuatorcount", "bac", "-", "Returns the amount of saved bow actuators"}
};

struct BowActuator {
    uint16_t firstTouchPressure = 0;
    uint16_t stallPressure = 65535;
    uint16_t restPosition = 0;
    String id;
};

class BowActuators
{
public:
//    BowActuators(CalibrationData *t_calibrationDataConnect);
//    virtual ~BowActuators();
    BowActuators(BowPressure *inBowPressure);

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);

    String dumpData();
private:
//    CalibrationData *m_calibrationDataConnect;
    BowPressure *bowPressure;

    uint8_t m_currentBowActuator = 0;

    std::vector<BowActuator> m_bowActuator;

private:
/***** Module specific commands mirroring serially attainable commands *****/
    uint8_t addBowActuator();

    bool removeBowActuator(uint8_t t_actuator);

    uint8_t setBowActuator(uint8_t t_actuator);

//    uint8_t getBowActuator();

    uint8_t getBowActuatorCount();

    bool loadBowActuator();

    bool loadBowActuator(uint8_t t_actuator);

    bool saveBowActuator(uint16_t actuator, String name);
/*
    bool setBowActuatorFirstTouchPressure(uint16_t t_firstTouchPressure);

    bool setBowActuatorStallPressure(uint16_t t_stallPressure);

    bool setBowActuatorRestPosition(uint16_t t_restPosition);

    bool setBowActuatorID(String t_id);
*/
    uint16_t getBowActuatorFirstTouchPressure(int8_t t_bowActuatorIndex = -1);

    uint16_t getBowActuatorStallPressure(int8_t t_bowActuatorIndex = -1);

    uint16_t getBowActuatorRestPosition(int8_t t_bowActuatorIndex = -1);

    String getBowActuatorID(int8_t t_bowActuatorIndex = -1);

    bool setBowActuatorData(int8_t t_bowActuatorIndex, uint16_t t_firstTouchPressure, uint16_t t_stallPressure, uint16_t t_restPosition, String t_id);
};

#endif // BOWACTUATORS_H
