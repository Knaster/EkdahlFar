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
#ifndef STRINGMODULE_HPP
#define STRINGMODULE_HPP

#include "harmonicserieshandler.cpp"

serialCommandItem serialCommandsStringModule[] = {
    { "bow", "b", "0-15", "Sets the currently selected bow"},
    { "solenoid", "so", "int", "Sets the currently selected solenoid" },

//    { "bowpid", "bpid", "1|0", "Sets the bow PID on/off" },
/*    { "bowpidki", "bpki", "float", "Sets the Ki parameter of the PID of the selected bow" },
    { "bowpidkp", "bpkp", "float", "Sets the Kp parameter of the PID of the selected bow" },
    { "bowpidkd", "bpkd", "float", "Sets the Kd parameter of the PID of the selected bow" },
    { "bowpidintegratorerror", "bpie", "float", "Sets the lower threshold of error values for the PID integrator to ignore of the selected bow" },
    { "bowpidr", "bpir", "-", "Resets the PID of the selected bow" },
    { "bowpidmaxerror", "bpme", "float", "Maximum error to correct in each PID loop, essentially sets acceleration" },
    { "bowpidpeakerror", "bpperr", "float", "Get latest PID peak error"},
*//*
    { "bowcontrolfrequency", "bcf", "float", "Bow motor speed frequency in hertz, requires that PID is turned on" },
    { "bowcontrolfundamental", "bcu", "float", "Bow fundamental frequency, all harmonics are calculated from this number" },
    { "bowcontrolharmonic", "bch", "int", "Bow motor speed in terms of a harmonic number. A ratio is taken from the given harmonic in the current harmonic list, the ratio is then multiplied by the bow fundamental frequency" },
    { "bowcontrolharmonicadd", "bcha", "int", "Additative version of bowcontrolharmonic, the number is given is added to the harmonic given" },
    { "bowcontrolharmonicbase", "bchb", "int", "Same as bowcontrolharmonic but where the harmonic number is based on a MIDI note given by bowcontrolbasenote" },
    { "bowcontrolharmonicbasenote", "bchbn", "0-127", "Sets the MIDI base note of the string, used in conjunction with bowcontrolharmonicbase"},
    { "bowcontrolharmonicshift", "bchsh", "-32767-32767", "Setting shift from the currently playing harmonic where 32767 equals the entire harmonic shift range shifted up" },
    { "bowcontrolharmonicshiftrange", "bchsr", "0-36", "Set the number of harmonic numbers that constitutes the entire harmonic shift" },
    { "bowcontrolharmonicshift5", "bchs5", "-32767-32767", "Setting shift from the currently playing harmonic over 5 octaves where 32767 equals 5 octaves shift up from the fundamental" },
    { "bowcontrolspeedmode", "bcsm", "0|1", "Bow motor speed mode, 0 = Automatic and 1 = Manual" },

    { "bowharmonicseries", "bhs", "int", "Get/set the current harmonic series" },
    { "bowharmonicseriesdata", "bhsd", "slot:name:ratios", "Sets all data for the harmonic series in the given slot" },
    { "bowharmonicseriesratio", "bhsr", "harmonic:ratio", "Sets the ratio of the given harmonic in current harmonic series, will increase the list size if needed to adress the harmonic" },
    { "bowharmonicseriesratioremove", "bhsrr", "ratio", "Remove the harmmonic ratio given in the current series and shift any ratios accordingly. Cannot remove all ratios"},
    { "bowharmonicseriescount", "bhsc", "-", "Returns the number of harmonic series in the list and their IDs"},
    { "bowharmonicseriessave", "bhss", "series:name", "Saves the current harmonic series in the slot given, or if the slot is out of range, creates a new one at the end of the list"},
    { "bowharmonicseriesremove", "bhsrm", "series", "Remove the series given and shift any series accordingly. Cannot remove all series"},
*/
    { "bowstatus", "bs", "-", "Prints status information for the selected bow" },
    { "pickupstringfrequency", "psf", "-", "Returns the fundamental tone calculated from the current audio signal if appliccable"},
    { "pickupaudiopeak", "pap", "-", "Returns the peak amplitude of the current audio signal"},
    { "pickupaudiorms", "par", "-", "Returns the RMS amplitude of the current audio signal"},
/*    { "bowdebugmeasuretimetotarget", "bdmtt", "float", "Measure the time it takes to change from the current frequency to the target frequency" },
    { "bowhome", "bh", "-", "Homing bow, used at startup and in case of the bow loosing position" },
*/
    { "bowcalibrateall", "bca", "-", "Performs all calibration routines on the selected bow, see below for routines performed" },
    { "bowcalibratespeed", "bcs", "-", "Finds the minimum and maximum bow speed of the selected bow" },
    { "bowcalibratepressure", "bcp", "-", "Finds the minimum and maximum bow pressure of the selected bow" },

//    { "bowmotorrun", "bmr", "1|0", "Set bow motor run on/off" },
//    { "bowmotordirectpwm", "bmdp", "0-65535", "Bow motor direct power in 16-bit PWM values, requires that the PID is turned off" },
//    { "bowmotortimeout", "bmt", "ms(0-65535)", "Bow motor shutdown timeout after bow having been put into the rest position" },
/*    { "bowmotorspeedmax", "bmsx", "0-65535", "Bow motor maximum speed limit" },
    { "bowmotorspeedmin", "bmsi", "0-65535", "Bow motor minimum speed limit"},
    { "bowmotorvoltage", "bmv", "1.2-9", "Bow motor voltage"},
    { "bowmotorfrequency", "bmf", "-", "Bow motor reported frequency"},
    { "bowmotorcurrent", "bmc", "0-3", "Bow motor reported current use" },
    { "bowmotorcurrentlimit", "bmcl", "0-3", "Bow motor current limit (A)- !WARNING! Can ruin your instrument if changed" },
    { "bowmotorpowerlimit", "bmpl", "0-255", "Bow motor power limit (W) - !WARNING! Can ruin your instrument if changed" },
    { "bowmotoremergencystop", "bmes", "ms (0-65535)", "Immediately stops the bowing motor and doesn't allow it to start again until the cool down period given in the first argument has lapsed (milliseconds)"},
*/
//    { "bowmotorfaultcommands", "bmfc", "command list", "Commands to execute when a motor fault is tripped - !WARNING! Can ruin your instrument if changed" },
//    { "bowmotoroverpowercommands", "bmopc", "command list", "Commands to execute when motor is over the power limit - !WARNING! Can ruin your instrument if changed" },

/*
    { "bowpressurebaseline", "bpb", "0-65535", "Bow pressure baseline, modulation is added to this point upward" },
    { "bowpressuremodifier", "bpm", "0-65535", "Bow pressure modulation, added to the baseline" },
    { "bowpressurerest", "bpr", "0|1", "Puts the bow pressure in the resting position (conditional)" },
    { "bowpressureengage", "bpe", "0|1", "Puts the bow pressure in the engage position (conditional)" },
    { "bowpressurepositionmax", "bppx", "0-65535", "Bow pressure stall/maximum position"},
    { "bowpressurepositionengage", "bppe", "0-65535", "Bow pressure touch/minimum position"},
    { "bowpressurepositionrest", "bppr", "0-65535", "Bow pressure rest position"},
    { "bowpressureengagespeed", "bpes", "1 - 100?", "Bow pressure movement speed when engaging or disengaging"},
    { "bowpressuremodulationspeed", "bpms", "0.1 - 10", "Bow pressure movement speed while engaged"},
    { "bowpressurehold", "bph", "0|1", "Sets bow hold on/off" },
*/
    { "mutecalibrate", "mca", "-", "Calibrate mute settings"},

 //   { "nooperation", "nop", "-", "Do absolutely, positively, nothing"}
};

class stringModule {
    public:
    std::vector<bowIO> bowIOArray;
    std::vector<bowControl> bowControlArray;
    std::vector<CalibrationData> calibrationDataArray;
    std::vector<calibrate> calibrateArray;

    std::vector<Solenoid> solenoidArray;
    std::vector<BowControl> bowControlArrayX;
    std::vector<MuteControl> muteArray;
    std::vector<calibrateMute> calibrateMuteArray;

    void frequencyChanged();
    HarmonicSeriesHandler *harmonicSeriesHandler;

//    int EEPROM_offset = 0;

    int currentBowSerial = 0;
    int currentSolenoidSerial = 0;

    BufferedOutput *slaveSerialOut = nullptr;
    SafeStringReader *slaveSerialRead = nullptr;

    stringModule();
    void updateString();
    bool addBow(char motorRevPin, char motorVoltagePin, char motorDCDCEn, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin,
                char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin, char stepCorrectionSensorPin);
    bool addSolenoid(char _solenoidPin);
    void writeToSlave(String command);
    bool addMute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin);

    bool processSerialCommand_GeneralControl(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
                                             commandList *delegatedCommands = nullptr);
    bool processSerialCommand_CalibrationsSettings(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
                                                   commandList *delegatedCommands = nullptr);
/*    bool processSerialCommand_MuteControl(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
                                          commandList *delegatedCommands = nullptr);*/
    bool processSerialCommand_StatusTesting(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
                                            commandList *delegatedCommands = nullptr);
    bool processSerialCommand_BowActuator(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
                                          commandList *delegatedCommands = nullptr);
    bool processSerialCommand_Calibrate(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
                                        commandList *delegatedCommands = nullptr);
    bool processRequestCommand(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool delegated);
    bool processSerialCommand(commandList *commands, uint16_t *index, std::vector<commandResponse> *commandResponses, bool delegated);
};
#endif
