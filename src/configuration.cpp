#ifndef CONFIGURATION_C
#define CONFIGURATION_C

#include <WProgram.h>
#include "avr_functions.h"
#include "configuration.h"

configuration::configuration() {
    #define noteOnSingle "m:guv0,b:0,bchb:note,bmr:1,bpid:1,bpe:1,se:(velocity*512)*(1-notecount),bcsm:0" //"m:uv0,b:0,hb:note,run:1,pid:1,engage:1,muterest,ssm:0"
    #define noteOffSingle "m:guv0,b:0,bpr:ibool(notecount),bcsm:0" // "m:uv0,b:0,rest:ibool(notecount),mutefullmute,ssm:0"

    noteOff = new String(noteOffSingle);
    noteOn = new String(noteOnSingle);
    polyAftertouch = new String("");
    programChange = new String("");
    channelAftertouch = new String("m:guv0,b:0,bpm:(pressure*512)");
    pitchBend = new String("m:0,b:0,bchsh:pitch/2");
    midiRxChannel = 0x7F;

    /*controlChange.push_back( { 0, "m:0,spb:value*512"} );
    controlChange.push_back( { 48, "se:value*512"} );
    controlChange.push_back( { 72, "m:uv0,b:0,msp:(value*512)"} );
    controlChange.push_back( { 2, "ssfm:value/127"} );*/
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

String configuration::dumpData() {
    String dump = "mrc:" + String(midiRxChannel)+ ",";
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
