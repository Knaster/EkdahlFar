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
#include "controlReader.h"

volatile bool adsNewData = false;
volatile bool adsNewData2 = false;
//volatile bool gateChanged = false;

//volatile bool adsNewDataX[2] = { false, false };

void IRS_AdsDataReady() {
    adsNewData = true;
//    adsNewDataX[0] = true;
}

void IRS_AdsDataReady2() {
    adsNewData2 = true;
//    adsNewDataX[1] = true;
}
/*
void IRS_GateChanged() {
    gateChanged = true;
}
*/
controlReader::controlReader(uint8_t inDataReadyPin, uint8_t inGatePin)
{
//    ADC[0].init(1); // = new ADC(1);
//    ADC[1].init(0); // = new ADC(0);

    setDefaults();

    pinDataReady = inDataReadyPin;
    pinGate = inGatePin;

    pinMode(pinDataReady, INPUT);
    pinMode(pinGate, INPUT);

    averages[4].trigger = true;
    averages[4].dataAverageLength = 1;
    averages[5].trigger = true;

    resetAds();

    attachInterrupt(digitalPinToInterrupt(pinDataReady), IRS_AdsDataReady, FALLING);
    attachInterrupt(digitalPinToInterrupt(pinGate), IRS_AdsDataReady2, FALLING);
    return;
}

/*! \brief Tries to reconnect to the ADC converters
 */

void controlReader::resetAds() {
    adsInit = false;

    if (!ads.begin(0x48, &Wire)) {
        debugPrintln("Failed to initialize ADS.", debugPrintType::Debug);
        return;
    } else {
        ads.setGain(GAIN_ONE);    //GAIN_TWOTHIRDS
        ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);
        ads.setDataRate(RATE_ADS1115_860SPS);
    }

    if (!ads2.begin(0x49, &Wire)) {
        debugPrintln("Failed to initialize ADS2", debugPrintType::Debug);
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
/*
void controlReader::handleADCData(uint16_t adcIndex) {
    bool iAdsNewData = false;
    bool iAdsConversionComplete;

    switch (adcIndex) {
        case 0:
            iAdsNewData = adsNewData;
            iAdsConversionComplete = ads.conversionComplete();
        case 1:

    }

Adafruit_ADS1X15
}
*/
void controlReader::readData() {
    int16_t a;

    if ((adsErrorReported || ads2ErrorReported) && ((millis() > (adsReinitCountStart + adsReinitializeTimeout)))) {
        adsReinitCountStart = millis();
        debugPrintln("Trying to reinitialize control box", debugPrintType::Debug);
        resetAds();
    }

    if (!adsInit) { return; }

    if (adsNewData2) {
        if (ads2.conversionComplete()) {
            adsNewData2 = false;
            a = ads2.getLastConversionResults();

            uint8_t ch = 0;
            switch(currentChannel2) {
                case ADS1X15_REG_CONFIG_MUX_SINGLE_0:
                    ch = 0;
                    currentChannel2 = ADS1X15_REG_CONFIG_MUX_SINGLE_1;
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_1:
                    ch = 1;
                    currentChannel2 = ADS1X15_REG_CONFIG_MUX_SINGLE_2;  // Stop here
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_2:
                    ch = 2;
                    currentChannel2 = ADS1X15_REG_CONFIG_MUX_SINGLE_3;
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_3:
                    ch = 3;
                    currentChannel2 = ADS1X15_REG_CONFIG_MUX_SINGLE_0;
                    break;
            }

            if (a < 0) { a = 0; }

//            if ((ch == 0) && (a > 100)) {
//                stringModuleArray[0].solenoidArray[0].solenoidEngage();
//            }

            if (testMeasurementOngoing) {
                debugPrintln("New data (" + String(a) + ") on ADC channel " + String(ch + 4) + " at " + String(testMeasurement) + "uS", Debug);
            }
            if (testChannel == (ch + 4)) {
                addTestData(a);
            }

            averages[4 + ch].addData(a);

            if (averages[4 + ch].dataChanged()) {
                // Convert value to 0-65535 range
                int32_t convertedValue = (int32_t) ((float) averages[4 + ch].value * ((float) 65536 / 2048));
                if (convertedValue > 65535) { convertedValue = 65535; }
                dvalue = (double) convertedValue;
//                if (ch == 1) {
//                    debugPrintln("Data changed on ADS2, channel " + String(ch) + " to " + String(convertedValue) + " (" + String(averages[4 + ch].value) + ")", debugPrintType::Debug);
                if (outputDebugData) {
                    debugPrintln("adcr:" + String(ch + 4) + ":" + String(convertedValue) + ":" + String(averages[4 + ch].value), debugPrintType::InfoRequest);
                }

//                }
                if (!testMeasurementOngoing) {
                    processLocalMessage(&cvInputCommands[4 + ch]);
                } else {
                    String tempMessages = "talr";
                    processLocalMessage(&tempMessages);
                }

                // The process adds roughly 5mS of latency, add to queue as priority aka first in line?
            }

            ads2.startADCReading(currentChannel2, false);
            ads2ConversionStart = millis();
            ads2ErrorReported = false;
        } else {
            if ((millis() > (ads2ConversionStart + ads2TimeOut)) && (!ads2ErrorReported)) {
                debugPrintln("No data on ADS2!", Error);
                ads2ErrorReported = true;
                adsReinitCountStart = millis();
            }
        }
    }

    if (adsNewData) {
        if (ads.conversionComplete()) {
            adsNewData = false;

            a = ads.getLastConversionResults();
            uint8_t ch = 0;
            switch(currentChannel) {
                case ADS1X15_REG_CONFIG_MUX_SINGLE_0:
                    ch = 0;
                    currentChannel = ADS1X15_REG_CONFIG_MUX_SINGLE_1;
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_1:
                    ch = 1;
                    currentChannel = ADS1X15_REG_CONFIG_MUX_SINGLE_2;
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_2:
                    ch = 2;
                    currentChannel = ADS1X15_REG_CONFIG_MUX_SINGLE_3;
                    break;
                case ADS1X15_REG_CONFIG_MUX_SINGLE_3:
                    ch = 3;
                    currentChannel = ADS1X15_REG_CONFIG_MUX_SINGLE_0;
                    break;
            }

            if (a < 0) { a = 0; }

            if (testMeasurementOngoing) {
                debugPrintln("New data (" + String(a) + ") on ADC channel " + String(ch) + " at " + String(testMeasurement) + "uS", Debug);
            }
            if (testChannel == (ch)) {
                addTestData(a);
            }

            averages[ch].addData(a);
/*
            if (ch == 1) {
                debugPrintln("Data on ADS1, channel " + String(ch) + " is " + String(a), debugPrintType::Debug);
            }
*/
            if (averages[ch].dataChanged()) {
                // Convert value to 0-65535 range
                int32_t convertedValue = (int32_t) ((float) averages[ch].value * ((float) 65536 / 32767));
                if (convertedValue > 65535) { convertedValue = 65535; }
                if (ch == 0) {
                    if (outputDebugData) {
                        debugPrintln("Data changed on ADS1, channel " + String(ch) + " to " + String(convertedValue) + " (" + String(averages[ch].value) + ")", debugPrintType::Debug);
                    }
                }
                if (outputDebugData) {
                    debugPrintln("adcr:" + String(ch) + ":" + String(convertedValue) + ":" + String(averages[ch].value), debugPrintType::InfoRequest);
                }
//                }
                dvalue = (double) convertedValue;
                // The process adds roughly 5mS of latency, add to queue as priority aka first in line?
                if (!testMeasurementOngoing) {
                    processLocalMessage(&cvInputCommands[ch]);
                } else {
                    String tempMessages = "talr";
                    processLocalMessage(&tempMessages);
                }
//                processLocalMessage(&cvInputCommands[ch]);
            }

            ads.startADCReading(currentChannel, false);
            adsConversionStart = millis();
            adsErrorReported = false;
        } else {
            if ((millis() > (adsConversionStart + adsTimeOut)) && (!adsErrorReported)) {
                debugPrintln("No data on ADS!", Error);
                adsErrorReported = true;
                adsReinitCountStart = millis();
            }
        }
    }
}


bool controlReader::setADCCommands(uint8_t channel, String commands) {
    if ((channel > 7)) { return false; }

    commands = stripQuotes(commands);
    /*if ((commands[0] == "'") || (commands[0] == '"')) {
        commands = commands.substring(1, commands.length() - 1);
        debugPrintln("Found initial quote, stripping and getting " + commands, debugPrintType::Debug);
    }*/
    cvInputCommands[channel] = commands;
    return true;
}

bool controlReader::setADCAveragerSettings(uint8_t t_channel, uint8_t t_averages, uint16_t t_interruptErrorThreshold, uint16_t t_continuousErrorThreshold, uint16_t t_continuousTimeout) {
    if ((t_channel > 7)) {
        return false;
    }
    averages[t_channel].dataAverageLength = t_averages;
    averages[t_channel].interruptedErrorThreshold = t_interruptErrorThreshold;
    averages[t_channel].continuousErrorThreshold = t_continuousErrorThreshold;
    averages[t_channel].continuousTimeout = t_continuousTimeout;

    return true;
}

String controlReader::getADCAveragerSettings(uint8_t t_channel) {
    if (t_channel > 7) { return ""; }
    return String(averages[t_channel].dataAverageLength) + ":" + String(averages[t_channel].interruptedErrorThreshold) + ":" + String(averages[t_channel].continuousErrorThreshold) + ":" +
        String(averages[t_channel].continuousTimeout);
}

void controlReader::addTestData(uint16_t value) {
    if (testBegin) {
        testMin = value;
        testMax = value;
        testBegin = false;
    } else {
        if (value < testMin) { testMin = value; }
        if (value > testMax) { testMax = value; }
    }
}

void controlReader::setADCMinMaxTestChannel(uint8_t t_channel) {
    testChannel = t_channel;
    testBegin = true;
}

void controlReader::setDefaults() {
    cvInputCommands.clear();
    cvInputCommands.push_back("bcha:value/1327.716667-20");
    cvInputCommands.push_back("bchs5:\"deadband(value-32236, 20)/2.425\"");
    cvInputCommands.push_back("bchsh:\"deadband((value-32600)*0.49064, 250)\"");//    cvInputCommands.push_back("msp:value");
    cvInputCommands.push_back("bpb:value");
    cvInputCommands.push_back("se:value");
    cvInputCommands.push_back("bmr:1,bpid:1,bcsm:0,bpe:bool(value-1000),bpr:ibool(value-1000),bph:ibool(value-1000)");
    cvInputCommands.push_back("sfm:\"deadband(1/65535*value,0.002)\"");
    cvInputCommands.push_back("msp:value");//    cvInputCommands.push_back("");
}

String controlReader::dumpData() {
//    if (!adsInit) { return ""; }
    String saveData = ""; // "epdbt:" + String(epDeadbandThreshold) + ",";
    for (uint8_t i = 0; i < 8; i++) {
        saveData += "acm:" + String(i) + ":" + delimitExpression(cvInputCommands[i], true) + ",";
        saveData += "adcs:" + String(i) + ":" + averages[i].dataAverageLength + ":" + averages[i].interruptedErrorThreshold + ":" + averages[i].continuousErrorThreshold + ":" + averages[i].continuousTimeout + ",";
    }
    return saveData;
}

int32_t controlReader::getData(int16_t channel) {
    if ((channel < 0) || (channel > 7)) {
        return -1;
    } else {
        return averages[channel].value;
    }
}
/*
long controlReader::findAndSetNull(uint16_t ch) {
    Adafruit_ADS1X15 *poo;

    if ((ch < 0) || (ch > 7)) { return -1; }
    if (ch < 4) {
        poo = &ads;
    } else {
        poo = &ads2;
        ch -= 4;
    }
    poo.startADCReading(ch, false);
    long timeOut = millis();
    while (!poo.conversionComplete() && (timeOut + 1000 < millis())) {
    };
}
*/
controlReader::~controlReader()
{
}
