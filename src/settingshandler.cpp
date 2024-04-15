#include <EEPROM.h>
/*
uint16_t loadConfigurationParams(uint8_t EEPROMVersion) {
  if (EEPROMVersion == 0x02) {
    uint8_t configCount;
    uint16_t offset = 0;
    configCount = eeprom_read_byte (currentEEPROMOffset);
    currentEEPROMOffset += 1;
    debugPrintln("Found " + String(configCount) + " configurations", Debug);
    for (uint8_t i=0; i<configCount; i++) {
      if (i >= configArray.size()) {
        debugPrintln("Inserting new configuration", Debug);
        configuration *tempConf = new configuration();
        configArray.push_back(*tempConf);
      }
      configArray[i].EEPROM_offset = currentEEPROMOffset;
      debugPrintln("Loading configuration data from offset " + String(configArray[i].EEPROM_offset), Debug);
      currentEEPROMOffset += configArray[i].loadParams(EEPROMVersion);
    }
  }
  return 0;
}

bool loadParams() {
  readEEPROMVersion = eeprom_read_byte(0); //1
  debugPrintln("EEPROM Version: " + String(readEEPROMVersion), Debug);
  if (readEEPROMVersion == 0x02) {
    currentEEPROMOffset = 1;
    currentEEPROMOffset += loadConfigurationParams(readEEPROMVersion);
  } else {
    currentEEPROMOffset = 0;
    readEEPROMVersion = 0;
  }
  return true;
}

// Save all once-occuring data and configurations
uint16_t saveParams() {
  eeprom_write_byte (0, 0x02);  // Version
  eeprom_write_byte (1, configArray.size());
  uint16_t localEEPROM_offset = 2;

  for (uint8_t i=0; i<configArray.size(); i++) {
    debugPrintln("Saving config " + String(i), Debug);
    configArray[i].EEPROM_offset = localEEPROM_offset;
    localEEPROM_offset += configArray[i].saveParams();
  }
  return localEEPROM_offset;
}
*/
String dumpData() {
    String saveData = "ver:1,";
    uint8_t saveCurrentConfig = currentConfig;
    for (int i = 0; i < (configArray.size()); i++) {
        saveData += "scf:" + String(i) + "," + configArray[i].dumpData();
    }
    saveData += "scf:" + String(saveCurrentConfig) + ",";
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
    /*  uint16_t localEEPROMOffset = saveParams(); // Save all once-occuring instrument data, configurations etc
    for (int i=0;i<stringModuleArray.size();i++) {
    stringModuleArray[i].EEPROM_offset = localEEPROMOffset;
    debugPrintln("Saving string data at " + String(localEEPROMOffset), Debug);
    localEEPROMOffset += stringModuleArray[i].saveParams();
    }
    debugPrintln("All parameters saved", Debug);*/
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
