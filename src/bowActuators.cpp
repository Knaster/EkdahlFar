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
#ifndef BOWACTUATORS_C
#define BOWACTUATORS_C

/*
    Conforming to STD C++ naming conventions with the exception of where external non-conforming items are referenced
*/

#include <vector>
#include "bowActuators.h"

BowActuators::BowActuators(CalibrationData *t_calibrationDataConnect)
{
    m_calibrationDataConnect = t_calibrationDataConnect;
    m_bowActuator.push_back({0, 65535, 0});
}

BowActuators::~BowActuators()
{
    //dtor
}

uint8_t BowActuators::addBowActuator() {
    m_bowActuator.push_back({0, 65535, 0});
    return m_bowActuator.size() - 1;
}

bool BowActuators::removeBowActuator(uint8_t t_actuator) {
    if (t_actuator >= m_bowActuator.size()) {
        return false;
    }
    m_bowActuator.erase(m_bowActuator.begin() + t_actuator);
    if (m_currentBowActuator >= t_actuator) {
        m_currentBowActuator = 0;
        debugPrintln("Removed active bow actuator, resetting to actuator 0", debugPrintType::Debug);
    }
    return true;
}


uint8_t BowActuators::setBowActuator(uint8_t t_actuator) {
    if (t_actuator >= m_bowActuator.size()) {
        //debugPrintln("Actuator over current size, increasing buffer", debugPrintType::Debug);
        //m_currentBowActuator = bowActuator.size() - 1;
        debugPrintln("Actuator over current size!", debugPrintType::Error);
    } else {
        m_currentBowActuator = t_actuator;
    }
    return m_currentBowActuator;
};

uint8_t BowActuators::getBowActuator() {
    return m_currentBowActuator;
}

uint8_t BowActuators::getBowActuatorCount() {
    return m_bowActuator.size();
}

bool BowActuators::loadBowActuator() {
    m_calibrationDataConnect->firstTouchPressure = m_bowActuator[m_currentBowActuator].firstTouchPressure;
    m_calibrationDataConnect->stallPressure = m_bowActuator[m_currentBowActuator].stallPressure;
    m_calibrationDataConnect->restPosition = m_bowActuator[m_currentBowActuator].restPosition;
    return true;
}

bool BowActuators::saveBowActuator() {
    m_bowActuator[m_currentBowActuator].firstTouchPressure = m_calibrationDataConnect->firstTouchPressure;
    m_bowActuator[m_currentBowActuator].stallPressure = m_calibrationDataConnect->stallPressure;
    m_bowActuator[m_currentBowActuator].restPosition = m_calibrationDataConnect->restPosition;
    return true;
};

bool BowActuators::setBowActuatorFirstTouchPressure(uint16_t t_firstTouchPressure)  {
    m_bowActuator[m_currentBowActuator].firstTouchPressure = t_firstTouchPressure;
    return true;
};

bool BowActuators::setBowActuatorStallPressure(uint16_t t_stallPressure)  {
    m_bowActuator[m_currentBowActuator].stallPressure = t_stallPressure;
    return true;
};

bool BowActuators::setBowActuatorRestPosition(uint16_t t_restPosition) {
    m_bowActuator[m_currentBowActuator].restPosition = t_restPosition;
    return true;
};

bool BowActuators::setBowActuatorID(String t_id) {
    m_bowActuator[m_currentBowActuator].id = t_id;
    return true;
}

uint16_t BowActuators::getBowActuatorFirstTouchPressure(int8_t t_bowActuatorIndex) {
    if ((t_bowActuatorIndex < 0) || (t_bowActuatorIndex > (m_bowActuator.size() -1))) { t_bowActuatorIndex = m_currentBowActuator; }
    return m_bowActuator[t_bowActuatorIndex].firstTouchPressure;
}

uint16_t BowActuators::getBowActuatorStallPressure(int8_t t_bowActuatorIndex) {
    if ((t_bowActuatorIndex < 0) || (t_bowActuatorIndex > (m_bowActuator.size() -1))) { t_bowActuatorIndex = m_currentBowActuator; }
    return m_bowActuator[t_bowActuatorIndex].stallPressure;
}

uint16_t BowActuators::getBowActuatorRestPosition(int8_t t_bowActuatorIndex) {
    if ((t_bowActuatorIndex < 0) || (t_bowActuatorIndex > (m_bowActuator.size() -1))) { t_bowActuatorIndex = m_currentBowActuator; }
    return m_bowActuator[t_bowActuatorIndex].restPosition;
}

String BowActuators::getBowActuatorID(int8_t t_bowActuatorIndex) {
    if ((t_bowActuatorIndex < 0) || (t_bowActuatorIndex > (m_bowActuator.size() -1))) { t_bowActuatorIndex = m_currentBowActuator; }
    return m_bowActuator[t_bowActuatorIndex].id;
}

bool BowActuators::setBowActuatorData(int8_t t_bowActuatorIndex, uint16_t t_firstTouchPressure, uint16_t t_stallPressure, uint16_t t_restPosition, String t_id) {
    if (t_bowActuatorIndex >= m_bowActuator.size()) {
        if (t_bowActuatorIndex == m_bowActuator.size()) {
            debugPrintln("Bow actuator index out of range by one, adding bow index", debugPrintType::Debug);
            t_bowActuatorIndex = addBowActuator();
        } else {
            debugPrintln("Bow actuator index out of range by more than one", debugPrintType::Error);
            return false;
        }
    }
    m_bowActuator[t_bowActuatorIndex].stallPressure = t_stallPressure;
    m_bowActuator[t_bowActuatorIndex].firstTouchPressure = t_firstTouchPressure;
    m_bowActuator[t_bowActuatorIndex].restPosition = t_restPosition;
    m_bowActuator[t_bowActuatorIndex].id = t_id;
    return true;
};

String BowActuators::dumpData() {
    String dump = "";
    for (uint8_t i = 0; i < (m_bowActuator.size()); i++) {
        dump += "bad:" + String(i) + ":" + String(m_bowActuator[i].firstTouchPressure) + ":" + String(m_bowActuator[i].stallPressure) + ":" +
            String(m_bowActuator[i].restPosition) + ":" + delimitExpression(m_bowActuator[i].id, true) + ",";
    }
    dump += "bas:" + String(m_currentBowActuator) + ",bal,";
    return dump;
}
#endif
