#ifndef CONTROLREADER_H
#define CONTROLREADER_H

#include <Adafruit_ADS1X15.h>

class averager
{
    public:
        int32_t value;                          // Final value after averaging
        uint8_t dataAverageLength = 1;          // Amount of data to store and average
        uint16_t dataIndex;                     // Current pointer in data array
        uint16_t dataArray[10];  // Data array
        uint16_t dataCount;                     // Amount of data stored, will be the same as dataAverageLength at all times except startup
    private:
        bool pDataChanged = false;              // Flag to signal that the data has changed in accordance to the error parameters
        elapsedMillis timeSinceChange = 0;      // Time measuring class for error calculation
        elapsedMillis tempEM;

    public:
        uint8_t errorThreshold = 2;         //2, 0
        uint8_t delayErrorThreshold = 10;   //10, 5, 20          // Amount of data change required to signal dataChanged after errorTimeThreshold has elapsed
        uint16_t errorTimeThreshold = 20;  //20, 10, 50     // Amount of time that has elapsed since last time the dataChanged flag has been signaled
        bool trigger = false;                   // Set to true to enable a pDataChanged whenever the data goes above or below the value given by triggerThreshold
        uint16_t triggerThreshold = 100;           // Threshold which at to trigger a data change
        int16_t triggerDelay = 3000;               // Delay from trigger to sampling, given in uS
        elapsedMicros triggerDelayEM;
        bool triggerDelayEngage = false;        // Flag to indicate trigger has been received and sampling delay engaged

        bool singleShot = false;

        averager() {
            dataIndex = 0;
            dataCount = 0;

            for (int i = 0; i < dataAverageLength; i++) {
                dataArray[i] = 0;
            }
        }

        bool dataChanged() {
            bool temp = pDataChanged;
            pDataChanged = false;
            return temp;
        }

        bool addData(int16_t data) {
            if (dataCount < dataAverageLength) {
                dataCount++;
                value = -1;

                dataArray[dataIndex] = data;
                dataIndex++;
                if (dataIndex > dataAverageLength) { dataIndex = 0; }
            } else {
                int16_t prevData = dataIndex - 1;
                if (prevData < 0) { prevData = dataAverageLength - 1; }

                dataArray[dataIndex] = data;
                dataIndex++;
                if (dataIndex > dataAverageLength) { dataIndex = 0; }

                int32_t temp = 0;
                for (int i = 0; i < dataAverageLength; i++) {
                    temp += dataArray[i];
                }
                temp = temp / dataAverageLength;

                if (trigger) {
                    if (triggerDelayEngage) {
                        if (triggerDelayEM >= triggerDelay) {
                            pDataChanged = true;
                            timeSinceChange = 0;
                            triggerDelayEngage = false;
                        }
                    } else
                    if (singleShot) {
//                        if (((value <= triggerThreshold) && (temp > triggerThreshold)  ))
                    } else
                    if (((value <= triggerThreshold) && (temp > triggerThreshold)) || ((value >= triggerThreshold) && (temp < triggerThreshold))) {
                        if (triggerDelay == 0) {
                            pDataChanged = true;
                            timeSinceChange = 0;
                        } else {
                            triggerDelayEngage = true;
                            triggerDelayEM = 0;
                        }
                    }
                } else {
                    if (timeSinceChange < errorTimeThreshold) {
                        if ((temp < (value - errorThreshold)) || (temp > (value + errorThreshold))) {
                            pDataChanged = true;
                            timeSinceChange = 0;
                        }
                    } else {
                        if ((temp < (value - delayErrorThreshold)) || (temp > (value + delayErrorThreshold))) {
                            pDataChanged = true;
                            timeSinceChange = 0;
                        }
                    }
                }

                value = temp;
            }
            return true;
        }
};

class controlReader
{
    public:
        controlReader(uint8_t inDataReadyPin, uint8_t inGatePin);
        virtual ~controlReader();
        void readData();
//                   normal -                   "m:0,f:65*2^(1 / 15878 * value)",
//                      quantize - "m:0,h:value/1191.516 667"
//m:0,shs:(-31040+value)

// Commands are; Base Note CV, Fine / Modulation CV, Mute CV, Pressure CV, Hammer CV-Trigger, Engage trigger

//        String cvInputCommands[8] = { "m:0,h:(value-157)/1191.6", "m:0,shs5:((value-31198.75)*0.934519)", "m:0,msp:value", "m:0,spm:value",
//            "m:0,se:value", "m:0,run:1,pid:1,ssm:0,engage:bool(value-32767),rest:ibool(value-32767)", "", "" };
            //"m:0,run:1,pid:1,engage:1,ssm:0", "m:0,rest:1,ssm:0" };

        String cvInputCommands[8] = {
            "m:0,bch:value/1327.716667-0.39",
            "m:0,bchs5:((value-32000)*1.018082958)",
            "m:0,msp:value",
            "m:0,bpb:value",
            "m:0,se:value",
            "m:0,bmr:bool(value-32767),bpid:1,bcsm:0,bpe:bool(value-32767),bpr:ibool(value-32767)",
            "", "" };

        bool adcDebugReport = true;

    protected:
        uint16_t gateState = 0;

    private:
        bool adsInit = false;
        Adafruit_ADS1115 ads;
        Adafruit_ADS1015 ads2;
        uint8_t pinDataReady, pinGate;

        averager averages[8];
        uint16_t currentChannel;
        uint16_t currentChannel2;
};

#endif // CONTROLREADER_H
