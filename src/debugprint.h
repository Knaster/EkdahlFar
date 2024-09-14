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
enum debugPrintType { Command, USB, Hardware, Undefined, Priority, Error, InfoRequest, EParser, Debug, TextInfo, Help};
#define debugPrintTypes 11
String debugPrintTypeName[debugPrintTypes] = { "command", "usb", "hardware" , "undefined", "priority", "error", "inforequest", "expressionparser", "debug", "textinfo", "help" };
String debugPrintTypeNameShort[debugPrintTypes] = { "cmd", "usb", "hw", "un", "pri", "err", "irq", "ep", "dbg", "txi", "hlp"};
bool debugPrintEnabled[debugPrintTypes] = { true, true, true, true, true, true, true, true, true, true, true };

bool debugPrintCheckType(debugPrintType printType) {
  if (debugPrintEnabled[printType] == true) { return true; }
  return false;
}
void debugPrint(String text, debugPrintType printType) {
  if (debugPrintCheckType(printType)) { ssOutput.print("[" + debugPrintTypeNameShort[printType] + "]" + text); }
  return;
}

void debugPrintln(String text, debugPrintType printType) {
  if (debugPrintCheckType(printType)) { ssOutput.println("[" + debugPrintTypeNameShort[printType] + "]" + text); } // + " [" + debugPrintTypeName[printType] + "]"
  return;
}

void debugRaw(String text) {
    ssOutput.println(text);
}

bool setDebugReporting(String debugName, bool setting) {
  for (int i=0; i<debugPrintTypes; i++) {
    if (debugPrintTypeName[i] == debugName) {
      debugPrintEnabled[i] = setting;
      debugPrintln("Setting reporting of name " + debugName + " to " + setting, Command);
      return true;
    }
  }
  return false;
}

#define commandResponseType debugPrintType

struct commandResponse {
    String response;
    commandResponseType responseType;
};
