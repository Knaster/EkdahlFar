#ifndef BOWACTUATORS_H
#define BOWACTUATORS_H

class BowActuators
{
    public:
        BowActuators(_calibrationData *inCalibrationDataConnect);
        virtual ~BowActuators();

        _calibrationData *calibrationDataConnect;

        struct BowActuator {
            uint16_t firstTouchPressure = 0;
            uint16_t stallPressure = 65535;
            uint16_t restPosition = 0;
        };

        uint8_t setBowActuator(uint8_t t_actuator);
        bool loadBowActuator();
        bool saveBowActuator();
        bool setBowActuatorFirstTouchPressure(uint16_t t_firstTouchPressure);
        bool setBowActuatorStallPressure(uint16_t t_stallPressure);
        bool setBowActuatorRestPosition(uint16_t t_restPosition);
        String dumpData();
    protected:
        uint8_t m_currentBowActuator = 0;
    private:
        std::vector<BowActuator> bowActuator;
};

#endif // BOWACTUATORS_H
