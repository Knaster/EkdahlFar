#ifndef EXTERNALMODULEHANDLER_CPP
#define EXTERNALMODULEHANDLER_CPP

#include "externalmodulehandler.hpp"

createSafeStringReader(ssExtReader, 512, "\r\n");       ///< SafeString reader creation
createBufferedOutput(ssExtOutput, 512, DROP_UNTIL_EMPTY); ///< SafeString buffer creation

SerialHardwareWrapper::SerialHardwareWrapper(HardwareSerial *inPort, uint32_t inBaud, ModuleHandler *inMainModule, void *inParent) {
    port = inPort;
    baud = inBaud;
    mainModule = inMainModule;
//    parent = inParent;
    childCount = 0;
    currentIndex = 0;
};

eScanResponse SerialHardwareWrapper::scanForModules() {
    port->begin(baud);
    ssExtReader.connect(Serial1);
    ssExtOutput.connect(Serial1);
    ssExtOutput.println("rqi:ver");

    uint32_t i = millis();
    bool boolread = false;
    while ((millis() < (i + serialTimeout)) && (!boolread)) { boolread = ssExtReader.read(); };
    if (!boolread) {
        debugPrintln("Serial device didn't respond", debugPrintType::Error);
        return eScanResponse::noResponse;
    }

    String deviceString = ssExtReader.c_str();;
    if (deviceString.substring(0,9) != "[irq]ver:") {
        debugPrintln("Invalid response:" + deviceString, debugPrintType::Error);
        return eScanResponse::errorInResponse;
    }
    deviceString = deviceString.substring(5);
    debugPrintln("Received external response " + deviceString, debugPrintType::Debug);
    ExternalModule *module = new ExternalModule(deviceString);
    module->extID = childCount;
    module->transmit = &s_transmit;
    module->transmissionObject = this;
//    ModuleGroup *group = mainModule->addModule(module);
    mainModule->addModule(module);
    modules.push_back(module);
//    debugPrintln("Module added to group: " + String(group->tmoduleID.longName) + " length: " + String(group->modules.size()), debugPrintType::Debug);
    childCount++;

    return eScanResponse::added;
}

void SerialHardwareWrapper::checkForData() {
//    if ()
}

void SerialHardwareWrapper::update() {
    if (ssExtReader.read()) {
        String extResponse = ssExtReader.c_str();
        debugPrintType dp = debugPrintGetType(&extResponse);
        extResponse = extResponse.substring(5);

        CommandItem extItem = CommandItem(&extResponse);
        if (extItem.hierarchy.size() > 0) {
            if (extItem.hierarchy[0].name == "ls" || extItem.hierarchy[0].name == "list") {
                String shortLineage;
                modules[currentIndex]->getLineage(&shortLineage, false);
                extResponse = extResponse.substring(0, extResponse.lastIndexOf(":") + 2) + shortLineage + "." + extResponse.substring(extResponse.lastIndexOf(":") + 2);
                //debugPrintln(extResponse.substring(0, extResponse.lastIndexOf(":") + 2), debugPrintType::Debug);
                //debugPrintln(extResponse.substring(extResponse.lastIndexOf(":") + 2), debugPrintType::Debug);
            }
        }
        String longLineage, shortLineage;
        modules[currentIndex]->getLineage(&longLineage, true);
        extResponse = extResponse.substring(0, extResponse.indexOf(":") + 1) + longLineage + "." + extResponse.substring(extResponse.indexOf(":") + 2);

        debugPrintln(extResponse, dp);
    }
    ssExtOutput.nextByteOut();
};

void SerialHardwareWrapper::s_transmit(String commands, uint8_t extID, void *parent) {
//void SerialHardwareWrapper::s_transmit(String commands, uint8_t extID, void *SerialHardwareWrapper) {
    //parent->transmit(commands + "\r\n", extID);
    ((SerialHardwareWrapper*) parent)->transmit(commands + "\r\n", extID);
}

void SerialHardwareWrapper::transmit(String commands, uint8_t extID) {
    //if (port->availableForWrite()) {
    //    port->println(commands);
    if (ssExtOutput.availableForWrite()) {
        debugPrintln("Transmit: " + commands, debugPrintType::Debug);
        ssExtOutput.println(commands);
    } else {
        debugPrintln("Not available for write, post-poning", debugPrintType::Debug);
        externalOutputBuffer.push_back({ extID, commands });
    }
};

ExternalModuleHandler::ExternalModuleHandler() {
}

void ExternalModuleHandler::addSerialHardware(HardwareSerial *port) {
    SerialHardwareWrapper shw(port, baud, mainModule, this);
    serial.push_back({ shw });
}

void ExternalModuleHandler::scanForModules() {
    for (uint8_t i = 0; i < serial.size(); i++) {
        if (serial[i].scanForModules() != eScanResponse::added) {
            delay(500);
            serial[i].scanForModules();
        };
    }
};

void ExternalModuleHandler::update() {
    for (int i = 0; i < serial.size(); i++) {
        serial[i].update();
    }
}
#endif
