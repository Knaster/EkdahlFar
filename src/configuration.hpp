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
