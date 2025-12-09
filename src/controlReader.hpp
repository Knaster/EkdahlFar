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

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

class averager
{
    public:
        int32_t value = 0;                          // Final value after averaging
        uint8_t dataAverageLength = 1;          // Amount of data points to store and average
        uint16_t dataIndex;                     // Current pointer in data array
        uint16_t dataArray[10];  // Data array
        uint16_t dataCount;                     // Amount of data stored, will be the same as dataAverageLength at all times except startup
    private:
        bool pDataChanged = false;              // Flag to signal that the data has changed in accordance to the error parameters
        elapsedMillis timeSinceChange = 0;      // Time measuring class for error calculation
        elapsedMillis tempEM;

    public:
        uint16_t continuousTimeout = 10;
        uint16_t continuousErrorThreshold = 2;  //PCB5 test - 5
        uint16_t interruptedErrorThreshold = 40; //PCB5 test - 20

        bool trigger = false;                   // Set to true to enable a pDataChanged whenever the data goes above or below the value given by triggerThreshold
        uint16_t triggerThreshold = 250;           // Threshold which at to trigger a data change
        int16_t triggerDelay = 3000;               // Delay from trigger to sampling, given in uS
        elapsedMicros triggerDelayEM;
        bool triggerDelayEngage = false;        // Flag to indicate trigger has been received and sampling delay engaged

        bool singleShot = false;

        bool outputDebugData = false;

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
            // If we are still just filling up the buffer then just add the data
            if ((dataCount + 1) < dataAverageLength) {
                dataCount++;
                value = -1;

                dataArray[dataIndex] = data;
                dataIndex++;
                if (dataIndex >= dataAverageLength) { dataIndex = 0; }
            } else {

                dataArray[dataIndex] = data;
                dataIndex++;
                if (dataIndex >= dataAverageLength) { dataIndex = 0; }

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
                        debugPrintln("Value " + String(value) + " and average " + String(temp) + " over threshold " + String(triggerThreshold), Debug);
                        if (triggerDelay == 0) {
                            pDataChanged = true;
                            timeSinceChange = 0;
                        } else {
                            triggerDelayEngage = true;
                            triggerDelayEM = 0;
                        }
                    }
                } else {
                    // If a quick change has happened, aka a new value has been received before the continuousTimeout has been passed
                    // then we are being lenient and passing any value change greater than continuousErrorThreshold. This is to allow continuous sweeps
                    if (timeSinceChange < continuousTimeout) {
                        if ((temp < (value - continuousErrorThreshold)) || (temp > (value + continuousErrorThreshold))) {
                            if (outputDebugData) {
                                debugPrintln("Continuous data change occured at " + String(timeSinceChange) + " ms which is below the continuousTimeout of " + String(continuousTimeout) + " ms."
                                     "The data is above the continuousErrorThreshold of " + String(continuousErrorThreshold) + ". The new data is " + String(temp) + " while the old data was " + String(value), Debug);
                            }
                            pDataChanged = true;
                            timeSinceChange = 0;
                        }
                    } else {
                        if ((temp < (value - interruptedErrorThreshold)) || (temp > (value + interruptedErrorThreshold))) {
                            if (outputDebugData) {
                                debugPrintln("A data change occured after the errorThreshold time, the data did surpass the interruptedErrorThreshold of " + String(interruptedErrorThreshold) +
                                    ". The new data is " + String(temp) + " while the old data was " + String(value), Debug);
                            }
                            pDataChanged = true;
                            timeSinceChange = 0;
                        }
                    }
                }

                if (pDataChanged) { value = temp; }
            }
            return true;
        }
};

class ControlReader
{
    public:
        ControlReader(uint8_t inDataReadyPin, uint8_t inGatePin);
        virtual ~ControlReader();
        void readData();

        std::vector<String> cvInputCommands;

        bool outputDebugData = true;
        bool setADCCommands(uint8_t channel, String commands);

        bool setADCAveragerSettings(uint8_t t_channel, uint8_t t_averages, uint16_t t_interruptErrorThreshold, uint16_t t_continuousErrorThreshold, uint16_t t_continuousTimeout);
        String getADCAveragerSettings(uint8_t channel);
        void setDefaults();
        String dumpData();

        uint16_t testMin, testMax;
        bool testBegin = true;
        uint8_t testChannel = 0;
        void setADCMinMaxTestChannel(uint8_t t_channel);

        void resetAds();
        int32_t getData(int16_t channel);

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

        void addTestData(uint16_t value);

        long adsTimeOut = 10;
        long adsConversionStart;
        bool adsErrorReported = false;
        long ads2TimeOut = 10;
        long ads2ConversionStart;
        bool ads2ErrorReported = false;

        long adsReinitializeTimeout = 1000;
        long adsReinitCountStart;

//        bool readSingleADS(Adafruit_ADS1115 *adsx, bool *newData, uint16_t channel, uint8_t channelOffset, long *conversionStart, bool *errorReported, uint16_t timeout);
//        bool readSingleADS(Adafruit_ADS1015 *adsx, bool *newData, uint16_t channel, uint8_t channelOffset, long *conversionStart, bool *errorReported, uint16_t timeout);
        bool readSingleADS(Adafruit_ADS1X15 &adsx, volatile bool &newData, uint16_t &channel, uint8_t channelOffset, long &conversionStart, bool &errorReported, uint16_t timeout, uint16_t resolution);
};

#endif // CONTROLREADER_H
