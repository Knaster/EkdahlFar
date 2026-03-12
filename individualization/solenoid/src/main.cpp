#include <Arduino.h>

#undef AVR
#ifdef AVR
#include <WString.h>
#include <HardwareSerial.h>
#endif // AVR

#include <../../lib/SafeString/src/BufferedInput.h>
#include <../../lib/SafeString/src/BufferedOutput.h>
#include <../../lib/SafeString/src/SafeString.h>
#include <../../lib/SafeString/src/SafeStringReader.h>
#include <../../lib/SafeString/src/SafeStringStream.h>

createSafeStringReader(ssReader, 512, "\r\n");       ///< SafeString reader creation
createBufferedOutput(ssOutput, 512, DROP_UNTIL_EMPTY); ///< SafeString buffer creation

createSafeStringReader(ssReaderUSB, 512, "\r\n");       ///< SafeString reader creation
createBufferedOutput(ssOutputUSB, 512, DROP_UNTIL_EMPTY); ///< SafeString buffer creation

#include "debugprint.cpp"
#include "generalhelpers.cpp"
#include "commandparser.hpp"
#include "solenoid.hpp"
#include "basemodule.hpp"
#define LED 32

Solenoid *solenoid;
BaseModule *baseModule;
std::vector<commandResponse> commandResponses;

tModuleID tmoduleID = { "", "", "Solenoid controller v1.0", eModuleType::hardware, false, "" };

void setup() {
	Serial.begin(115200);
	Serial1.begin(115200);

	ssOutputUSB.connect(Serial);
	ssReaderUSB.connect(Serial);

    ssOutput.connect(Serial1);
    ssReader.connect(Serial1);

    solenoid = new Solenoid(LED);
    baseModule = new BaseModule(solenoid);
    baseModule->tmoduleID = &tmoduleID;

    debugPrintln("Initialized", debugPrintType::InfoRequest);

    pinMode(32, OUTPUT);
};

bool avaliable;
String read;

void loop() {
    avaliable = false;

	if (ssReaderUSB.read()) {
        read = ssReaderUSB.c_str();
        avaliable = true;
	} else
	if (ssReader.read()) {
        read = ssReader.c_str();
        avaliable = true;
	}

	if (avaliable) {
        read.trim();
        read = stripQuotes(read);

        CommandItem inCommandItem(&read);
        baseModule->processSelf(&inCommandItem, &commandResponses, false, eProcessResult::NotFound);

        for (int i = 0; i < commandResponses.size(); i++) {
            debugPrintln(commandResponses[i].response, commandResponses[i].responseType);
            debugPrintln(commandResponses[i].response, commandResponses[i].responseType, &ssOutputUSB);
        }
        commandResponses.clear();
        //debugPrint("got " + read, debugPrintType::Debug);
	}
    baseModule->update();

    ssOutputUSB.nextByteOut();
    ssOutput.nextByteOut();
};

