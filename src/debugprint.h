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
