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
 * The Ekdahl FAR is structured around a hierarchal concept of \ref "Module"modules that handles specific parts of the instrument, whether internal or external, software or hardware related.
 * All \a modules have a \ref "ModuleCommandDeclaration" "module command declaration" which shows what \a commands the module has and some basic information about the \a commands.
 * This information is used both internally to call the associated functions as well as offering an interface for users or connected software to discover the capabilities of each \a module.
 *
 * The subclass \ref "ModuleHandler" "module handler" allows a module to contain other modules using \ref "ModuleGroup" "module groups". Each \a module \a group can contain any number of
 * instances of single type of \ref Module or \ref "ModuleHandler" "module handler" class.
 * Addressing of \a modules is done in a hierarchal manner using the full names of all preceding modules separated with a '.'. Individual children of \a module \a groups are optionally
 * addressed using \a indexing through brackets '[]' and accepts both singles ('[2]'), ranges ('[0-2]') and comma separated instances ('[0-1,3]').
 *
 * Addressing a child with an \a index but without calling a \a command in that child creates a \a selection, these \a selections are used by owning \a modules in order to do things like
 * changing MIDI configuration or harmonic table. A \a selection can also be used by later \a commands in order to invoke functions on the currently selected child by omitting the brackets
 * when addressing. If omitting the brackets and there is no previous selection, the first ([0]) child is addressed.
 *
 * The main \ref loop function periodically runs basic function calls which are not time-sensitive while time-critical functions are called through periodic interrupts.
 * All messages are processed by their respective \a modules with the \ref "MasterModule" "master module" offering a few system functions,
 * the root \a module \a handler passed when creating the \a master \a module contains commands specific to the current setup.
 * Certain \a commands are offered by ALL modules and are declared in Module::builtinCommands, as of this writing these are: \n \n
 * list - returns all \a commands and child \a modules within the \a module \n
 * help - returns detailed information about each child \a module and \a command, for use both by direct users and software \n
 * dump - returns all data currently associated with each \a command and child \a module, used by software and internally to store parameters \n \n
 *
 * All \a commands can be invoked through USB-Serial, RS232 and added to queue by internal functions that may or may not be connected to other external hardware.
 * All incoming MIDI-messages are mapped to a editable string of \a command messages, this way complete freedom in midi-mapping is obtained.
 * A universal messaging system that is ignorant of the source of the \a command messages makes for a more transparent and uniform way of handling events, hardware control and data processing.
 * This also makes for a system where minimal code changes are required when doing modifications or introducing new functionality and options.
 *
 * @cond
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
 * @endcond
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

#include <base/arduinorequired.hpp>

#include "master_controller/automaticversion.hpp"

#include "string.h"
#include <HardwareSerial.h>
#include <BufferedInput.h>
#include <BufferedOutput.h>
#include <SafeString.h>
#include <SafeStringReader.h>

//String customStartupParameters = "";
//8192
createSafeStringReader(ssReader, 28192, "\r\n");       ///< SafeString reader creation
createBufferedOutput(ssOutput, 28192, DROP_UNTIL_EMPTY); ///< SafeString buffer creation

#ifdef USE_USART_AS_EXT
createSafeStringReader(ssExternalInput, 8192, "\r\n");
createBufferedOutput(ssExternalOutput, 8192, DROP_UNTIL_EMPTY);
#endif

#include "../../src/base/debugprint.cpp"

#include "base/commandparser.hpp"
#include "base/module.hpp"
#include "single_far/farsingle.hpp"
FAR_SINGLE_CREATE_INSTANCE(farSingle)
#include "master_controller/mastermodule.hpp"
MasterModule *masterModule;
#include "master_controller/global_generics.hpp"

//#ifdef GLOBALS
CommandList globalCommands;
CommandList globalResponseCommands;
//#endif

void processSerialCommands(CommandList inCommandList, bool isInternal = false) {
    if (inCommandList.item.size() == 0) { return; }
    std::vector <commandResponse> commandResponses;
    while (inCommandList.item.size() > 0) {
        processCommandList(masterModule, &inCommandList, &commandResponses);
        inCommandList.item.clear();
        inCommandList = globalResponseCommands;
        globalResponseCommands.item.clear();
    }
    //printResponses(&commandResponses, isInternal);
    if ((!isInternal) || debugPrintEnabled[debugPrintType::Internal]) {
        for (int i = 0; i < commandResponses.size(); i++) {
            if (!isInternal) {
                debugPrintln(commandResponses[i].response, commandResponses[i].responseType);
            } else {
                debugPrintln(commandResponses[i].response, debugPrintType::Internal);
            }
        }
    }
}

void processSerialCommands() {
    globalCommands = globalResponseCommands;
    if (ssReader.read()) { globalCommands.addCommands(ssReader.c_str()); }
#ifdef USE_USART_AS_EXT
    if (ssExternalInput.read()) { globalCommands.addCommands(ssExternalInput.c_str()); }
#endif // USE_USART_AS_EXT

    processSerialCommands(globalCommands, false);
    globalCommands.item.clear();
}

uint32_t startupTime;
bool startupReached = false;
#define startupTimeout 250

void setup() {
    Serial.begin(115200);
    ssOutput.connect(Serial);
    ssReader.connect(Serial);

    #ifdef USE_USART_AS_EXT
    Serial1.begin(115200);
    ssExternalInput.connect(Serial1);
    ssExternalOutput.connect(Serial1);
    #endif // USE_USART_AS_EXT

    analogReadResolution(12);

    debugPrintln("RAM free " + String(freeram()), Command);

    farSingle = new FARSingle(&farSingleUpdateServoStepperMute0, &farSingleUpdateServoStepperPressure0, &farSingleUpdateTachometer0, &farSingleUpdatePID0);

    masterModule = new MasterModule(farSingle, farSingle->expressionParser);
    masterModule->loadAllParameters();
    masterModule->init();
    farSingle->initFAR();

    debugPrintln("Initialized", InfoRequest);

    masterModule->version();
    startupTime = millis();
}

int currentStringModule = 0;

//#include "maincommandhandler.cpp"
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

unsigned long previousAlive = 0;
unsigned long currentAlive = 0;
unsigned long aliveUpdateInterval = 500;

#include "MIDI.h"
extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDI;

void loop() {
    if (!startupReached) {
        if (millis() > (startupTime + startupTimeout) && (!startupReached)) {
            startupReached = true;
        }
    } else {
        outputNext();
        usbMIDI.read();
        if (controlReaderInterval >= controlReadIntervalTime) {
            std::vector<commandResponse> inCommandResponse;
            farSingle->updateControlReader(&inCommandResponse);

            for (int i=0; i<inCommandResponse.size(); i++) {
                debugPrintln("cb." + inCommandResponse[i].response, inCommandResponse[i].responseType);
            }
            controlReaderInterval = 0;
        }
#ifdef USE_USART_AS_MIDI
        MIDI.read();
#endif
    }

    masterModule->update();
    processSerialCommands(globalResponseCommands, true);

    unsigned long currentTime = micros();
    if (currentTime - previousTime >= commandUpadateInterval) {
        previousTime = currentTime;
        processSerialCommands();
    }

/*
    currentAlive = millis();
    if (currentAlive - previousAlive >= aliveUpdateInterval) {
        previousAlive = currentAlive;
        debugPrintln("Alive", debugPrintType::TextInfo);
        Serial.println("hello");
    }*/
}
