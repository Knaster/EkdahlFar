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

#ifndef MIDI_CPP
#define MIDI_CPP

#include <MIDI.h>
#include "midi.h"

#define MIDI_CONFIG_CHECK \
    if (midiMessageConfiguration == nullptr) { \
        debugPrintln("No configuration selected", debugPrintType::Error); \
        return; \
    }

MIDIHandler::MIDIHandler() {
}

/** messaging system begin **/

void MIDIHandler::updateLocalVariables() {
    expressionParser.dnotecount = noteCount;
}

void MIDIHandler::processLocalMessage(String *message) {
    updateLocalVariables();
    globalResponseCommands.addCommands(*message);
    globalResponseCommands.parseCommandExpressions(expressionParser);
}
/** messaging system end **/

/** note cue begin **/

int8_t MIDIHandler::findNote(byte channel, byte note) {
    for (int i = 0; i < int(notesHeld.size()); i++) {
        if ((notesHeld[i].note == note) && (notesHeld[i].channel == channel)) {
            return i;
        }
    }
    return -1;
}

bool MIDIHandler::removeNote(byte channel, byte note) {
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

void MIDIHandler::addNote(byte channel, byte note, byte velocity) {
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

void MIDIHandler::setNote() {
    debugPrintln("Set note", Debug);
    expressionParser.dchannel = notesHeld[notesHeld.size() - 1].channel;
    expressionParser.dnote = notesHeld[notesHeld.size() - 1].note;
    expressionParser.dvelocity = notesHeld[notesHeld.size() - 1].velocity;
    processLocalMessage(&midiMessageConfiguration->midiEventMap[midiMessageConfiguration->eMIDIEvent::ev_noteOn]);
}

// Set all notes bottoms up so that the que doesn't need to know which message goes to which string but so that any existing notes are still held
void MIDIHandler::setNotes() {
    debugPrintln("Set notes", Debug);
    for (int i = 0; i < int(notesHeld.size()); i ++) {
        expressionParser.dchannel = notesHeld[i].channel;
        expressionParser.dnote = notesHeld[i].note;
        expressionParser.dvelocity = notesHeld[i].velocity;
        processLocalMessage(&midiMessageConfiguration->midiEventMap[midiMessageConfiguration->eMIDIEvent::ev_noteOn]);
    }
}

/// Handle note off message
void MIDIHandler::OnNoteOff(byte channel, byte note, byte velocity) {
    MIDI_CONFIG_CHECK
    if ((midiMessageConfiguration->midiRxChannel != channel) && (midiMessageConfiguration->midiRxChannel < 17) &&
        (midiMessageConfiguration->midiRxChannel > 0)) { return; }

    noteCount--;
    if (noteCount < 0) { noteCount = 0; }

    removeNote(channel, note);
    expressionParser.dchannel = channel;
    expressionParser.dnote = note;
    expressionParser.dvelocity = velocity;
    processLocalMessage(&midiMessageConfiguration->midiEventMap[midiMessageConfiguration->eMIDIEvent::ev_noteOff]);

    if ((notesHeld.size() > 0) && (monoMode)) {
        setNotes();
    }

    debugPrintln("NoteOff with note " + String(note) + " at velocity " + String(velocity), USB);
}

/// Handles note on messages
void MIDIHandler::OnNoteOn(byte channel, byte note, byte velocity) {
    MIDI_CONFIG_CHECK

    if ((midiMessageConfiguration->midiRxChannel != channel) && (midiMessageConfiguration->midiRxChannel < 17) &&
        (midiMessageConfiguration->midiRxChannel > 0)) { return; }

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
void MIDIHandler::OnAfterTouchPoly(byte channel, byte note, byte pressure) {
    MIDI_CONFIG_CHECK

    if ((midiMessageConfiguration->midiRxChannel != channel) && (midiMessageConfiguration->midiRxChannel < 17) &&
        (midiMessageConfiguration->midiRxChannel > 0)) { return; }

    expressionParser.dchannel = channel;
    expressionParser.dpressure = pressure;
    expressionParser.dnote = note;

    processLocalMessage(&midiMessageConfiguration->midiEventMap[midiMessageConfiguration->eMIDIEvent::ev_polyAfterTouch]);

    debugPrintln("PolyAT on channel " + String(channel) + " note " + String(note) + " value " + String(pressure), USB);
}

/// Handle poly aftertouch message
void MIDIHandler::OnChannelAftertouch(byte channel, byte pressure) {
    MIDI_CONFIG_CHECK

    if ((midiMessageConfiguration->midiRxChannel != channel) && (midiMessageConfiguration->midiRxChannel < 17) &&
        (midiMessageConfiguration->midiRxChannel > 0)) { return; }

    expressionParser.dchannel = channel;
    expressionParser.dpressure = pressure;

    processLocalMessage(&midiMessageConfiguration->midiEventMap[midiMessageConfiguration->eMIDIEvent::ev_channelAftertouch]);

    debugPrintln("ChannelAT on channel " + String(channel) + " value " + String(pressure), USB);
}

/// handle control change message
void MIDIHandler::OnControlChange(byte channel, byte control, byte value) {
    MIDI_CONFIG_CHECK
    if ((midiMessageConfiguration->midiRxChannel != channel) && (midiMessageConfiguration->midiRxChannel < 17) &&
        (midiMessageConfiguration->midiRxChannel > 0)) { return; }

    debugPrintln("Control change on channel " + String(channel) + " control " + String(control) + " value " + String(value), USB);

    for (int i = 0; i < int(midiMessageConfiguration->controlChange.size()); i++) {
        if (midiMessageConfiguration->controlChange[i].control == control) {
            expressionParser.dnote = 0;
            expressionParser.dvelocity = 0;
            expressionParser.dchannel = channel;
            expressionParser.dvalue = value;
            processLocalMessage(&midiMessageConfiguration->controlChange[i].command);
        }
    }
}

void MIDIHandler::midiAllNotesOff() {
    MIDI_CONFIG_CHECK

    if (noteCount != 0) {
        debugPrintln("All notes not removed!", Error);
        noteCount = 0;

        expressionParser.dchannel = notesHeld[0].channel;
        expressionParser.dnote = notesHeld[0].note;
        expressionParser.dvelocity = notesHeld[0].velocity;

        processLocalMessage(&midiMessageConfiguration->midiEventMap[midiMessageConfiguration->eMIDIEvent::ev_noteOff]);
        notesHeld.clear();
    }
    debugPrintln("All notes off", USB);
}

/// handle pitch bend message
void MIDIHandler::OnPitchBend(byte channel, int pitch) {
    MIDI_CONFIG_CHECK

    if ((midiMessageConfiguration->midiRxChannel != channel) && (midiMessageConfiguration->midiRxChannel < 17) &&
        (midiMessageConfiguration->midiRxChannel > 0)) { return; }

    expressionParser.dchannel = channel;
    expressionParser.dpitch = pitch;

    processLocalMessage(&midiMessageConfiguration->midiEventMap[midiMessageConfiguration->eMIDIEvent::ev_PitchBend]);
    debugPrintln("Pitch bend on channel " + String(channel) + " value " + String(pitch), USB);
}

void MIDIHandler::OnProgramChange(uint8_t channel, uint8_t program) {
    MIDI_CONFIG_CHECK

    if ((midiMessageConfiguration->midiRxChannel != channel) && (midiMessageConfiguration->midiRxChannel < 17) &&
        (midiMessageConfiguration->midiRxChannel > 0)) { return; }

    expressionParser.dchannel = channel;
    expressionParser.dprogram = program;

    processLocalMessage(&midiMessageConfiguration->midiEventMap[midiMessageConfiguration->eMIDIEvent::ev_programChange]);
    debugPrintln("Program change on channel " + String(channel) + " program " + String(program), USB);
}
#endif
