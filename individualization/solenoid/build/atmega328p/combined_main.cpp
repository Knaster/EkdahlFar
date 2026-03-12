/*
  main.cpp - Main loop for Arduino sketches
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>

extern "C" void __cxa_pure_virtual() { while(1);};
#include <Arduino.h>

#undef AVR
#ifdef AVR
#include <WString.h>
#include <HardwareSerial.h>
#endif // AVR

#define GLOBALS

#include "debugprint.cpp"
#include "commandparser.hpp"
#include "solenoid.hpp"

#define LED 32

Solenoid *solenoid;
std::vector<commandResponse> commandResponses;

void setup() {
	Serial.begin(115200);
    solenoid = new Solenoid(32);
    debugPrintln("Initialized", debugPrintType::InfoRequest);

    pinMode(32, OUTPUT);
};

void loop() {
	if (Serial.available()) {
        String read = Serial.readString();
        read.trim();

        commandItem inCommandItem(read);
        Serial.flush();

        String response = inCommandItem.command;
        for (int i = 0; i < inCommandItem.argument.size(); i++) {
            response += ":" + inCommandItem.argument[i];
        }
        eProcessResult processResult = solenoid->processCommands(&inCommandItem, &commandResponses, false);

        switch (processResult) {
        case eProcessResult::NotFound:
            commandResponses.push_back({"Command not found: " + response, debugPrintType::Error});
            break;
        case eProcessResult::CommandFailed:
            commandResponses.push_back({"Command failed: " + response, debugPrintType::Error});
            break;
        case eProcessResult::WrongArgumentCount:
        case eProcessResult::WrongArgumentMinimum:
            commandResponses.push_back({"Wrong command arguments: " + response, debugPrintType::Error});
            break;
        case eProcessResult::WrongArgumentValue:
            commandResponses.push_back({"Wrong command argument values: " + response, debugPrintType::Error});
            break;
        case eProcessResult::Ok:
            //debugPrintln("Ok", debugPrintType::TextInfo);
            break;
        case eProcessResult::PassThrough:
            //debugPrintln("Ok, pass-through", debugPrintType::TextInfo);
            break;
        default:
            debugPrintln("Unknown response " + String(processResult), debugPrintType::Error);
        };

        for (int i=0; i<commandResponses.size(); i++) {
            debugPrintln(commandResponses[i].response, commandResponses[i].responseType);
        }

	}
	solenoid->update();
};

int atexit(void (* /*func*/ )()) { return 0; }

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

void setupUSB() __attribute__((weak));
void setupUSB() { }

int main(void)
{
	init();

	initVariant();

#if defined(USBCON)
	USBDevice.attach();
#endif
	
	setup();
    
	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}
        
	return 0;
}

