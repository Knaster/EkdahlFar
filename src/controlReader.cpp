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

    averages[4].trigger = true;
    averages[4].dataAverageLength = 1;
    averages[5].trigger = true;

//    averages[1].errorTimeThreshold = 25;
//    averages[1].delayErrorThreshold = 10;
    averages[1].delayErrorThreshold = 5;

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
        ads.setDataRate(RATE_ADS1115_128SPS);   // default = RATE_ADS1115_128SPS*/
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

    Wire.setClock(100000);
//    Wire.setClock(400000);

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
                    currentChannel2 = ADS1X15_REG_CONFIG_MUX_SINGLE_0;  // Stop here
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
                processLocalMessage(&cvInputCommands[4 + ch]);
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

            averages[ch].addData(a);

            if (averages[ch].dataChanged()) {
                // Convert value to 0-65535 range
                int32_t convertedValue = (int32_t) ((float) averages[ch].value * ((float) 65536 / 32767));
                if (convertedValue > 65535) { convertedValue = 65535; }
//                if (ch == 0) {
//                    debugPrintln("Data changed on ADS1, channel " + String(ch) + " to " + String(convertedValue) + " (" + String(averages[ch].value) + ")", debugPrintType::Debug);
                if (adcDebugReport) {
                    debugPrintln("adcr:" + String(ch) + ":" + String(convertedValue) + ":" + String(averages[ch].value), debugPrintType::InfoRequest);
                }
//                }
                dvalue = (double) convertedValue;
                // The process adds roughly 5mS of latency, add to queue as priority aka first in line?
                processLocalMessage(&cvInputCommands[ch]);
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

void controlReader::setDefaults() {
    cvInputCommands.clear();
    cvInputCommands.push_back("m:0,bch:value/1327.716667-0.39");
    cvInputCommands.push_back("m:0,bchs5:((value-32000)*1.018082958)");
    cvInputCommands.push_back("m:0,msp:value");
    cvInputCommands.push_back("m:0,bpb:value");
    cvInputCommands.push_back("m:0,se:value");
    cvInputCommands.push_back("m:0,bmr:bool(value-32767),bpid:1,bcsm:0,bpe:bool(value-32767),bpr:ibool(value-32767)");
    cvInputCommands.push_back("");
    cvInputCommands.push_back("");
}

String controlReader::dumpData() {
//    if (!adsInit) { return ""; }

    String saveData = "";
    for (uint8_t i = 0; i < 8; i++) {
        saveData += "acm:" + String(i) + ":'" + cvInputCommands[i] + "',";
    }
    return saveData;
}

controlReader::~controlReader()
{
}
