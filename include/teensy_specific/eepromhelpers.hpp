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
#ifndef EEPROM_HELPERS_HPP
#define EEPROM_HELPERS_HPP

uint32_t EEPROMSaveString(String *data, uint32_t EEPROM_offset) {
    uint32_t datal = data->length();
    eeprom_write_dword(0, datal);
    eeprom_write_block(data->c_str(), (void*) (EEPROM_offset + 4), data->length() + 1); // Save one above length so the trailing NULL is included
    return 2 + data->length();
}

uint32_t EEPROMLoadString(String *data, uint32_t EEPROM_offset) {
    uint32_t datal = 0;
    eeprom_read_block(&datal, (void*) (EEPROM_offset), 1);
    datal = eeprom_read_dword(0);
    if (datal != 0) {
        char text[datal + 1];  // Make enough room for the Text + NULL
        eeprom_read_block(&text, (void*) (EEPROM_offset + 4), datal + 1); // save text + NULL
        *data = text;
    } else {
        *data = "";
    }

    return datal + 2;  // +1 for intial length byte, +1 for trailing NULL
}

#endif
