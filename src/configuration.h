#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <vector>

class configuration {
public:
    String *name;
    String *noteOn;
    String *noteOff;
    String *polyAftertouch;
    String *programChange;
    String *channelAftertouch;
    String *pitchBend;

    uint8_t midiRxChannel;

//    uint16_t EEPROM_offset = 0;

    struct _controlChange {
        byte control;
        String command;
    };

    std::vector<_controlChange> controlChange;

    configuration();
    uint8_t setCC(uint8_t controller, String *command);
    bool removeCC(uint8_t controller);
    void setDefaultBaseParameters();
    void setDefaultCCs();
    void setDefaults();
//    uint16_t saveParams();
//    uint16_t loadParams(uint8_t EEPROMVersion);
    String dumpData();
};
#endif
