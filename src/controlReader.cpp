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
#include "controlReader.hpp"

volatile bool adsNewData = false;
volatile bool adsNewData2 = false;

void ISR_AdsDataReady() {
    adsNewData = true;
}

void ISR_AdsDataReady2() {
    adsNewData2 = true;
}

const ModuleCommandDeclaration ControlReader::moduleCommands[] = {
  { "controldata", "cda", "channel:command string", "Sets the command string invoked when the value on ADC channel [channel] changes", false, true, &s_controlData},
  { "controldefaults", "cde", "-" "Reverts all ADC command strings to default values", false, false, &s_controlDefaults },
  { "datareturn", "dr", "channel:value", "Sent when a new value is presented on one of the ADC channels, cannot be invoked", false, false, &s_dataReturn },
  { "adcsettings", "ads", "channel:averages:interrupterrorthreshold:continuouserrorthreshold:continuoustimeout", "Explain ADC settings here", false, true, &s_adcSettings },
  { "testadclatency", "tal", "0-65535", "Test ADC Latency", true, false, &s_testADCLatency },
  { "testadclatencyreturn", "talr", "-", "Return from test", true, false, &s_testADCLatencyReturn },
  { "testadcminmax", "tix", "channel", "Measure min/max value for a given channel and resets the counter", true, false, &s_testADCMinMax }
};

#define DATARETURN_CMD 2

getModuleCount(ControlReader)

ControlReader::ControlReader(uint8_t inDataReadyPin, uint8_t inGatePin)
{
    moduleID = new ModuleID("controlbox", "cb", "Control Box 1.0", eModuleType::hardware);

    setDefaults();

    pinDataReady = inDataReadyPin;
    pinGate = inGatePin;

    pinMode(pinDataReady, INPUT);
    pinMode(pinGate, INPUT);

    averages[4].trigger = true;
    averages[4].dataAverageLength = 1;
    averages[5].trigger = true;

    resetAds();

    attachInterrupt(digitalPinToInterrupt(pinDataReady), ISR_AdsDataReady, FALLING);
    attachInterrupt(digitalPinToInterrupt(pinGate), ISR_AdsDataReady2, FALLING);

    midiHandler = new MIDIHandler();
    return;
}

CREATE_MODULE_COMMAND_FUNCTION(controlData, ControlReader) {
    int8_t channel = -1;
    if (checkArgumentsMin(inCommandItem, inCommandResponses, 1, true)) {
        channel = inCommandItem->argument[0].toInt();
    }

    if (request) {
        if (channel != -1) {
            inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(channel) + ":" + delimitExpression(cvInputCommands[channel], true), InfoRequest});
        } else {
            for (int channel = 0; channel < 8; channel++) {
                inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(channel) + ":" + delimitExpression(cvInputCommands[channel], true), InfoRequest});
            }
        }
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        setADCCommands(channel, stripQuotes(inCommandItem->argument[1]));
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(channel) + ":" + delimitExpression(cvInputCommands[channel], true), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(controlDefaults, ControlReader) {
    setDefaults();
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(dataReturn, ControlReader) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    int i = inCommandItem->argument[0].toInt();
    if ((i < 0) || (i > 7)) { return eProcessResult::WrongArgumentValue; }

    int32_t convertedValue;
    if (i < 5) {
        convertedValue = (int32_t) ((float) getData(i) * ((float) 65536 / 32767));
    } else {
        convertedValue = (int32_t) ((float) getData(i) * ((float) 65536 / 2048));
    }
    if (convertedValue > 65535) { convertedValue = 65535; }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(i) + ":" + String(convertedValue) + ":" + String(getData(i)), debugPrintType::InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(adcSettings, ControlReader) {
    int8_t channel = -1;
    if (checkArgumentsMin(inCommandItem, inCommandResponses, 1, true)) {
        channel = inCommandItem->argument[0].toInt();
    }
    if (channel > 7) { return eProcessResult::WrongArgumentValue; }

    if (request) {
        if (channel != -1) {
            inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(channel) + ":" + getADCAveragerSettings(channel), InfoRequest});
        } else {
            for (int channel = 0; channel < 8; channel++) {
                inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(channel) + ":" + getADCAveragerSettings(channel), InfoRequest});
            }
        }
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 5)) { return eProcessResult::WrongArgumentCount; }
        if (!setADCAveragerSettings(inCommandItem->argument[0].toInt(), inCommandItem->argument[1].toInt(), inCommandItem->argument[2].toInt(), inCommandItem->argument[3].toInt(),
            inCommandItem->argument[4].toInt())) { return eProcessResult::CommandFailed; }
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(channel) + ":" + getADCAveragerSettings(channel), debugPrintType::InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(testADCLatency, ControlReader) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    testMeasurement = 0;
    testMeasurementOngoing = true;
    analogWrite(3, int(inCommandItem->argument[0].toInt()));
    inCommandResponses->push_back({"Starting ADC latency test", debugPrintType::TextInfo});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(testADCLatencyReturn, ControlReader) {
    testMeasurementOngoing = false;
    inCommandResponses->push_back({"Returned from ADC latency test in " + String(testMeasurement) + " uS", debugPrintType::TextInfo});
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest });
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(testADCMinMax, ControlReader) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(testChannel) + ":" + String(testMin) + ":" + String(testMax), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        int testChannel = inCommandItem->argument[0].toInt();
        if ((testChannel < 0) || (testChannel > 7)) { return eProcessResult::WrongArgumentValue; }
        setADCMinMaxTestChannel(testChannel);
        inCommandResponses->push_back({ "Starting ADC min/max test on channel " + String(testChannel), debugPrintType::TextInfo });
        inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::Debug });
    }
    return eProcessResult::Ok;
};

/*! \brief Tries to reconnect to the ADC converters
 */

void ControlReader::resetAds() {
    adsInit = false;

    if (!ads.begin(0x48, &Wire)) {
        debugPrintln("Failed to initialize ADS.", debugPrintType::Error);
        return;
    } else {
        ads.setGain(GAIN_ONE);    //GAIN_TWOTHIRDS
        ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);
        ads.setDataRate(RATE_ADS1115_860SPS);
    }

    if (!ads2.begin(0x49, &Wire)) {
        debugPrintln("Failed to initialize ADS2", debugPrintType::Error);
        return;
    }

    adsInit = true;
    ads2.setGain(GAIN_ONE);
    ads2.setDataRate(RATE_ADS1015_3300SPS);

    Wire.setClock(400000);

    ads2.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);

    currentChannel = ADS1X15_REG_CONFIG_MUX_SINGLE_0;
    currentChannel2 = ADS1X15_REG_CONFIG_MUX_SINGLE_0;

    debugPrintln("ADS Initialized", debugPrintType::Debug);
}

bool ControlReader::readSingleADS(Adafruit_ADS1X15 &adsx, volatile bool &newData, uint16_t &channel, uint8_t channelOffset, long &conversionStart, bool &errorReported, uint16_t timeout,
                                  uint16_t resolution, std::vector<commandResponse> *inCommandResponses) {

   int16_t a;
   if (newData) {
        if (adsx.conversionComplete()) {
            newData = false;
            a = adsx.getLastConversionResults();
            uint8_t ch = 0;
            switch(channel) {
                case ADS1X15_REG_CONFIG_MUX_SINGLE_0:
                    ch = 0;
                    channel = ADS1X15_REG_CONFIG_MUX_SINGLE_1;
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_1:
                    ch = 1;
                    channel = ADS1X15_REG_CONFIG_MUX_SINGLE_2;  // Stop here
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_2:
                    ch = 2;
                    channel = ADS1X15_REG_CONFIG_MUX_SINGLE_3;
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_3:
                    ch = 3;
                    channel = ADS1X15_REG_CONFIG_MUX_SINGLE_0;
                    break;
            }

            if (a < 0) { a = 0; }

            if (testMeasurementOngoing) {
                debugPrintln("New data (" + String(a) + ") on ADC channel " + String(ch + channelOffset) + " at " + String(testMeasurement) + "uS", Debug);
            }
            if (testChannel == (ch + channelOffset)) {
                addTestData(a);
            }

            averages[channelOffset + ch].addData(a);
            if (averages[channelOffset + ch].dataChanged()) {
                // Convert value to 0-65535 range
                int32_t convertedValue = (int32_t) ((float) averages[channelOffset + ch].value * ((float) 65536 / resolution));
                if (convertedValue > 65535) { convertedValue = 65535; }
                expressionParser.dvalue = (double) convertedValue;

                inCommandResponses->push_back({ moduleCommands[DATARETURN_CMD].shortCommand + ":" + String(ch + channelOffset) + ":" + String(convertedValue) + ":" +
                                              String(averages[channelOffset + ch].value), debugPrintType::InfoRequest });
/*
                if (outputDebugData) {
                    debugPrintln("adcr:" + String(ch + channelOffset) + ":" + String(convertedValue) + ":" + String(averages[channelOffset + ch].value), debugPrintType::InfoRequest);
                }
*/
                if (!testMeasurementOngoing) {
                    midiHandler->processLocalMessage(&cvInputCommands[channelOffset + ch]);
                } else {
                    String tempMessages = "talr";
                    midiHandler->processLocalMessage(&tempMessages);
                }

                // The process adds roughly 5mS of latency, add to queue as priority aka first in line?
            }

            adsx.startADCReading(channel, false);
            conversionStart = millis();
            errorReported = false;
        } else {
            if ((millis() > (conversionStart + timeout)) && (!errorReported)) {
                debugPrintln("No data on ADSx!", Error);
                errorReported = true;
                adsReinitCountStart = millis();
                return false;
            }
        }
    }

    return true;
}

void ControlReader::readData(std::vector<commandResponse> *inCommandResponses) {
    if ((adsErrorReported || ads2ErrorReported) && ((millis() > (adsReinitCountStart + adsReinitializeTimeout)))) {
        adsReinitCountStart = millis();
        debugPrintln("Trying to reinitialize control box", debugPrintType::Debug);
        resetAds();
    }

    if (!adsInit) { return; }

    readSingleADS(ads2, adsNewData2, currentChannel2, 4, ads2ConversionStart, ads2ErrorReported, ads2TimeOut, 2048, inCommandResponses);
    readSingleADS(ads, adsNewData, currentChannel, 0, adsConversionStart, adsErrorReported, adsTimeOut, 32767, inCommandResponses);
}


bool ControlReader::setADCCommands(uint8_t channel, String commands) {
    if ((channel > 7)) { return false; }

    commands = stripQuotes(commands);
    cvInputCommands[channel] = commands;
    return true;
}

bool ControlReader::setADCAveragerSettings(uint8_t t_channel, uint8_t t_averages, uint16_t t_interruptErrorThreshold, uint16_t t_continuousErrorThreshold, uint16_t t_continuousTimeout) {
    if ((t_channel > 7)) {
        return false;
    }
    averages[t_channel].dataAverageLength = t_averages;
    averages[t_channel].interruptedErrorThreshold = t_interruptErrorThreshold;
    averages[t_channel].continuousErrorThreshold = t_continuousErrorThreshold;
    averages[t_channel].continuousTimeout = t_continuousTimeout;
    return true;
}

String ControlReader::getADCAveragerSettings(uint8_t t_channel) {
    if (t_channel > 7) { return ""; }
    return String(averages[t_channel].dataAverageLength) + ":" + String(averages[t_channel].interruptedErrorThreshold) + ":" + String(averages[t_channel].continuousErrorThreshold) + ":" +
        String(averages[t_channel].continuousTimeout);
}

void ControlReader::addTestData(uint16_t value) {
    if (testBegin) {
        testMin = value;
        testMax = value;
        testBegin = false;
    } else {
        if (value < testMin) { testMin = value; }
        if (value > testMax) { testMax = value; }
    }
}

void ControlReader::setADCMinMaxTestChannel(uint8_t t_channel) {
    testChannel = t_channel;
    testBegin = true;
}

void ControlReader::setDefaults() {
    cvInputCommands.clear();
    cvInputCommands.push_back("bw.hsh.ha:\"value/1327.716667-20\"");
    cvInputCommands.push_back("bw.hsh.sh5:\"deadband(value-32236, 20)/2.425\"");
    cvInputCommands.push_back("bw.hsh.sh:\"deadband((value-32600)*0.49064, 250)\"");//    cvInputCommands.push_back("msp:value");
    cvInputCommands.push_back("bw.bp.ba:value");
    cvInputCommands.push_back("so.en:value");
    cvInputCommands.push_back("bw.dcm.ru:bool(value-10000),bw.pe:1,bw.sm:0,bw.bp.en:bool(value-10000),bw.bp.rs:ibool(value-10000),bw.bp.hd:ibool(value-10000)");
    cvInputCommands.push_back("so.fm:\"deadband(1/65535*value,0.002)\"");
    cvInputCommands.push_back("mu.sp:value");//    cvInputCommands.push_back("");
}

int32_t ControlReader::getData(int16_t channel) {
    if ((channel < 0) || (channel > 7)) {
        return -1;
    } else {
        return averages[channel].value;
    }
}
