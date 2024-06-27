#ifndef BOWACTUATORS_H
#define BOWACTUATORS_H

/*
    Conforming to STD C++ naming conventions with the exception of where external non-conforming items are referenced
*/

class BowActuators
{
    public:
        BowActuators(CalibrationData *t_calibrationDataConnect);
        virtual ~BowActuators();

        CalibrationData *m_calibrationDataConnect;

        struct BowActuator {
            uint16_t firstTouchPressure = 0;
            uint16_t stallPressure = 65535;
            uint16_t restPosition = 0;
            String id;
        };
        uint8_t addBowActuator();
        bool removeBowActuator(uint8_t t_actuator);
        uint8_t setBowActuator(uint8_t t_actuator);
        uint8_t getBowActuator();
        uint8_t getBowActuatorCount();
        bool loadBowActuator();
        bool saveBowActuator();
        bool setBowActuatorFirstTouchPressure(uint16_t t_firstTouchPressure);
        bool setBowActuatorStallPressure(uint16_t t_stallPressure);
        bool setBowActuatorRestPosition(uint16_t t_restPosition);
        bool setBowActuatorID(String t_id);
        uint16_t getBowActuatorFirstTouchPressure(int8_t t_bowActuatorIndex = -1);
        uint16_t getBowActuatorStallPressure(int8_t t_bowActuatorIndex = -1);
        uint16_t getBowActuatorRestPosition(int8_t t_bowActuatorIndex = -1);
        String getBowActuatorID(int8_t t_bowActuatorIndex = -1);
        bool setBowActuatorData(int8_t t_bowActuatorIndex, uint16_t t_firstTouchPressure, uint16_t t_stallPressure, uint16_t t_restPosition, String t_id);
        String dumpData();
    protected:
        uint8_t m_currentBowActuator = 0;
    private:
        std::vector<BowActuator> m_bowActuator;
};

#endif // BOWACTUATORS_H
