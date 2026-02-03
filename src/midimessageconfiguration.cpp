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
#ifndef MIDIMESSAGECONFIGURATION_CPP
#define MIDIMESSAGECONFIGURATION_CPP

#include <WProgram.h>
#include "avr_functions.h"
#include "midimessageconfiguration.hpp"

const ModuleCommandDeclaration MIDIMessageConfiguration::moduleCommands[] = {
    { "", "", "int", "Sets the current MIDI configuration", false, false, nullptr, eCommandType::Index },
    { "name", "na", "string", "Set the name of the MIDI configuration (for request, argument is index of configuration to return name for (optional))", false, true, &s_namef, eCommandType::Name },
    { "data", "da", "noteon|noteoff|pat|cc:(0-127)|cat|pb|pc", "Set the MIDI event handling data, the event given will execute the given command string", false, true, &s_eventHandler, eCommandType::OutputAssignment },
    { "addcc", "ac", "cc(0-127):command string", "Add continuous controller and the command string to execute", false, false, &s_addcc, eCommandType::Data },
    { "removecc", "rmc", "cc(0-127)", "remove continuous controller from list", false, false, &s_ccRemove, eCommandType::Remove },
    { "defaults", "d", "-", "Reverts the current configuration to default values and CCs", false, false, &s_defaults, eCommandType::Immediate },
    { "receivechannel", "rc", "-", "Sets the MIDI receive channel of the current configuration. 1-16 sets specific channel, any other value for OMNI", false, false, &s_receiveChannel, eCommandType::SimpleUInt8}
};

getModuleCount(MIDIMessageConfiguration)

MIDIMessageConfiguration::MIDIMessageConfiguration() {
//    moduleID = new ModuleID("midiconfiguration", "mc", "MIDI message configuration 1.0", eModuleType::software);
    name = new String();
    setDefaultBaseParameters();
    setDefaultCCs();
}

CREATE_MODULE_COMMAND_FUNCTION(eventHandler, MIDIMessageConfiguration) {
    int argument = 0, evtype;
    int argumentCount = inCommandItem->argument.size();
    bool changedSomething = false;

    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        while (argument < argumentCount) {
            evtype = -1;
            for (int j = 0; j < 7; j++) {
                if (inCommandItem->argument[argument].toLowerCase() == eventID[j]) {
                    evtype = j;
                    break;
                } else {
//                    debugPrintln("Event type -" + inCommandItem->argument[argument].toLowerCase() + "- is not -" + eventID[j] + "-", debugPrintType::Debug);
                }
            }
            if (evtype == -1) {
                inCommandResponses->push_back({ "Event type not found: -" + inCommandItem->argument[argument].toLowerCase() + "-", debugPrintType::Error });
                return eProcessResult::WrongArgumentValue;
            }

            argument++;

            if ((argument >= argumentCount) || ((evtype == ev_cc) && ((argument + 1) >= argumentCount))) { return eProcessResult::WrongArgumentCount; }

            if (evtype == ev_cc) {
                String temp = stripQuotes(inCommandItem->argument[argument + 1]);
                setCC(inCommandItem->argument[argument].toInt(), &temp);
                argument++;
                changedSomething = true;
            } else {
                midiEventMap[evtype] = stripQuotes(inCommandItem->argument[argument]);
                changedSomething = true;
            }
            argument++;
        }
        if (!changedSomething) {
            inCommandResponses->push_back({ "Error setting midi configuration data: " + inCommandItem->originalCommand, debugPrintType::Error });
            return eProcessResult::CommandFailed;
        }
    }

    for (int i = 0; i < 6; i++) {
        String out = delimitExpression(midiEventMap[i], true);
        if (out == "") { out = "''"; }
        inCommandResponses->push_back({thisItem.shortCommand + ":" + eventID[i] + ":" + out, debugPrintType::InfoRequest});
    }
    for (int i = 0; i < controlChange.size(); i++) {
        String out = delimitExpression(controlChange[i].command, true);
        if (out == "") { out = "''"; }
        inCommandResponses->push_back({thisItem.shortCommand + ":" + eventID[ev_cc] + ":" + String(controlChange[i].control) + ":" +
                                      out, debugPrintType::InfoRequest});
    }

    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(addcc, MIDIMessageConfiguration) {
    if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
    if (!request) {
        String temp = stripQuotes(inCommandItem->argument[1]);
        int i = setCC(inCommandItem->argument[0].toInt(), &temp);
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(controlChange[i].control) + ":" + delimitExpression(stripQuotes(controlChange[i].command), true), debugPrintType::InfoRequest });
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(ccRemove, MIDIMessageConfiguration) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    if (!validateNumber(inCommandItem->argument[0].toInt(), 0, 127)) { return eProcessResult::WrongArgumentValue; }
    if (removeCC(inCommandItem->argument[0].toInt())) {
        inCommandResponses->push_back({thisItem.shortCommand + ":" + inCommandItem->argument[0], InfoRequest});
    } else {
        inCommandResponses->push_back({"Unknown CC " + inCommandItem->argument[0], Error});
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(namef, MIDIMessageConfiguration) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + (*name), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        *name = inCommandItem->argument[0];
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + (*name), InfoRequest });
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(defaults, MIDIMessageConfiguration) {
    setDefaults();
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest});

    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(receiveChannel, MIDIMessageConfiguration) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(midiRxChannel), InfoRequest});
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (!validateNumber(inCommandItem->argument[0].toInt(), 0, 255)) { return eProcessResult::WrongArgumentValue; }
        midiRxChannel = inCommandItem->argument[0].toInt();
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(midiRxChannel), debugPrintType::InfoRequest});
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(allNotesOff, MIDIMessageConfiguration) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    if (inCommandItem->argument[0].toInt() == 1) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest});
    }
    return eProcessResult::Ok;
}

uint8_t MIDIMessageConfiguration::setCC(uint8_t controller, String *command) {
    for (uint16_t i=0; i<controlChange.size(); i++) {
        if (controlChange[i].control == controller) {
            controlChange[i].command = *command;
            return i;
        }
    }
    controlChange.push_back({controller, *command });
    return controlChange.size() - 1;
}

bool MIDIMessageConfiguration::removeCC(uint8_t controller) {
    for (uint16_t i=0; i<controlChange.size(); i++) {
        if (controlChange[i].control == controller) {
            controlChange.erase(controlChange.begin() + i);
            return true;
        }
    }
    return false;
}

void MIDIMessageConfiguration::setDefaultBaseParameters() {
    *name = "default";
    midiRxChannel = 0x7F;

    midiEventMap[ev_noteOn] = "bw.hsh.hb:note,bw.dcm.ru:1,bw.pe:1,bw.bp.en:1,so.en:(velocity*512)*(1-notecount),bw.sm:0";
    midiEventMap[ev_noteOff] = "bw.bp.rs:ibool(notecount),bw.sm:0";
    midiEventMap[ev_polyAfterTouch] = "";
    midiEventMap[ev_programChange] = "";
    midiEventMap[ev_channelAftertouch] = "bw.bp.mo:(pressure*512)";
    midiEventMap[ev_PitchBend] = "bw.hsh.sh:pitch*4";
}

void MIDIMessageConfiguration::setDefaultCCs() {
    controlChange.clear();
    controlChange.push_back({64, "bw.bp.hd:bool(value)"});
    controlChange.push_back({123, "mcf.ano:1"});
}

void MIDIMessageConfiguration::setDefaults() {
    setDefaultBaseParameters();
    setDefaultCCs();
}

#endif
