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
 * @file midi.h
 *
 * Processing various MIDI messages
 *
 * Current POSSIBLE behavior of MIDI messages \todo REVISE
 *  - NoteOn with velocity 0 sets speed only
 *  - NoteOn with velocity >0 sets speed and pressure
 *  - NoteOff disengage string if no notes present
 *  - Aftertouch <127 sets pressure only
 *  - Aftertouch 127 engages solenoid
 *  - Pitch bend sets frequency deviation
 *  - CC 64 / 0x40 sets sustain on/off
 *  - CC 16 engages solenoid
 *
*/

#ifndef MIDI_H
#define MIDI_H

#include <MIDI.h>

int noteCount = 0;         ///< The current amount of keys depressed
//bool sustain = true;       ///< State of sustain pedal
bool monoMode = true;
signed int pitchBend = 0;  ///< Current pit mod value
//uint8_t midiRxChannel = 0x7F;

/** messaging system begin **/
double dnote;
double dchannel;
double dvelocity;
double dnotecount;
double dpressure;
double dvalue;
double dpitch;
double dprogram;
#define userVariableMax 10
double duv[userVariableMax] = { 0,0,0,0,0,0,0,0,0,0 };

#define mapSize 12    // C  C# D  D# E  F  F# G  G# A  A# B
double dMap[mapSize] = { 1, 1, 0, 1, 2, 1, 0, 0, 1, 0, 1, 2 };

double mapKeys(double key) {
    int mapIndex = (((int) key) % 12);
    //debugPrintln("Map index " + String(mapIndex), Debug);
    if ((mapIndex < 0) || (mapIndex >= mapSize)) { return 0; }
    return dMap[mapIndex];
}

double expBool(double input) {
    if (input <= 0) { return 0; } else { return 1; };
}

double expIBool(double input) {
    if (input <= 0) { return 1; } else { return 0; };
}

double epDeadbandThreshold = 250;

double deadbando(double value) {
    double target = 0;
    //if ((value < (target + epDeadbandThreshold)) && (value > (target - epDeadbandThreshold))) { return target; } else { return value; };
    if ((value < (target + epDeadbandThreshold)) && (value > (target - epDeadbandThreshold))) { return target; } else {
        if (value > target) {
            return value - epDeadbandThreshold;
        } else {
            return value + epDeadbandThreshold;
        }

    };
}

double deadband(double value, double threshold) {
    double target = 0;
    //if ((value < (target + epDeadbandThreshold)) && (value > (target - epDeadbandThreshold))) { return target; } else { return value; };
    if ((value < (target + threshold)) && (value > (target - threshold))) { return target; } else {
        if (value > target) {
            return value - threshold;
        } else {
            return value + threshold;
        }

    };
}


double epZeroThreshold = 200;

double zeroThreshold(double value) {
    if (value < epZeroThreshold) {
        return 0;
    } else {
        return value;
    }
}

#define expFunctionCount 24
te_variable expFunctions[expFunctionCount] = {{ "channel", &dchannel } , { "note", &dnote }, { "velocity", &dvelocity }, { "notecount", &dnotecount},
    { "bool", (const void*) expBool, TE_FUNCTION1 }, { "ibool", (const void*) expIBool, TE_FUNCTION1 }, { "pressure", &dpressure}, { "value", &dvalue },
    { "mapkeys", (const void*) mapKeys, TE_FUNCTION1 }, { "pitch", &dpitch }, { "program", &dprogram }, { "deadbando", (const void*) deadbando, TE_FUNCTION1 },
    { "deadband", (const void*) deadband, TE_FUNCTION2 }, { "zerothreshold", (const void*) zeroThreshold, TE_FUNCTION1 },
    { "uv0", &duv[0]}, { "uv1", &duv[1]}, { "uv2", &duv[2]}, { "uv3", &duv[3]}, { "uv4", &duv[4]},
    { "uv5", &duv[5]}, { "uv6", &duv[6]}, { "uv7", &duv[7]}, { "uv8", &duv[8]}, { "uv9", &duv[9]} };

void updateLocalVariables() {
    dnotecount = noteCount;
}

void processLocalMessage(String *message) {
    updateLocalVariables();
    commands->addCommands(*message);
    commands->parseCommandExpressions(expFunctions, expFunctionCount);
}
/** messaging system end **/

/** note cue begin **/
struct noteMsg {
    char channel = -1;
    char note = -1;
    char velocity = -1;
};

std::vector<noteMsg> notesHeld;

int8_t findNote(byte channel, byte note) {
    for (int i = 0; i < int(notesHeld.size()); i++) {
        if ((notesHeld[i].note == note) && (notesHeld[i].channel == channel)) {
            return i;
        }
    }
    return -1;
}

bool removeNote(byte channel, byte note) {
    int8_t i = findNote(channel, note);
    if (i == -1) {
        debugPrintln("Couldn't find note " + String(note), Debug);
        return false;
    } else {
        notesHeld.erase(notesHeld.begin() + i);
        debugPrintln("Removing note " + String(note), Debug);
        return true;
    }
};

void addNote(byte channel, byte note, byte velocity) {
    noteMsg local;
    local.note = note;
    local.channel = channel;
    local.velocity = velocity;
    if (removeNote(channel, note)) {
        debugPrintln("Note added twice!!", Error);
    }
    notesHeld.push_back(local);
    debugPrintln("Adding note " + String(note) + " (note no " + String(notesHeld.size()) + ")", Debug);
};

/** messaging note cue end **/

void setNote() {
    debugPrintln("Set note", Debug);
    dchannel = notesHeld[notesHeld.size() - 1].channel;
    dnote = notesHeld[notesHeld.size() - 1].note;
    dvelocity = notesHeld[notesHeld.size() - 1].velocity;
    processLocalMessage(configArray[currentConfig].noteOn);
}

// Set all notes bottoms up so that the que doesn't need to know which message goes to which string but so that any existing notes are still held
void setNotes() {
    debugPrintln("Set notes", Debug);
    for (int i = 0; i < int(notesHeld.size()); i ++) {
        dchannel = notesHeld[i].channel;
        dnote = notesHeld[i].note;
        dvelocity = notesHeld[i].velocity;
        processLocalMessage(configArray[currentConfig].noteOn);
    }
}

/// Handle note off message
void OnNoteOff(byte channel, byte note, byte velocity) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    noteCount--;
    if (noteCount < 0) { noteCount = 0; }

    removeNote(channel, note);
    dchannel = channel; dnote = note; dvelocity = velocity;
    processLocalMessage(configArray[currentConfig].noteOff);

    if ((notesHeld.size() > 0) && (monoMode)) {
        setNotes();
    }

    debugPrintln("NoteOff with note " + String(note) + " at velocity " + String(velocity), USB);
}

/// Handles note on messages
void OnNoteOn(byte channel, byte note, byte velocity) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    debugPrintln("NoteOn with note " + String(note) + " at velocity " + String(velocity), USB);
    if (velocity == 0) {
        OnNoteOff(channel, note, velocity);
        return;
    }
    addNote(channel, note, velocity);

    setNote();

    noteCount++;
}

/// Handle poly aftertouch message
void OnAfterTouchPoly(byte channel, byte note, byte velocity) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    if (velocity == 127) {
        stringModuleArray[0].solenoidArray[0].solenoidEngage();
    }

    debugPrintln("PolyAT on channel " + String(channel) + " note " + String(note) + " value " + String(velocity), USB);
//  setPressure7bit(velocity);
}

/// Handle poly aftertouch message
void OnChannelAftertouch(byte channel, byte pressure) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    dchannel = channel; dpressure = pressure;
    processLocalMessage(configArray[currentConfig].channelAftertouch);

    debugPrintln("ChannelAT on channel " + String(channel) + " value " + String(pressure), USB);
}

/// handle control change message
void OnControlChange(byte channel, byte control, byte value) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    debugPrintln("Control change on channel " + String(channel) + " control " + String(control) + " value " + String(value), USB);

    for (int i = 0; i < int(configArray[currentConfig].controlChange.size()); i++) {
        if (configArray[currentConfig].controlChange[i].control == control) {
            dnote = 0; dvelocity = 0;
            dchannel = channel; dvalue = value;
            processLocalMessage(&configArray[currentConfig].controlChange[i].command);
        //debugPrintln(configArray[currentConfig].controlChange[i].command, Debug);
        }
    }
/*
    if (control == 64) {  // Sustain
        if (value == 0) {
            sustain = false;
        } else {
            sustain = true;
        }
        debugPrintln("Setting sustain to " + String(sustain), USB);
    } else*/
/*    if (control == 123) {  /// All notes off
        if (noteCount != 0) {
            debugPrintln("All notes not removed!", Error);
            noteCount = 0;

            dchannel = notesHeld[0].channel; dnote = notesHeld[0].note; dvelocity = notesHeld[0].velocity;
            processLocalMessage(configArray[currentConfig].noteOff);
            notesHeld.clear();
        }
    }*/
}

void midiAllNotesOff() {
    if (noteCount != 0) {
        debugPrintln("All notes not removed!", Error);
        noteCount = 0;

        dchannel = notesHeld[0].channel; dnote = notesHeld[0].note; dvelocity = notesHeld[0].velocity;
        processLocalMessage(configArray[currentConfig].noteOff);
        notesHeld.clear();
    }
    debugPrintln("All notes off", USB);
}

/// handle pitch bend message
void OnPitchBend(byte channel, int pitch) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    dchannel = channel;
    dpitch = pitch;
    processLocalMessage(configArray[currentConfig].pitchBend);
    //stringModuleArray[0].bowControlArray[0].setHarmonicShift(pitch * 2);
    debugPrintln("Pitch bend on channel " + String(channel) + " value " + String(pitch), USB);
}

void OnProgramChange(uint8_t channel, uint8_t program) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    dchannel = channel;
    dprogram = program;
    processLocalMessage(configArray[currentConfig].programChange);

    debugPrintln("Program change on channel " + String(channel) + " program " + String(program), USB);
}

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void initMidi() {
    usbMIDI.setHandleNoteOff(OnNoteOff);
    usbMIDI.setHandleNoteOn(OnNoteOn);
    usbMIDI.setHandleAfterTouchPoly(OnAfterTouchPoly);
    usbMIDI.setHandleControlChange(OnControlChange);
    usbMIDI.setHandlePitchChange(OnPitchBend);
    usbMIDI.setHandleAfterTouch(OnChannelAftertouch);
    usbMIDI.setHandleProgramChange(OnProgramChange);

    Serial1.begin(31250, SERIAL_8N1);
    MIDI.begin();
    MIDI.setHandleNoteOff(OnNoteOff);
    MIDI.setHandleNoteOn(OnNoteOn);
    MIDI.setHandleAfterTouchPoly(OnAfterTouchPoly);
    MIDI.setHandleControlChange(OnControlChange);
    MIDI.setHandlePitchBend(OnPitchBend);
    MIDI.setHandleAfterTouchChannel(OnChannelAftertouch);
    MIDI.setHandleProgramChange(OnProgramChange);

}
#endif
