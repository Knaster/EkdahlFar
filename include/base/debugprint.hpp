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

 #ifndef DEBUGPRINT_HPP
 #define DEBUGPRINT_HPP

// #include <WProgram.h>
// #include "string.h"

#include "base/arduinorequired.hpp"

enum debugPrintType { Command, USB, Hardware, Undefined, Priority, Error, InfoRequest, EParser, Debug, TextInfo, Help, Internal};
#define debugPrintTypes 12
extern String debugPrintTypeName[];
extern String debugPrintTypeNameShort[];
extern bool debugPrintEnabled[];

void outputNext();

bool debugPrintConnect();

bool debugPrintCheckType(debugPrintType printType);

void debugPrint(String text, debugPrintType printType);

void debugPrintln(String text, debugPrintType printType);

void debugRaw(String text);

bool setDebugReporting(String debugName, bool setting);

class threadSafeDebugPrint {
private:
    String intDrivenMsgP;
    bool locked = false;
    int i;
public:
    bool tsMsgFlag = false;

    threadSafeDebugPrint();

    void tsDebugPrintln(String m_msg, debugPrintType m_dpType);

    String tsRetrieveMessages();
};

#define commandResponseType debugPrintType

struct commandResponse {
    String response;
    commandResponseType responseType;
};
#endif // DEBUGPRINT_H
