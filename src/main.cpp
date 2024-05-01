/**
 * @file teensyfarcode.ino
 *
 * @mainpage Ekdahl FAR main file
 *
 * @section description Description
 * Code for using and controlling the Ekdahl FAR and one or more strings with their associated hardware
 *
 * @section libraries Libraries
 * - SafeString
 *
 * @section classes Internal classes and files
 * - stringModule
 * - bowIO
 * - bowControl
 * - solenoid
 * - calibrate
 * - commandItem
 * - commandList
 * - configuration
 *
 * @section goals Goals
 *
 *  Write down flow from MIDI input to hardware output including all pressure modifiers etc.\n
 *  Save only data not in default
 *  Replace keymap with per-key commands, pre/post?\n
 *  Make global sustain variable\n
 *  Ignore out of bounds references for strings, bows, solenoids etc.\n
 *  Edit map commands\n (??)
 *  DONE, TEST - Implement all midi events\n
 *  DONE - TEST - Implement all functions needed for configuration switching, adding and removing\n
 *  DONE - Implement bow current limit\n
 *  DONE - Check bow current limit in pressure calibration routine\n
 *  Implement current-limited forward/backwards homing
 *  DONE - Implement "set harmonic shift" for pitch bend
 *  DONE - Implement sustain
 *  DONE - Make bow hold go over any previously held notes and turn them off
 *  Double check that stepper speeds are good / no missed steps
 *  Change ADC I2C speed to 2MHz
 *  Create individual averaging per input
 *
 * @section author Author
 * - Created by Karl Ekdahl on 2023-09-03
 * - Modified by Karl Ekdahl on 2023-12-18
 *
 */

#define AUDIO_INTERFACE
//#define AUDIO_BLOCK_SAMPLES 2048

#include <WProgram.h>

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

createBufferedOutput(ssOutput, 66, DROP_UNTIL_EMPTY); ///< SafeString buffer creation
createSafeStringReader(ssReader, 4096, "\r\n");       ///< SafeString reader creation

#ifdef EFARSLAVE
createBufferedOutput(masterSerialOut, 66, DROP_UNTIL_EMPTY); ///< SafeString buffer creation
createSafeStringReader(masterSerialRead, 4096, "\r\n");       ///< SafeString reader creation
#elif EFARMASTER
createBufferedOutput(slaveSerialOut1, 66, DROP_UNTIL_EMPTY); ///< SafeString buffer creation
createSafeStringReader(slaveSerialRead1, 256, "\r\n");       ///< SafeString reader creation
#endif

#define masterSlaveBaudRate 2000000

#include <algorithm> // for min/max
#define clamp(v,i,x) min(max(v,i),x)
#include <vector>

#include "debugprint.h"

extern unsigned long _heap_start;
extern unsigned long _heap_end;
extern char *__brkval;

int freeram() {
  return (char *)&_heap_end - __brkval;
}

#include "harmonicSeries.cpp"

#define currentEEPROMVersion 0x02
uint8_t readEEPROMVersion;
uint16_t currentEEPROMOffset = 0;
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
class _calibrationData;
#include "bowActuators.h"
class bowControl;
#include "bowcontrol.cpp"
#include "calibrate.cpp"
#include "bowActuators.cpp"
#include "solenoid.cpp"
#include "mute.cpp"
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
    Serial1.begin(masterSlaveBaudRate); //2000000
    masterSerialOut.connect(Serial1);
    masterSerialRead.connect(Serial1);
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

    stringModuleArray[moduleIndex].EEPROM_offset = currentEEPROMOffset;
    debugPrintln("Loading string data from offset " + String(stringModuleArray[moduleIndex].EEPROM_offset), Debug);

    stringModuleArray[0].calibrationDataArray[0].firstTouchPressure = 0;
    stringModuleArray[0].calibrationDataArray[0].stallPressure = 55000;

    stringModuleArray[0].bowIOArray[0].enableBowPower();

    stringModuleArray[0].bowIOArray[0].stepServoStepper->stepIntervalCallback = &updateServoStepperPressure0;
    stringModuleArray[0].muteArray[0].stepServoStepper->stepIntervalCallback = &updateServoStepperMute0;
//    stringModuleArray[0].bowIOArray[0].getTMC2209Info();
//    stringModuleArray[0].muteArray[0].getTMC2209Info();


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
    usbMIDI.setHandleNoteOff(OnNoteOff);
    usbMIDI.setHandleNoteOn(OnNoteOn);
    usbMIDI.setHandleAfterTouchPoly(OnAfterTouchPoly);
    usbMIDI.setHandleControlChange(OnControlChange);
    usbMIDI.setHandlePitchChange(OnPitchBend);
    usbMIDI.setHandleAfterTouch(OnChannelAftertouch);
    usbMIDI.setHandleProgramChange(OnProgramChange);

    startAudioAnalyze();

    debugPrintln("Initialized", InfoRequest);
#ifdef EFARMASTER
    debugPrintln("This is a master", InfoRequest);
#elif EFARSLAVE
    debugPrintln("This is a slave", InfoRequest);
#else
#error "Must be master or slave"
#endif
}

bool fContinuous = false; ///< Variable for rolling status on/off
bool freqReport = false;
int freqReportChannel = 0;

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

unsigned long previousTime = 0;
unsigned long currentTime = 0;
unsigned long commandUpadateInterval = 1000;

elapsedMillis updateRollingStatus;
//elapsedMillis updateStrings;
elapsedMillis elTest;

elapsedMicros controlReaderInterval;
#define controlReadIntervalTime 500

void loop() {
    ssOutput.nextByteOut();
    usbMIDI.read();
    if (controlReaderInterval >= controlReadIntervalTime) {
        controlRead->readData();
        controlReaderInterval = 0;
    }

#if EFARSLAVE
    masterSerialOut.nextByteOut();
    if (masterSerialRead.read()) {
        commands->addCommands(masterSerialRead.c_str());
//        debugPrintln("Master reception: " + String(masterSerialRead.c_str()), Debug);
    }
#elif EFARMASTER
    stringModuleArray[0].slaveSerialOut->nextByteOut();
#endif

    unsigned long currentTime = micros();
    if (currentTime - previousTime >= commandUpadateInterval) {
        previousTime = currentTime;
        processSerialCommands();
    }

    //if (Serial1.available()) {
#ifdef EFARSLAVE
    for (int i = 0; i < int(stringModuleArray.size()); i++) {
        stringModuleArray[i].updateString();
        if (stringModuleArray[i].bowIOArray[0].bowOverPowerFlag) {
            stringModuleArray[i].bowIOArray[0].tiltAdjust += 1000;
//            debugPrintln("Bow over power! Adjusting down", debugPrintType::Error);
        }
    }
#endif

    if (fContinuous) {
        if (updateRollingStatus >= 100) {
            pinMode(23, INPUT);
            float avFreq = stringModuleArray[0].bowIOArray[0].averageFreq();
            float targetFreq = stringModuleArray[0].bowControlArray[0].getPIDTarget();
            float freqError = (avFreq / targetFreq * 100) - 100;
            float current = stringModuleArray[0].bowIOArray[0].getBowCurrent();
            bool fault = stringModuleArray[0].bowIOArray[0].getMotorFault();
            debugPrintln("Read frequency " + String(avFreq) + " Hz, Target freq " + String(targetFreq) + " Hz" +
                ", error %" + String(freqError) + " current use " + String(current) + " Fault " + String(fault)
                , InfoRequest);
            updateRollingStatus = 0;
        }
    }

    if (freqReport) {
        //    setAMUXAddress(freqReportChannel*2);
        if (audioFrequencyAvaliable()) {
                debugPrint("Frequency " + String(audioFrequency(),1) + " acc " + String(audioProbability()) + " @ " + String(audioProcessorUsage()) + " CPU ", InfoRequest);
            if (audioPeak->available()) {
                debugPrintln("Peak " + String(audioPeak->read()), InfoRequest);
            } else {
                debugPrintln("", InfoRequest);
            }
        }
    }
}
