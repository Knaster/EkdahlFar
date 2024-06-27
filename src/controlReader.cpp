#include "controlReader.h"

volatile bool adsNewData = false;
volatile bool adsNewData2 = false;
volatile bool gateChanged = false;

void IRS_AdsDataReady() {
    adsNewData = true;
}

void IRS_AdsDataReady2() {
    adsNewData2 = true;
}


void IRS_GateChanged() {
    gateChanged = true;
}

controlReader::controlReader(uint8_t inDataReadyPin, uint8_t inGatePin)
{
    setDefaults();

    pinDataReady = inDataReadyPin;
    pinGate = inGatePin;

    pinMode(pinDataReady, INPUT);
    pinMode(pinGate, INPUT);

    averages[0].continuousTimeout = 5;
    averages[0].interruptedErrorThreshold = 20;

    averages[1].interruptedErrorThreshold = 25;
    //averages[1].dataAverageLength = 1;

    //averages[3].dataAverageLength = 3;
    averages[3].interruptedErrorThreshold = 20;
    averages[4].trigger = true;
    averages[4].dataAverageLength = 1;
    averages[5].trigger = true;
    averages[6].interruptedErrorThreshold = 10;

//    averages[1].errorTimeThreshold = 25;
//    averages[1].delayErrorThreshold = 10;

//    averages[0].errorThreshold = 150;
//    averages[0].delayErrorThreshold = 50;

//    Wire.setClock(1000000);
    if (!ads.begin(0x48, &Wire)) {
        debugPrintln("Failed to initialize ADS.", debugPrintType::Debug);
        //return;
    } else {
        adsInit = true;
        ads.setGain(GAIN_ONE);    //GAIN_TWOTHIRDS
        ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);
        //ads.setDataRate(RATE_ADS1115_128SPS);   // default = RATE_ADS1115_128SPS*/
        ads.setDataRate(RATE_ADS1115_860SPS);
        //RATE_ADS1115_860SPS
    }

    if (!ads2.begin(0x49, &Wire)) {
        debugPrintln("Failed to initialize ADS2", debugPrintType::Debug);
        return;
    }
    if (!adsInit) { return; }
    adsInit = true;
    ads2.setGain(GAIN_ONE);
    ads2.setDataRate(RATE_ADS1015_3300SPS);

//    Wire.setClock(100000);
    Wire.setClock(400000);

    attachInterrupt(digitalPinToInterrupt(pinDataReady), IRS_AdsDataReady, FALLING);
    //attachInterrupt(digitalPinToInterrupt(pinGate), IRS_GateChanged, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinGate), IRS_AdsDataReady2, FALLING);

    ads2.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);

    currentChannel = ADS1X15_REG_CONFIG_MUX_SINGLE_0;
    currentChannel2 = ADS1X15_REG_CONFIG_MUX_SINGLE_0;

    debugPrintln("ADS Initialized", debugPrintType::Debug);
    return;
}

//elapsedMicros tempEM;

void controlReader::readData() {
    int16_t a;

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
                    currentChannel2 = ADS1X15_REG_CONFIG_MUX_SINGLE_0;
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
                if (adcDebugReport) {
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
        } else {
            debugPrintln("No data on ADS2!", Debug);
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
                    debugPrintln("Data changed on ADS1, channel " + String(ch) + " to " + String(convertedValue) + " (" + String(averages[ch].value) + ")", debugPrintType::Debug);
                }
                if (adcDebugReport) {
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

        }
    }

}


bool controlReader::setADCCommands(uint8_t channel, String commands) {
    if ((channel > 7)) {
        return false;
    }
    //cvInputCommands.erase(cvInputCommands.begin() + channel);
    //cvInputCommands.insert(cvInputCommands.begin() + channel, commands);

    commands.replace("\"", "");
    commands.replace("'", "");
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
    cvInputCommands.push_back("bch:value/1327.716667-0.39");
    cvInputCommands.push_back("bchs5:deadband((value-38350)*0.49064)");
    cvInputCommands.push_back("msp:value");
    cvInputCommands.push_back("bpb:value");
    cvInputCommands.push_back("se:value");
    cvInputCommands.push_back("bmr:1,bpid:1,bcsm:0,bpe:bool(value-32767),bpr:ibool(value-32767)");
    cvInputCommands.push_back("sfm:1/65535*value");
    cvInputCommands.push_back("");
}

String controlReader::dumpData() {
//    if (!adsInit) { return ""; }

    String saveData = "epdbt:" + String(epDeadbandThreshold);
    for (uint8_t i = 0; i < 8; i++) {
        saveData += "acm:" + String(i) + ":'" + cvInputCommands[i] + "',";
    }
    return saveData;
}

controlReader::~controlReader()
{
}
