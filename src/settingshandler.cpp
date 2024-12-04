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

String dumpData() {
    String saveData = "ver:" + currentFirmwareVersion + ","; // "ver:1,";
    uint8_t saveCurrentConfig = currentConfig;
    debugPrintln("Saving configurations", debugPrintType::Debug);
    for (int i = 0; i < (configArray.size()); i++) {
        saveData += "mcf:" + String(i) + "," + configArray[i].dumpData();
    }
    saveData += "mcf:" + String(saveCurrentConfig) + ",";

    debugPrintln("Saving cv mapping", debugPrintType::Debug);
    saveData += controlRead->dumpData();

    debugPrintln("Saving calibration data", debugPrintType::Debug);
    saveData += "m:0," + stringModuleArray[0].calibrateArray[0].dumpData();
    debugPrintln("Saving harmonic series", debugPrintType::Debug);
    saveData += stringModuleArray[0].bowControlArray[0].harmonicSeriesList.dumpData();
    debugPrintln("Saving mute data", debugPrintType::Debug);
    saveData += stringModuleArray[0].muteArray[0].dumpData();
    debugPrintln("Saving bow control data", debugPrintType::Debug);
    saveData += stringModuleArray[0].bowControlArray[0].dumpData();
    debugPrintln("Saving bow IO data", debugPrintType::Debug);
    saveData += stringModuleArray[0].bowIOArray[0].dumpData();
    debugPrintln("Saving solenoid data", debugPrintType::Debug);
    saveData += stringModuleArray[0].solenoidArray[0].dumpData();
    debugPrintln("Saving actuator data", debugPrintType::Debug);
    saveData += stringModuleArray[0].bowControlArray[0].bowActuators->dumpData();
    saveData += "nop";
    return saveData;
}

bool resetAllParams() {
    String saveData = "ver:" + currentFirmwareVersion + ","; // "ver:1,";
    debugPrintln("Saving\n" + saveData + "\n" + "Total: " + String(EEPROMSaveString(&saveData, 0)) + " bytes", Command);
    return true;
}

bool saveAllParams() {
    String saveData = dumpData();
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
