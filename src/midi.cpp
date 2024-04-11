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

int noteCount = 0;         ///< The current amount of keys depressed
bool sustain = true;       ///< State of sustain pedal
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

#define expFunctionCount 21
te_variable expFunctions[expFunctionCount] = {{ "channel", &dchannel } , { "note", &dnote }, { "velocity", &dvelocity }, { "notecount", &dnotecount},
    { "bool", (const void*) expBool, TE_FUNCTION1 }, { "ibool", (const void*) expIBool, TE_FUNCTION1 }, { "pressure", &dpressure}, { "value", &dvalue },
    { "mapkeys", (const void*) mapKeys, TE_FUNCTION1 }, { "pitch", &dpitch }, { "program", &dprogram },
    { "uv0", &duv[0]}, { "uv1", &duv[1]}, { "uv2", &duv[2]}, { "uv3", &duv[3]}, { "uv4", &duv[4]},
    { "uv5", &duv[5]}, { "uv6", &duv[6]}, { "uv7", &duv[7]}, { "uv8", &duv[8]}, { "uv9", &duv[9]} };

void updateLocalVariables() {
    dnotecount = noteCount;
}

void processLocalMessage(String *message) {
    updateLocalVariables();
//    debugPrintln("processLocalMessage: " + *message, Debug);
    commands->addCommands(*message);
    //  commands->parseCommandExpressions(midiVariables, midiVariableCount);
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
/*    for (int i = 0; i < int(notesHeld.size()); i++) {
        if ((notesHeld[i].note == note) && (notesHeld[i].channel == channel)) {
            notesHeld.erase(notesHeld.begin() + i);
            debugPrintln("Removing note " + String(note), Debug);
            return;
        }
    }*/
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

/*
/// Sets the tilt using MIDI PRESSURE
/// \todo contains arbitrary values that need to be defined or otherwise manipulative
void setPressure7bit(int pressure) {
  if (pressure > 127) { pressure = 127; }
  if (pressure < 0) { pressure = 0; }

  unsigned int tiltPWM = stringModuleArray[0].calibrationDataArray[0].minUsablePressure - 4000 +
    ((double)(stringModuleArray[0].calibrationDataArray[0].maxUsablePressure - stringModuleArray[0].calibrationDataArray[0].minUsablePressure + 8000)
    / 65535 * ((double)(stringModuleArray[0].bowControlArray[0].manualTiltPWM / 127 * (pressure + 1))));
  stringModuleArray[0].bowIOArray[0].setTiltPWM(tiltPWM);
  debugPrintln("Setting pressure " + String(pressure) + " = PWM " + String(tiltPWM), USB);
}
*/

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

    debugPrintln("NoteOff", USB);
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
//  setPressure7bit(velocity);
}

/// Handle poly aftertouch message
void OnChannelAftertouch(byte channel, byte pressure) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    dchannel = channel; dpressure = pressure;
    processLocalMessage(configArray[currentConfig].channelAftertouch);
}

/// handle control change message
void OnControlChange(byte channel, byte control, byte value) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    debugPrintln("Control change " + String(control) + " with value " + String(value), USB);

    for (int i = 0; i < int(configArray[currentConfig].controlChange.size()); i++) {
        if (configArray[currentConfig].controlChange[i].control == control) {
            dnote = 0; dvelocity = 0;
            dchannel = channel; dvalue = value;
            processLocalMessage(&configArray[currentConfig].controlChange[i].command);
        //debugPrintln(configArray[currentConfig].controlChange[i].command, Debug);
        }
    }

    if (control == 64) {  // Sustain
        if (value == 0) {
            sustain = false;
        } else {
            sustain = true;
        }
        debugPrintln("Setting sustain to " + String(sustain), USB);
    } else
    if (control == 123) {  /// All notes off
        if (noteCount != 0) {
            debugPrintln("All notes not removed!", Error);
            noteCount = 0;

            dchannel = notesHeld[0].channel; dnote = notesHeld[0].note; dvelocity = notesHeld[0].velocity;
            processLocalMessage(configArray[currentConfig].noteOff);
            notesHeld.clear();
        }
    }
}

/// handle pitch bend message
void OnPitchBend(byte channel, int pitch) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    dchannel = channel;
    dpitch = pitch;
    processLocalMessage(configArray[currentConfig].pitchBend);
    //stringModuleArray[0].bowControlArray[0].setHarmonicShift(pitch * 2);
}

void OnProgramChange(uint8_t channel, uint8_t program) {
    if ((configArray[currentConfig].midiRxChannel != channel) && (configArray[currentConfig].midiRxChannel < 17) &&
        (configArray[currentConfig].midiRxChannel > 0)) { return; }

    dchannel = channel;
    dprogram = program;
    processLocalMessage(configArray[currentConfig].programChange);
}
#endif
