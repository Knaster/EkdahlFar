#ifndef BOWACTUATORS_C
#define BOWACTUATORS_C

#include <vector>
#include "bowActuators.h"

BowActuators::BowActuators(_calibrationData *inCalibrationDataConnect)
{
    calibrationDataConnect = inCalibrationDataConnect;
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

    debugPrintln("Current bow actuator set to " + String(m_currentBowActuator), debugPrintType::Debug);
    return m_currentBowActuator;
};

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

String BowActuators::dumpData() {
    String dump = "";
 /*   dump += "mfmp:" + String(fullMutePosition) + ",";
    dump += "mhmp:" + String(halfMutePosition) + ",";
    dump += "mrp:" + String(restPosition) + ",";*/
    return dump;
}
#endif
