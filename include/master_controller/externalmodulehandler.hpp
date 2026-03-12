#ifndef EXTERNALMODULEHANDLER_HPP
#define EXTERNALMODULEHANDLER_HPP

#include <vector>
//#include "HardwareSerial.h"

//#include "string.h"
//#include <base/arduinorequired.hpp>
#include <HardwareSerial.h>
#include <BufferedInput.h>
#include <BufferedOutput.h>
#include <SafeString.h>
#include <SafeStringReader.h>

#include "debugprint.hpp"
#include "commandparser.hpp"
#include "commandlist.hpp"
#include "modulehandler.hpp"
#include "externalmodule.hpp"

/*
Transmission:
	Select TX channel
	Transmit - slaves should always be ready

Reading:
	CTS LOW
	Check if DATA_AVALIABLE
		iterate through RX addresses until slave found, i.e. master_RTS on that channel is TRUE
			CTS HIGH
			- receive
			CTS LOW
*/

extern CommandList globalCommands;
extern CommandList globalResponseCommands;

struct externalOutItem {
    uint8_t extID;
    String commandString;
};

enum eScanResponse {
    noResponse,
    errorInResponse,
    added
};

class SerialHardwareWrapper {
public:
    SerialHardwareWrapper(HardwareSerial *inPort, uint32_t inBaud, ModuleHandler *inMainModule, void *parent);
    eScanResponse scanForModules();
    tModuleID *tmoduleID;

    static void s_transmit(String commands, uint8_t extID, void *parent);
    void checkForData();
    void transmit(String commands, uint8_t extID);
    void update();

    uint8_t childCount;
protected:
    HardwareSerial *port;
    uint32_t baud;
private:
    uint8_t currentIndex;
    const uint16_t serialTimeout = 250;
    ModuleHandler *mainModule;
    std::vector<ExternalModule*> modules;
    bool (emhCallback) (Module);
    std::vector<externalOutItem> externalOutputBuffer;
    String beep;
};

class ExternalModuleHandler
{
public:
    ExternalModuleHandler();
    void addSerialHardware(HardwareSerial *port);
    void scanForModules();

    ModuleHandler *mainModule;

    void update();
protected:

private:
    std::vector<SerialHardwareWrapper> serial;
    const uint32_t baud = 115200;
};

#endif // EXTERNALMODULEHANDLER_HPP
