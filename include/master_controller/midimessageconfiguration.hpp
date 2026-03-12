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
#ifndef MIDIMESSAGECONFIGURATION_H
#define MIDIMESSAGECONFIGURATION_H

#include "base/arduinorequired.hpp"
#include <vector>
#include "base/module.hpp"

class MIDIMessageConfiguration : public Module {
public:
    SETMODULEID("midiconfiguration", "mc", "MIDI message configuration 1.0", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(eventHandler, MIDIMessageConfiguration)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(addcc, MIDIMessageConfiguration)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ccRemove, MIDIMessageConfiguration)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(defaults, MIDIMessageConfiguration)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(receiveChannel, MIDIMessageConfiguration)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(allNotesOff, MIDIMessageConfiguration)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(namef, MIDIMessageConfiguration)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(eventHandlerNew, MIDIMessageConfiguration)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(continuouscontroller, MIDIMessageConfiguration)

    String *name;
/*
    String *noteOn;
    String *noteOff;
    String *polyAftertouch;
    String *programChange;
    String *channelAftertouch;
    String *pitchBend;
*/
    enum eMIDIEvent {
        ev_noteOn = 0,
        ev_noteOff = 1,
        ev_polyAfterTouch = 2,
        ev_programChange = 3,
        ev_channelAftertouch = 4,
        ev_PitchBend = 5,
        ev_cc = 6
    };

    const String eventID[7] = { "noteon", "noteoff", "pat", "pc", "cat", "pb", "cc" };
    const String eventID2[7] = { "noteon", "noteoff", "polyaftertouch", "programchange", "channelaftertouch", "pitchbend", "continuouscontroller" };

    String midiEventMap[7];

    uint8_t midiRxChannel;

    struct _controlChange {
        byte control;
        String command;
    };

    std::vector<_controlChange> controlChange;

    MIDIMessageConfiguration();
    uint8_t setCC(uint8_t controller, String *command);
    bool removeCC(uint8_t controller);
    void setDefaultBaseParameters();
    void setDefaultCCs();
    void setDefaults();
//    String dumpData();
};
#endif
