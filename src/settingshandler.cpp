#include <EEPROM.h>

String dumpData() {
    String saveData = "ver:1,";
    uint8_t saveCurrentConfig = currentConfig;
    for (int i = 0; i < (configArray.size()); i++) {
        saveData += "mscf:" + String(i) + "," + configArray[i].dumpData();
    }
    saveData += "mscf:" + String(saveCurrentConfig) + ",";
    saveData += "m:0," + stringModuleArray[0].calibrateArray[0].dumpData();
    saveData += stringModuleArray[0].muteArray[0].dumpData();
    saveData += stringModuleArray[0].bowControlArray[0].dumpData();
    saveData += stringModuleArray[0].bowIOArray[0].dumpData();
    saveData += stringModuleArray[0].solenoidArray[0].dumpData();
    saveData += stringModuleArray[0].bowControlArray[0].harmonicSeriesList.dumpData();
    saveData += stringModuleArray[0].bowControlArray[0].bowActuators->dumpData() + "\n";
    return saveData;
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
