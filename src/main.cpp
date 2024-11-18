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

/**
 * @file main.cpp
 *
 * @mainpage Ekdahl FAR main file
 *
 * \image html knaslogo_nofont.svg
 * @section description Description
 * Firmware for the KNAS Ekdahl FAR, a string based electro-acoustic music instrument
 *
 * @section terminology Terminologoy
 * - actuator - the object that is physically rotated against the string, usually a replaceable felt disc.
 * - soft-limit - a stored value above or below which a certain physical movement or action will not be taken, e.g. minimum motor speed or maximum bow pressure
 * - command message - a text-based message with none or more arguments, all external control of the software is done through these messages
 * - bowing jack - the assembly that holds the DC bowing motor and reflection sensor tachometer
 * - pressure - the position of the bowing jack as set by the stepper motor connected to the assembly
 * - string module - a complete assembly of functions connected to a single string, this includes pickups, solenoids, mutes, bows etc.
 *
 * @section brief Brief overview
 *
 * The code is structured primarily around a hierarchial class structure where the base objects is a vector of stringModule instances. This class handles all <em>command messages</em> surrounding a single string
 * and delegates data and commands to other classes down the line. The top level classes deals with direct hardware interfacing, the middle level takes care of the
 * logical control of these classes depending on the outcome of processed data and the bottom level deals with interpretation and handling of command messages.
 * Everything is tied together through the main function which periodically runs basic function calls which are not time-sensitive while time-critical functions are
 * called through periodic interrupts. Global messages are handled through the loop via the use of functions in maincommandhandler.cpp
 *
 * All outwards functionality can be accessed through command messages, these can be invoked through USB-Serial, RS232 and added to que by internal functions that may or may not be connected to other external hardware.
 * All incoming MIDI-messages are mapped to a editable string of command messages, this way complete freedom in midi-mapping is obtained.
 * A universal messaging system that is ignorant of the source of the control messages makes for a more transparent and uniform way of handling events, hardware control and data processing.
 * This also makes for a system where minimal code changes are required when doing modifications or introducing new functionaltiy and options.
 * The 'help'-command exposes existing commands, their arguments and a brief description.
 *
 * Classes that contain data that are to be saved into EEPROM implements a function with the name dumpData() that returns a string of commands with any applicable parameters containing the data to be saved.
 * On load, the command string can be directly added to que and thus executed as is to set the desired parameters.
 *
 * @section hardwareclasses Classes and header files with direct hardware access:
 * - servoStepper - library for handling stepper motor step/dir signals as well as homing switch control. Based on a positional approach like that of a classic RC servo,
 * includes speed and acceleration parameters. The class is normally used with periodic interrupt driven polling of the servoStepper::updatePosition() function but can be used in a
 * blocking manner by utilization of the servoStepper::completeTask() function
 * \n
 * - bowIO - handles bowing jack stepper motor position (through a servoStepper class instance), direct speed control of the DC bowing motor, tachometer interrupt calls and control of the
 * DC/DC converter that drives the DC motor driver. It also provides functionality for checking motor driver fault conditions and over-current/power measurements
 * through the current sensor connected to the DC/DC converter.
 * \n
 * - \ref mute - handles mute stepper motor position and provides functions for setting various mute states. Requires frequent polling of stepServoStepper->updatePosition() in order for the
 * stepper motor to continuously update its position, this is preferably done via interrupts.
 * \n
 * - \ref solenoid - provides variable force solenoid control through the utilization of PWM, engages the solenoid for a set period of time (in uS) after which it will automatically disengage,
 * Requires frequent polling of the solenoid::updateSolenoid() function in order to engage and disengage the solenoid in a timely manner.
 * \n
 * - stringModule - this class handles an entire string module with vectors of bowControl, \ref solenoid and \ref mute objects as well as bowIO, \ref calibrate and CalibrationData objects.
 * The stringModule instance parses all local command messages applicable and does the appropriate function calls.
 * \n
 * - controlReader - reads data from i2c ADC converter(s) and issues command messages associated with the given ADC channel at certain value-change conditions. This class is updated through
 * the controlReader::readData() function which is to to be called periodically. Due to the blocking nature of the ARM i2c library one needs to take care with how often controlReader::readData() is called.
 * \n
 * - \ref audioanalyze.h - samples audio data from a pin connected to the electromagnetic pickup and does basic DSP calculations on frequency and audio level
 *
 * @section controlclasses Intermediary data and sensor data processing classes and header files
 * - bowControl - Control class for high level interfacing with a bowIO object. This object contains the PID for stable bowing motor control, calcualtes motor set frequency from harmonic tables and
 * pressure engage, rest and free positiong. Parameters are limited by user set soft-limitis contained in the pointed to CalibrationData and BowActuators class instances
 * \n
 * - main.cpp - Main software starting point. Initializes various hardware, class instances, loads any eeprom settings and binds interrupts to functions. Also contains the main loop function as well as
 * various helper functions. This class has an array of stringModule class instances and is through maincommandhandler.cpp deciding which stringModule instance is currently being controlled and is
 * passing along any command messages that aren't recognized as global.
 * \n
 * - maincommandhandler.cpp - contains functions for processing of global commands sent to the internal command message que.
 * \n
 * - midi.cpp - contains functions for processing midi commands according to the current \ref configuration class. Not directly bound to hardware but used with callback function pointers by main.cpp
 * \n
 *
 * @section auxclasses Auxilary / Misc classes and header files
 * - BowActuators - contains handling of a vector of bowActuator classes which in turn contains soft-limit data for user-defined actuators
 * \n
 * - \ref calibrate - contains functions for finding soft-limits of the current actuator, utilizes pointeres to bowIO and bowControl class instances to perform calibration tests and monitor sensor outputs
 * \n
 * - \ref commandparser.h - contains functions and classes for parsing and storing command messages. The main classes are commandItem which contains a single command and arguments, and commandList which
 * contains a vector of commandItem instances.
 * \n
 * - \ref configuration - data storage class for midi mapped command strings
 * - debugprint.h - contains functions for message reporting over USB-Serial
 * \n
 * - eepromhelpers.cpp - contains functions for saving and loading EEPROM data
 * \n
 * - harmonicSeries.cpp and harmonicSeries.h - contains a vector of harmonicSeries instances which in turn contains the harmonic ratios used by functions in the bowControl class to set bowing frequencies
 * \n
 * - isrclasswrapper.cpp - wrapper enabling class-based functions to be called by interrupts
 * \n
 * - name.c - Teensy 4-specific class for setting USB device names
 * \n
 * - settingshandler.cpp - functions for saving EEPROM data
 * \n
 * - automaticversion.hpp - functions for autmatically creating a build version number at each compile
 *
 * @section libraries Libraries
 * - Adafruit_ADS1X15 - library for using the ADS1X15 ADC converters, used by the controlReader class
 * \n
 * - Adafruit_BusIO - library used by the Adafruit_ADS1X15 library
 * \n
 * - Audio - PJRC Teensy 4 Audio library, used by functions in \ref audioanalyze.h
 * \n
 * - EEPROM - Library for saving data into the Teensy 4 EEPROM, used by \ref eepromhelpers.cpp
 * \n
 * - SafeString - Partial use of its RS-232 functionality, used by \ref main.cpp and \ref debugprint.h
 * \n
 * - SD, SdFat, SerialFlash, SPI - Libraries required by the Teensy 4 Audio library
 * \n
 * - Teensy4 - The PJRC Teensy 4 library
 * \n
 * - Teensy_PWM - Library for better manipulation of the Teensy 4 PWM ports, used by the \ref bowIO and \ref solenoid classes
 * \n
 * - tinyexpr - Expression parser library used for parsing command message expressions in \ref commandList::parseCommandExpressions
 * \n
 * - TMC2209 - Library for the TMC2209 stepper motor driver IC, used by the \ref bowIO and \ref mute classes
 * \n
 * - Wire - Library for i2c communications, used by the controlReader class
 * \n
 *
 * @section author Author
 * - Created by Karl Ekdahl on 2023-09-03
 * - Modified by Karl Ekdahl on 2024-07-26
 *
 */

#define AUDIO_INTERFACE
//#define AUDIO_BLOCK_SAMPLES 2048

#include <WProgram.h>

#include "automaticversion.hpp"

#include "string.h"
#include <HardwareSerial.h>

#include <BufferedInput.h>
#include <BufferedOutput.h>
#include <PinFlasher.h>
#include <SafeString.h>
#include <SafeStringNameSpace.h>
#include <SafeStringNameSpaceEnd.h>
#include <SafeStringNameSpaceStart.h>
#include <SafeStringReader.h>
#include <SafeStringStream.h>
#include <SerialComs.h>
#include <loopTimer.h>
#include <millisDelay.h>

createBufferedOutput(ssOutput, 8192, DROP_UNTIL_EMPTY); ///< SafeString buffer creation
createSafeStringReader(ssReader, 8192, "\r\n");       ///< SafeString reader creation

#ifdef EFARSLAVE
createBufferedOutput(masterSerialOut, 8192, DROP_UNTIL_EMPTY); ///< SafeString buffer creation
createSafeStringReader(masterSerialRead, 8192, "\r\n");       ///< SafeString reader creation
#elif EFARMASTER
createBufferedOutput(slaveSerialOut1, 66, DROP_UNTIL_EMPTY); ///< SafeString buffer creation
createSafeStringReader(slaveSerialRead1, 256, "\r\n");       ///< SafeString reader creation
#endif

#define masterSlaveBaudRate 2000000

#include <algorithm> // for min/max
#define clamp(v,i,x) min(max(v,i),x)
#include <vector>

elapsedMicros testMeasurement;  ///< Used for various internal tests
bool testMeasurementOngoing = false;

#include "debugprint.h"

extern unsigned long _heap_start;
extern unsigned long _heap_end;
extern char *__brkval;

int freeram() {
  return (char *)&_heap_end - __brkval;
}

#include "harmonicSeries.cpp"

//#define currentEEPROMVersion 0x02
//#uint8_t readEEPROMVersion;
//uint16_t currentEEPROMOffset = 0;

String customStartupParameters = "";

#include "eepromhelpers.cpp"
#include "configuration.cpp"
int currentConfig = 0;
std::vector<configuration> configArray;

#include "commandparser.h"
commandList *commands;

#define masterRx 0
#define masterTx 1

#include "audioanalyze.h"

#include "bowio.cpp"
class CalibrationData;
#include "bowActuators.h"
class bowControl;
#include "bowcontrol.cpp"
#include "calibrate.cpp"
#include "bowActuators.cpp"
#include "solenoid.cpp"
#include "mute.cpp"
#include "mutecalibration.cpp"
#include "stringmodule.cpp"

std::vector <stringModule> stringModuleArray;

#include "midi.cpp"

#include "isrclasswrapper.cpp"

/*! \brietf Interrupt callback function for measuring bowing speed of string unit
 *
 *  fmPinStateChangedISR will be called whenever an Interrupt event happens on the pin associated. The pin associated is in turn connected to the reflection sensor output
 *  of a string unit. The function also includes the setting up of a timeout function in case of no bow movement as the interrupt will never be triggered in that case.
 */
/// Interrupt call

/// Interrupt callback function tied to the ~FAULT output of the motor driver of a string unit
//bool motorDriverFaultFlag = false;
void motorDriverFault() {
  //motorDriverFaultFlag = true;
  debugPrintln("Fault!", Error);
}


#include "controlReader.cpp"
controlReader *controlRead;

#include "settingshandler.cpp"

/*! \brief Initial setup function
 *
 */

void updateServoStepperPressure0() {
    if (stringModuleArray[0].bowIOArray[0].stepServoStepper != nullptr) {
        stringModuleArray[0].bowIOArray[0].stepServoStepper->updatePosition();
    }
}


void updateServoStepperMute0() {
    if (stringModuleArray[0].muteArray[0].stepServoStepper != nullptr) {
        stringModuleArray[0].muteArray[0].stepServoStepper->updatePosition();
    }
}

void setup() {
    Serial.begin(115200);
    ssOutput.connect(Serial);
    ssReader.connect(Serial);

    analogReadResolution(12);

    debugPrintln("RAM free " + String(freeram()), Command);

    configuration defaultConfig;
    configArray.push_back(defaultConfig);
    currentConfig = 0;

    pinMode(masterRx, INPUT);
    pinMode(masterTx, OUTPUT);

#ifdef EFARMASTER
    Serial1.begin(masterSlaveBaudRate); //2000000
    stringModuleArray[0].slaveSerialOut = &slaveSerialOut1;
    stringModuleArray[0].slaveSerialRead = &slaveSerialRead1;
    //stringModuleArray[0].bowControlArray[0].slaveSerialOut = &slaveSerialOut1;
    //stringModuleArray[0].bowControlArray[0].slaveSerialRead = &slaveSerialRead1;
    slaveSerialOut1.connect(Serial1);
    slaveSerialRead1.connect(Serial1);
#elif EFARSLAVE
/*    Serial1.begin(masterSlaveBaudRate); //2000000
    masterSerialOut.connect(Serial1);
    masterSerialRead.connect(Serial1);*/
#endif // EFARMASTER

    commands = new commandList();

#if EFARSLAVE
    stringModule _stringModule;
    stringModuleArray.push_back(_stringModule);

    int moduleIndex = stringModuleArray.size() - 1;

    // Using TCM2209 #1 (pin 22 is used instead of 6 due to broken teensy, change!!)
//    stringModuleArray[moduleIndex].addBow(2, 14, 15, 12, 23, 11, 4, 10, 9, &Serial2, 22, 5);
//    stringModuleArray[moduleIndex].addBow(2, 14, 15, 12, 23, 11, -1, 10, 9, &Serial2, 22, 5);
    stringModuleArray[moduleIndex].addBow(2, 14, 15, 12, 23, 11, -1, 10, 9, &Serial2, 6, 5);
      // Using TCM2209 #2
    //stringModuleArray[moduleIndex].addBow(2, 14, 15, 12, 23, 17, 19, 18, &Serial5, 13, 22);
    stringModuleArray[moduleIndex].bowIOArray[0].setSpeedPWM(0);
    stringModuleArray[moduleIndex].addSolenoid(3);
//    stringModuleArray[moduleIndex].addMute(17, 19, 18, &Serial5, 13);
    stringModuleArray[moduleIndex].addMute(-1, 5, 4, &Serial5, 13);

    tachoISR_assignInterrupt(stringModuleArray[moduleIndex].bowIOArray[0].reflectorInterruptPin, &stringModuleArray[moduleIndex].bowControlArray[0]);
    pidISR_assignInterrupt(&stringModuleArray[moduleIndex].bowControlArray[0]);
//    attachInterrupt(digitalPinToInterrupt(11), motorDriverFault, CHANGE);

//    stringModuleArray[moduleIndex].EEPROM_offset = currentEEPROMOffset;
//    debugPrintln("Loading string data from offset " + String(stringModuleArray[moduleIndex].EEPROM_offset), Debug);

    stringModuleArray[0].calibrationDataArray[0].firstTouchPressure = 0;
    stringModuleArray[0].calibrationDataArray[0].stallPressure = 55000;

    stringModuleArray[0].bowIOArray[0].enableBowPower();

    stringModuleArray[0].bowIOArray[0].stepServoStepper->stepIntervalCallback = &updateServoStepperPressure0;
    stringModuleArray[0].muteArray[0].stepServoStepper->stepIntervalCallback = &updateServoStepperMute0;
    stringModuleArray[0].bowIOArray[0].getTMC2209Info();
    stringModuleArray[0].muteArray[0].getTMC2209Info();
    if (stringModuleArray[0].bowIOArray[0].stepServoStepper->homing == servoStepper::UNHOMED) {
        stringModuleArray[0].bowIOArray[0].homeBow();
    }
    if (stringModuleArray[0].muteArray[0].stepServoStepper->homing == servoStepper::UNHOMED) {
        stringModuleArray[0].muteArray[0].homeMute();
    }


    float equalSeries[12] = { 1, 1.059463094, 1.122462048, 1.189207115, 1.25992105, 1.334839854, 1.414213562, 1.498307077, 1.587401052, 1.681792831, 1.781797436, 1.887748625 };
    float justSeries[12] = {1, 1.06667, 1.125, 1.2, 1.25, 1.3333, 1.40625, 1.5, 1.6, 1.66667, 1.8, 1.875 };
                        //C  C#       D      D#   E     F       F#       G    G#   A        A#   B
    stringModuleArray[0].bowControlArray[0].harmonicSeriesList.addHarmonicSeries(equalSeries);
    stringModuleArray[0].bowControlArray[0].harmonicSeriesList.addHarmonicSeries(justSeries);

    controlRead = new controlReader(17, 16);
    //controlRead->cvInputCommands[0] = "m:0,f:" + String(stringModuleArray[0].calibrationDataArray[0].fundamentalFrequency) + "*2^(1 / 15878 * value)";
    //controlRead->cvInputCommands[0] = "m:0,h:value/1323.166667";

    loadAllParams();
#endif
    initMidi();

    startAudioAnalyze();

    debugPrintln("Initialized", InfoRequest);
#ifdef EFARMASTER
    debugPrintln("This is a master", InfoRequest);
#elif EFARSLAVE
    debugPrintln("This is a slave", InfoRequest);
#else
#error "Must be master or slave"
#endif

    //String currentVersion(reinterpret_cast< char const* >(&completeVersion));
    currentFirmwareVersion = String(reinterpret_cast< char const* >(&completeVersion));
    debugPrintln("Current version is " + currentFirmwareVersion, debugPrintType::Debug);
    delay(100);
    commands->addCommands("bpr:1");
}

//bool fContinuous = false; ///< Variable for rolling status on/off
//bool freqReport = false;
//int freqReportChannel = 0;

int currentStringModule = 0;

#include "maincommandhandler.cpp"
/*! \brief Main loop function
 *
 *  - The flow of the main loop is as follows:
 *    -# Process serial buffer
 *    -# Process USB commands
 *    -# Add any serial commands to command list and execute all commands in list
 *    -# Process each strings update function
 */

// Variables for command execution interval
unsigned long previousTime = 0;
unsigned long currentTime = 0;
unsigned long commandUpadateInterval = 1000;
elapsedMillis updateRollingStatus;
elapsedMicros controlReaderInterval;
#define controlReadIntervalTime 50

void loop() {
    ssOutput.nextByteOut();
    usbMIDI.read();
    if (controlReaderInterval >= controlReadIntervalTime) {
        controlRead->readData();
        controlReaderInterval = 0;
    }
    MIDI.read();

#if EFARSLAVE
// commented out in order to use the master serial for HW MIDI
/*    masterSerialOut.nextByteOut();
    if (masterSerialRead.read()) {
        commands->addCommands(masterSerialRead.c_str());
//        debugPrintln("Master reception: " + String(masterSerialRead.c_str()), Debug);
    }*/
#elif EFARMASTER
    stringModuleArray[0].slaveSerialOut->nextByteOut();
#endif

#ifdef EFARSLAVE
    for (int i = 0; i < int(stringModuleArray.size()); i++) {
        stringModuleArray[i].updateString();

        for (int j=0; j<stringModuleArray[i].bowControlArray.size(); j++) {
            // If the motor is not running, ignore errors as it's likely that a motor simply isn't connected
            if (stringModuleArray[i].bowIOArray[j].getSpeedPWM() != 0) {
                if (stringModuleArray[i].bowIOArray[j].bowOverPowerFlag) {
                    debugPrintln("Bow over power!", debugPrintType::Error);
                    commands->addCommands(stringModuleArray[i].bowControlArray[j].commandsOverPowerCurrent);
                }
                if (stringModuleArray[i].bowControlArray[j].checkMotorFault()) {
                    debugPrintln("Bow motor fault!", debugPrintType::Error);
                    commands->addCommands(stringModuleArray[i].bowControlArray[j].commandsMotorFault);
                }
            }
        }
    }
#endif

    unsigned long currentTime = micros();
    if (currentTime - previousTime >= commandUpadateInterval) {
        previousTime = currentTime;
        processSerialCommands();
    }

    if (stringModuleArray[0].bowIOArray[0].stepServoStepper->intDrivenMsgFlag) {
        debugRaw(stringModuleArray[0].bowIOArray[0].stepServoStepper->intDrivenMsg);
        stringModuleArray[0].bowIOArray[0].stepServoStepper->intDrivenMsgFlag = false;
    }
}
