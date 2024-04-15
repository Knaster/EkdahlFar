#ifndef BOWACTUATORS_C
#define BOWACTUATORS_C

#include <vector>
#include "bowActuators.h"

BowActuators::BowActuators(_calibrationData *t_calibrationDataConnect)
{
    calibrationDataConnect = t_calibrationDataConnect;
    bowActuator.push_back({0, 65535, 0});
}

BowActuators::~BowActuators()
{
    //dtor
}

uint8_t BowActuators::setBowActuator(uint8_t t_actuator) {
    if (t_actuator >= bowActuator.size()) {
        debugPrintln("Actuator over current size, increasing buffer", debugPrintType::Debug);
        bowActuator.push_back({0, 65535, 0});
        m_currentBowActuator = bowActuator.size() - 1;
    } else {
        m_currentBowActuator = t_actuator;
    }
    return m_currentBowActuator;
};

uint8_t BowActuators::getBowActuator() {
    return m_currentBowActuator;
}

uint8_t BowActuators::getBowActuatorCount() {
    return bowActuator.size();
}

bool BowActuators::loadBowActuator() {
    calibrationDataConnect->firstTouchPressure = bowActuator[m_currentBowActuator].firstTouchPressure;
    calibrationDataConnect->stallPressure = bowActuator[m_currentBowActuator].stallPressure;
    calibrationDataConnect->restPosition = bowActuator[m_currentBowActuator].restPosition;
    return true;
}

bool BowActuators::saveBowActuator() {
    bowActuator[m_currentBowActuator].firstTouchPressure = calibrationDataConnect->firstTouchPressure;
    bowActuator[m_currentBowActuator].stallPressure = calibrationDataConnect->stallPressure;
    bowActuator[m_currentBowActuator].restPosition = calibrationDataConnect->restPosition;
    return true;
};

bool BowActuators::setBowActuatorFirstTouchPressure(uint16_t t_firstTouchPressure)  {
    bowActuator[m_currentBowActuator].firstTouchPressure = t_firstTouchPressure;
    return true;
};

bool BowActuators::setBowActuatorStallPressure(uint16_t t_stallPressure)  {
    bowActuator[m_currentBowActuator].stallPressure = t_stallPressure;
    return true;
};

bool BowActuators::setBowActuatorRestPosition(uint16_t t_restPosition) {
    bowActuator[m_currentBowActuator].restPosition = t_restPosition;
    return true;
};

bool BowActuators::setBowActuatorID(String t_id) {
    bowActuator[m_currentBowActuator].id = t_id;
    return true;
}

uint16_t BowActuators::getBowActuatorFirstTouchPressure() {
    return bowActuator[m_currentBowActuator].firstTouchPressure;
}

uint16_t BowActuators::getBowActuatorStallPressure() {
    return bowActuator[m_currentBowActuator].stallPressure;
}

uint16_t BowActuators::getBowActuatorRestPosition() {
    return bowActuator[m_currentBowActuator].restPosition;
}

String BowActuators::getBowActuatorID() {
    return bowActuator[m_currentBowActuator].id;
}

bool BowActuators::setBowActuatorData(uint16_t t_firstTouchPressure, uint16_t t_stallPressure, uint16_t t_restPosition, String t_id) {
    bowActuator[m_currentBowActuator].stallPressure = t_stallPressure;
    bowActuator[m_currentBowActuator].firstTouchPressure = t_firstTouchPressure;
    bowActuator[m_currentBowActuator].restPosition = t_restPosition;
    bowActuator[m_currentBowActuator].id = t_id;
    return true;
};

String BowActuators::dumpData() {
    String dump = "";
    for (uint8_t i = 0; i < (bowActuator.size()); i++) {
        dump += "bas:" + String(i) + ",";
        dump += "bad:" + String(bowActuator[i].firstTouchPressure) + ":" + String(bowActuator[i].stallPressure) + ":" + String(bowActuator[i].restPosition) + ":" +
            bowActuator[i].id + ",";
    }
    dump += "bas:" + String(m_currentBowActuator) + ",bal,";
    return dump;
}
#endif
