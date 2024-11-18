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
#ifndef CONFIGURATION_C
#define CONFIGURATION_C

#include <WProgram.h>
#include "avr_functions.h"
#include "configuration.h"

configuration::configuration() {
    #define noteOnSingle "m:uv0,b:0,bchb:note,bmr:1,bpid:1,bpe:1,se:(velocity*512)*(1-notecount),bcsm:0" //"m:uv0,b:0,hb:note,run:1,pid:1,engage:1,muterest,ssm:0"
    #define noteOffSingle "m:uv0,b:0,bpr:ibool(notecount),bcsm:0" // "m:uv0,b:0,rest:ibool(notecount),mutefullmute,ssm:0"

    name = new String(); //String("default");
    noteOff = new String(); //String(noteOffSingle);
    noteOn = new String(); //String(noteOnSingle);
    polyAftertouch = new String();
    programChange = new String();
    channelAftertouch = new String(); //String("m:uv0,b:0,bpm:(pressure*512)");
    pitchBend = new String(); //String("m:0,b:0,bchsh:pitch/2");
//    midiRxChannel = 0x7F;
    setDefaultBaseParameters();
}

uint8_t configuration::setCC(uint8_t controller, String *command) {
    for (uint16_t i=0; i<controlChange.size(); i++) {
        if (controlChange[i].control == controller) {
            controlChange[i].command = *command;
            return i;
        }
    }
    controlChange.push_back({controller, *command });
    return controlChange.size() - 1;
}

bool configuration::removeCC(uint8_t controller) {
    for (uint16_t i=0; i<controlChange.size(); i++) {
        if (controlChange[i].control == controller) {
            controlChange.erase(controlChange.begin() + i);
            return true;
        }
    }
    return false;
}

void configuration::setDefaultBaseParameters() {
    *name = "default";
    *noteOff = "m:0,b:0,bpr:ibool(notecount),bcsm:0";
    *noteOn = "m:0,b:0,bchb:note,bmr:1,bpid:1,bpe:1,se:(velocity*512)*(1-notecount),bcsm:0";
    *polyAftertouch = "";
    *programChange = "";
    *channelAftertouch = "m:0,b:0,bpm:(pressure*512)";
    *pitchBend = "m:0,b:0,bchsh:pitch*4";
    midiRxChannel = 0x7F;
}

void configuration::setDefaultCCs() {
    controlChange.clear();
    controlChange.push_back({64, "midisustain:bool(value)"});
    controlChange.push_back({123, "midiallnotesoff:1"});
}

void configuration::setDefaults() {
    setDefaultBaseParameters();
    setDefaultCCs();
}

String configuration::dumpData() {
    String dump = "mrc:" + String(midiRxChannel)+ ",";
    if (*name != "default") { dump += "mcfn:\"" +  (*name) + "\","; }
    if (*noteOff != "") { dump += "mev:noteoff:\"" + (*noteOff) + "\","; }
    if (*noteOn != "") { dump += "mev:noteon:\"" + (*noteOn) + "\","; }
    if (*polyAftertouch != "") { dump += "mev:pat:\"" + (*polyAftertouch) + "\","; }
    if (*programChange != "") { dump += "mev:pc:\"" + (*programChange) + "\","; }
    if (*channelAftertouch != "") { dump += "mev:cat:\"" + (*channelAftertouch) + "\","; }
    if (*pitchBend != "") { dump += "mev:pb:\"" + (*pitchBend) + "\","; }
    for (int i = 0; i < int(controlChange.size()); i++) {
        dump += "mev:cc:" + String(controlChange[i].control) + ":\"" + controlChange[i].command + "\",";
    }

    return dump;
}
#endif
