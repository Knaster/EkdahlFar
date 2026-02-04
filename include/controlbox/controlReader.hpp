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
#ifndef CONTROLREADER_H
#define CONTROLREADER_H

#include <base/module.hpp>
#include <master_controller/midi.h>

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "averager.h"

class ControlReader : public Module
{
public:
    SETMODULEID("controlbox", "cb", "Control Box 1.0", eModuleType::hardware, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(controlData, ControlReader)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(controlDefaults, ControlReader)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(dataReturn, ControlReader)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(adcSettings, ControlReader)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(testADCLatency, ControlReader)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(testADCLatencyReturn, ControlReader)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(testADCMinMax, ControlReader)

    ControlReader(uint8_t inDataReadyPin, uint8_t inGatePin, MIDIHandler *inMidiHandler);

public:
    void readData(std::vector<commandResponse> *inCommandResponses);

    std::vector<String> cvInputCommands;

    bool outputDebugData = true;
    bool setADCCommands(uint8_t channel, String commands);

    bool setADCAveragerSettings(uint8_t t_channel, uint8_t t_averages, uint16_t t_interruptErrorThreshold, uint16_t t_continuousErrorThreshold, uint16_t t_continuousTimeout);
    String getADCAveragerSettings(uint8_t channel);
    void setDefaults();

    uint16_t testMin, testMax;
    bool testBegin = true;
    uint8_t testChannel = 0;
    void setADCMinMaxTestChannel(uint8_t t_channel);

    void resetAds();
    int32_t getData(int16_t channel);

protected:
    uint16_t gateState = 0;

private:
    elapsedMicros testMeasurement;  ///< Used for various internal tests
    bool testMeasurementOngoing = false;
    ExpressionParser *expressionParser;

    MIDIHandler *midiHandler;

    bool adsInit = false;
    Adafruit_ADS1115 ads;
    Adafruit_ADS1015 ads2;
    uint8_t pinDataReady, pinGate;

    averager averages[8];
    uint16_t currentChannel;
    uint16_t currentChannel2;

    void addTestData(uint16_t value);

    long adsTimeOut = 10;
    long adsConversionStart;
    bool adsErrorReported = false;
    long ads2TimeOut = 10;
    long ads2ConversionStart;
    bool ads2ErrorReported = false;

    long adsReinitializeTimeout = 1000;
    long adsReinitCountStart;

    bool readSingleADS(Adafruit_ADS1X15 &adsx, volatile bool &newData, uint16_t &channel, uint8_t channelOffset, long &conversionStart, bool &errorReported, uint16_t timeout, uint16_t resolution,
                       std::vector<commandResponse> *inCommandResponses);
};

#endif // CONTROLREADER_H
