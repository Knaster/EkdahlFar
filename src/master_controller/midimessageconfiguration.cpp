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

#include <base/arduinorequired.hpp>
//#include "avr_functions.h"
#include "master_controller/midimessageconfiguration.hpp"
#include "master_controller/midicc.hpp"

const ModuleCommandDeclaration MIDIMessageConfiguration::moduleCommands[] = {
    //{ "", "", "int", "Sets the current MIDI configuration", false, false, nullptr, eCommandType_function::ectIndex },
    { "name", "na", "string", "Set the name of the MIDI configuration", false, true, &s_namef,
        eCommandType_data::ectSimpleString | eCommandType_function::ectName },
/*    { "data", "da", "noteon|noteoff|pat|cc:(0-127)|cat|pb|pc:outputassignment", "Set the MIDI event handling data, the event given will execute the given command string",
        false, false, &s_eventHandler, eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment | eCommandType_dataOptions::ectExpression },*/
    { "noteon", "non", "outputassignment", "Sets the commands to execute at a MIDI Note On event", false, true, &s_eventHandlerNew,
        eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment | eCommandType_dataOptions::ectExpression, "channel,note,velocity" },
    { "noteoff", "nof", "outputassignment", "Sets the commands to execute at a MIDI Note Off event", false, true, &s_eventHandlerNew,
        eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment | eCommandType_dataOptions::ectExpression, "channel,note,velocity" },
    { "polyaftertouch", "pat", "outputassignment", "Sets the commands to execute at a MIDI Polyphonic after touch event", false, true, &s_eventHandlerNew,
        eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment | eCommandType_dataOptions::ectExpression, "channel,note,pressure" },
    { "channelaftertouch", "cat", "outputassignment", "Sets the commands to execute at a MIDI Channel after touch event", false, true, &s_eventHandlerNew,
        eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment | eCommandType_dataOptions::ectExpression, "channel,velocity" },
    { "pitchbend", "pb", "outputassignment", "Sets the commands to execute at a MIDI Pitch bend event", false, true, &s_eventHandlerNew,
        eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment | eCommandType_dataOptions::ectExpression, "channel,pitch" },
    { "programchange", "pc", "outputassignment", "Sets the commands to execute at a MIDI Program change event", false, true, &s_eventHandlerNew,
        eCommandType_data::ectOutputAssignment | eCommandType_function::ectAssignment | eCommandType_dataOptions::ectExpression, "channel,program" },
/*    { "continuouscontrollerdata", "ccd", "number:outputassignment", "Sets the commands to execute at a MIDI Continuous controller event with the given [number]", false, true, &s_continuouscontroller,
        eCommandType_data::ectData | eCommandType_function::ectParameter | eCommandType_dataOptions::ectExpression, "channel,control,value" },*/
    { "addcc", "ac", "0-127:outputassignment", "Add continuous controller and the command string to execute", false, false, &s_ccAdd,
        eCommandType_data::ectData | eCommandType_function::ectAddModule },
    { "removecc", "rmc", "cc number", "remove continuous controller from list", false, false, &s_ccRemove, eCommandType_function::ectRemoveModule },
    { "countcc", "ccc", "-", "returns the number of configured continuous controllers", false, false, &s_ccCount, eCommandType_function::ectCountModules | eCommandType_access::ectRequest },
    { "defaults", "d", "-", "Reverts the current configuration to default values and CCs", false, false, &s_defaults, eCommandType_data::ectConditional | eCommandType_function::ectAdminAction },
    { "receivechannel", "rc", "-", "Sets the MIDI receive channel of the current configuration. 1-16 sets specific channel, any other value for OMNI", false, true, &s_receiveChannel,
        eCommandType_data::ectSimpleUInt8 | eCommandType_function::ectLiveParameter }
};

getModuleCount(MIDIMessageConfiguration)

MIDIMessageConfiguration::MIDIMessageConfiguration() {
//    moduleID = new ModuleID("midiconfiguration", "mc", "MIDI message configuration 1.0", eModuleType::software);
    name = new String();
    setDefaultBaseParameters();
    setDefaultCCs();
}

CREATE_MODULE_COMMAND_FUNCTION(eventHandlerNew, MIDIMessageConfiguration) {
    uint8_t evtype = -1;

    for (int j = 0; j < 7; j++) {
        if (thisItem.longCommand == eventID2[j]) {
            evtype = j;
            break;
        } else {
        }
    }
    if (evtype == -1) {
        inCommandResponses->push_back({ "Event type not found: -" + thisItem.longCommand + "-", debugPrintType::Error });
        return eProcessResult::WrongArgumentValue;
    }

    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }

        midiEventMap[evtype] = stripQuotes(inCommandItem->argument[0]);
    }

    String out = delimitExpression(midiEventMap[evtype], true);
    if (out == "") { out = "''"; }
    inCommandResponses->push_back({thisItem.shortCommand + ":" + out, debugPrintType::InfoRequest});
    return eProcessResult::Ok;
}
/*
CREATE_MODULE_COMMAND_FUNCTION(continuouscontroller, MIDIMessageConfiguration) {
    ModuleGroup *ccgroup;
    MIDICC *mcc = nullptr;

    if (!checkArgumentsMin(inCommandItem, inCommandResponses, 1, true)) {
        if (!request) { return eProcessResult::WrongArgumentCount; }

        ccgroup = getGroup("cc");
        if (ccgroup != nullptr) {
            for (uint8_t i=0; i<ccgroup->modules.size(); i++) {
                mcc = static_cast<MIDICC*> (ccgroup->modules[i]);
                inCommandResponses->push_back({thisItem.shortCommand + ":" + String(mcc->ccnum) + ":" + delimitExpression(mcc->outputS, true), debugPrintType::InfoRequest});
            }
        }
        return eProcessResult::Ok;
    }

    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 2, true)) { return eProcessResult::WrongArgumentCount; }
        setCC(inCommandItem->argument[0].toInt(), &stripQuotes(inCommandItem->argument[1]));
    }

    ccgroup = getGroup("cc");
    if (ccgroup == nullptr) { return eProcessResult::CommandFailed; }
    uint8_t controller = inCommandItem->argument[0].toInt();

    for (uint8_t i=0; i<ccgroup->modules.size(); i++) {
        mcc = static_cast<MIDICC*> (ccgroup->modules[i]);
        if (mcc->ccnum == controller) {
            inCommandResponses->push_back({thisItem.shortCommand + ":" + String(mcc->ccnum) + ":" + delimitExpression(mcc->outputS, true), debugPrintType::InfoRequest});
            return eProcessResult::Ok;
        }
    }

    return eProcessResult::CommandFailed;
}
*/
CREATE_MODULE_COMMAND_FUNCTION(ccAdd, MIDIMessageConfiguration) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        if (!request) {
            String temp = stripQuotes(inCommandItem->argument[1]);
            uint8_t cc = setCC(inCommandItem->argument[0].toInt(), &temp);
    //        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(controlChange[i].control) + ":" + delimitExpression(stripQuotes(controlChange[i].command), true), debugPrintType::InfoRequest });
            inCommandResponses->push_back({ thisItem.shortCommand + ":cc:" + String(cc) + ":" + delimitExpression(stripQuotes(temp), true), debugPrintType::InfoRequest });
        }
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(ccRemove, MIDIMessageConfiguration) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (!validateNumber(inCommandItem->argument[0].toInt(), 0, 127)) { return eProcessResult::WrongArgumentValue; }

        int8_t cc = removeCC(inCommandItem->argument[0].toInt());

        if (cc != -1) {
            inCommandResponses->push_back({thisItem.shortCommand + ":cc:" + String(cc), InfoRequest});
        } else {
            inCommandResponses->push_back({"Unknown CC " + inCommandItem->argument[0], Error});
        }
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(ccCount, MIDIMessageConfiguration) {
    ModuleGroup *group = getGroup("continuouscontroller");
    if (group == nullptr) { return eProcessResult::CommandFailed; }

    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(group->modules.size()), debugPrintType::InfoRequest});
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
    if (!request) {
        setDefaults();
        inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest});
    }
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
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() == 1) {
            inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest});
        }
    }
    return eProcessResult::Ok;
}

uint8_t MIDIMessageConfiguration::setCC(uint8_t controller, String *command) {
    ModuleGroup *ccgroup = getGroup("cc");
    if (ccgroup != nullptr) {
        for (uint8_t i=0; i<ccgroup->modules.size(); i++) {

            if ((static_cast<MIDICC*> (ccgroup->modules[i]))->ccnum == controller) {
                (static_cast<MIDICC*> (ccgroup->modules[i]))->outputS = *command;
                //return controlChange.size() - 1;
                return i;
            }
        }
    }
    addModule(new MIDICC(controller, command));
    return ccgroup->modules.size() - 1;
    //return controlChange.size() - 1;
}

int8_t MIDIMessageConfiguration::removeCC(uint8_t controller) {
/*    for (uint16_t i=0; i<controlChange.size(); i++) {
        if (controlChange[i].control == controller) {
            controlChange.erase(controlChange.begin() + i);
            return true;
        }
    }*/
    ModuleGroup *ccgroup = getGroup("cc");
    if (ccgroup != nullptr) {
        for (uint8_t i=0; i<ccgroup->modules.size(); i++) {
            //ccgroup->modules.erase(ccgroup->modules.begin() + i);
            if ((static_cast<MIDICC*> (ccgroup->modules[i]))->ccnum == controller) {
                ccgroup->modules.erase(ccgroup->modules.begin() + i);
                return i;
            }
        }
    }

    return -1;
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
    //controlChange.clear();
    //controlChange.push_back({64, "bw.bp.hd:bool(value)"});
    //controlChange.push_back({123, "mcf.ano:1"});
    ModuleGroup *ccgroup = getGroup("cc");
    if (ccgroup != nullptr) {
        ccgroup->modules.clear();
    }
    setCC(64, &String("bw.bp.hd:bool(value)"));
    setCC(123, &String("mcf.ano:1"));
}

void MIDIMessageConfiguration::setDefaults() {
    setDefaultBaseParameters();
    setDefaultCCs();
}

void MIDIMessageConfiguration::addInstances(String name, uint8_t count) {
    debugPrintln("addInstances reached", debugPrintType::Debug);
    if ((name != "cc") and (name != "continuouscontroller")) {
        debugPrintln("Unknown instance name '" + name + "'", debugPrintType::Error);
    } else {
        ModuleGroup *group = getGroup("cc");
        if (group == nullptr) {
                debugPrintln("Error in addInstances", debugPrintType::Error);
                return;
        }
        debugPrintln("Count is " + String(count) + " size is " + String(group->modules.size()), debugPrintType::Debug);
        while (count > group->modules.size()) {
            group->addModule(new MIDICC(0, &String("")));
            debugPrintln("Added module '" + name + "'", debugPrintType::Debug);
        }
    }
}
#endif
