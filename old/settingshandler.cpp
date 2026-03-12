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
#include <EEPROM.h>
/*
String dumpData() {
    String saveData = "ver:" + currentFirmwareVersion + ","; // "ver:1,";
    saveData += "nick:" + nickName + ",";
    uint8_t saveCurrentConfig = currentConfig;
    debugPrintln("Saving configurations", debugPrintType::Debug);
    for (int i = 0; i < (configArray.size()); i++) {
        saveData += "mcf:" + String(i) + "," + configArray[i].dumpData();
    }
    saveData += "mcf:" + String(saveCurrentConfig) + ",";

    debugPrintln("Saving cv mapping", debugPrintType::Debug);
    saveData += farSingle->dumpData();
    saveData += "nop";
    return saveData;
}
*/
bool resetAllParams() {
    String saveData = "ver:" + currentFirmwareVersion + ","; // "ver:1,";
    debugPrintln("Saving\n" + saveData + "\n" + "Total: " + String(EEPROMSaveString(&saveData, 0)) + " bytes", Command);
    return true;
}

bool saveAllParams() {
    std::vector<commandResponse> inCommandResponse;
    inCommandResponse = baseModule->dump()
    String saveData = ""; //dumpData();
    for (i = 0; i < inCommandResponse.size(); i++) {
        saveData += inCommandResponse[i]
    }
    debugPrintln("Saving\n" + saveData + "\n" + "Total: " + String(EEPROMSaveString(&saveData, 0)) + " bytes", Command);
    return true;
}

bool loadAllParams() {
    String loadData; //  = new String();
    uint32_t datal = EEPROMLoadString(&loadData, 0);
    debugPrintln(loadData + "\nLoaded " + String(datal) + " bytes of data", Command);
    commands->addCommands(loadData);
    return true;
}
