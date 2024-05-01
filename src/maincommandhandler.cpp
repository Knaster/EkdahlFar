serialCommandItem serialCommandsMain[] = {
  { "requestinfo", "rqi", "command", "Retrives rather than sets data associated with a command, if appliccable"},
  { "module", "m", "0-15", "Sets the currently active string module"},
  { "modulecount", "mc", "-", "Returns the number of string modules detected"},
  // runningstatus, rs
  { "debugrunningstatus", "drs", "0|1", "Turns on or off continuous status updates" },
  { "debugprint", "dp", "command|usb|hardware|undefined|priority|error|inforequest|expressionparser|debug:1|0", "Turns on or off serial feedback for the given item"},
  // freqreport, fr
  { "debugfreqreport", "dfr", "0|1", "Sets string frequency reporting on off" },
  // saveallparameters
  { "globalsaveallparameters", "gsap", "-", "Saves all avaliable parameters"},
  // loadallparameters
  { "globalloadallparameters", "glap", "-", "Loads all avaliable parameters"},
  // uservariable, uv
  { "globaluservariable", "guv", "variable(0-9):value", "Set user variable 0-9 to value"},
  // setconfiguration, scf
  { "midisetconfiguration", "mscf", "int", "Sets the current MIDI configuration" },
  // addconfiguration, acf
  { "midiaddconfiguration", "macf", "-", "Adds a new MIDI configuration" },
  // removeconfiguration, rcf
  { "midiremoveconfiguration", "mrcf", "int", "Remove the specified MIDI configuration" },
  // numberofconfigurations, ?
  { "midiconfigurationcount", "mcc", "-", "Returns the number of MIDI configurations" },
  // eventhandler, ev
  { "midieventhandler", "mev", "noteon|noteoff|pat|cc:(0-127)|cat|pb|pc", "Set the MIDI event handling string in the current configuration"},
  // eventhandlerccremove, evccr
  { "midieventhandlerccremove", "mevcr", "cc(0-127)", "remove CC from list"},
  { "midireceivechannel", "mrc", "-", "Sets the midi receive channel of the current configuration. 1-16 sets specific channel, any other value for OMNI"},
  { "adccommandmap", "acm", "channel:command string", "Sets the command string invoked when the value on ADC channel [channel] changes"}
};

bool processMainCommands(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
    if (_commandItem->command == "module") {
        if (request) {
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!validateNumber(_commandItem->argument[0].toInt(), 0, stringModuleArray.size())) { return false; }
            currentStringModule = _commandItem->argument[0].toInt();
            freqReportChannel = currentStringModule;
            commandResponses->push_back({"Setting current string module to " + String(currentStringModule), Command});
        }
    } else
    if (_commandItem->command == "debugrunningstatus") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0] == 1) { fContinuous = true; } else { fContinuous = false; }
        commandResponses->push_back({"Setting show running status to " + String(fContinuous), Command});
    }  else
    if (_commandItem->command == "debugprint") {
        if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
        bool found = false;
        for (int i=0; i<debugPrintTypes; i++ ) {
            if (debugPrintTypeName[i] == _commandItem->argument[0]) {
                debugPrintEnabled[i] = _commandItem->argument[1].toInt();
                commandResponses->push_back({"Setting " + debugPrintTypeName[i] + " echo to " + String(debugPrintEnabled[i]), Command});
                found = true;
                break;
            }
        }
        if (!found) { debugPrintln("Print type not found", Error); }
    } else
    if (_commandItem->command == "freqreport") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0] == 1) { freqReport = true; } else { freqReport = false; }
    } else
    if (_commandItem->command == "ver") {
        debugPrintln("Ekdahl far version " + _commandItem->argument[0], Command);
    } else
    if (_commandItem->command == "globaluservariable") {
        if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
        if (!validateNumber(_commandItem->argument[0].toInt(), 0, userVariableMax)) { return false; }

        int userVariable = _commandItem->argument[0].toInt();
            duv[userVariable] = _commandItem->argument[1].toFloat();
            commandResponses->push_back({"Set user variable " + String(userVariable) + " to " + String(duv[userVariable]), Command});
    } else
    if (_commandItem->command == "midieventhandler") {
        if (request) {
            int check = 10;
            if (checkArguments(_commandItem, commandResponses, 1, true)) {
/*                check = 10;
            } else {*/
                if (_commandItem->argument[0] == "noteon") {
                    check = 1;
                } else
                if (_commandItem->argument[0] == "noteoff") {
                    check = 2;
                } else
                if (_commandItem->argument[0] == "cc") {
                    check = 3;
                } else
                if (_commandItem->argument[0] == "pat") {
                    check = 4;
                } else
                if (_commandItem->argument[0] == "pb") {
                    check = 5;
                } else
                if (_commandItem->argument[0] == "cat") {
                    check = 6;
                } else
                if (_commandItem->argument[0] == "pc") {
                    check = 7;
                };
            }

            String response = "";

            if ((check == 1) || (check == 10)) {
                response += "mev:noteon:\"" + (*configArray[currentConfig].noteOn) + "\"";
                if (check == 10) { response += ","; }
            }
            if ((check == 2) || (check == 10)) {
                response += "mev:noteoff:\"" + (*configArray[currentConfig].noteOff) + "\"";
                if (check == 10) { response += ","; }
            }
            if ((check == 3) || (check == 10)) {
                for (int i = 0; i < int(configArray[currentConfig].controlChange.size()); i++) {
                    response += "mev:cc:" + String(configArray[currentConfig].controlChange[i].control) + ":\"" + (configArray[currentConfig].controlChange[i].command) + "\"";
                    //if ((i + 1) < int(configArray[currentConfig].controlChange.size())) { response += ","; }
                    response += ",";
                }
            }
            if ((check == 4) || (check == 10)) {
                response += "mev:pat:\"" + (*configArray[currentConfig].polyAftertouch) + "\"";
                if (check == 10) { response += ","; }
            }
            if ((check == 5) || (check == 10)) {
                response += "mev:pb:\"" + (*configArray[currentConfig].pitchBend) + "\"";
                if (check == 10) { response += ","; }
            }
            if ((check == 6) || (check == 10)) {
                response += "mev:cat:\"" + (*configArray[currentConfig].channelAftertouch) + "\"";
                if (check == 10) { response += ","; }
            }
            if ((check == 7) || (check == 10)) {
                response += "mev:pc:\"" + (*configArray[currentConfig].programChange) + "\"";
                if (check == 10) { response += ","; }
            }

            if (response == "") { return true; }
            commandResponses->push_back({response, InfoRequest});
        } else {
            if (!checkArgumentsMin(_commandItem, commandResponses, 2)) { return false; }
            if (_commandItem->argument[0] == "noteon") {
                *configArray[currentConfig].noteOn = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                commandResponses->push_back({"Setting noteOn event handler to " + *configArray[currentConfig].noteOn, Command});
            } else
            if (_commandItem->argument[0] == "noteoff") {
                *configArray[currentConfig].noteOff = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                commandResponses->push_back({"Setting noteOff event handler to " + *configArray[currentConfig].noteOff, Command});
            } else
            if (_commandItem->argument[0] == "cc") {
                if (!checkArguments(_commandItem, commandResponses, 3)) { return false; }
                if (!validateNumber(_commandItem->argument[1].toInt(), 0, 127)) { return false; }
                String *sTemp = new String(_commandItem->argument[2].substring(1, _commandItem->argument[2].length() - 1));
                configArray[currentConfig].setCC(_commandItem->argument[1].toInt(), sTemp);
                commandResponses->push_back({"Setting CC " + String(_commandItem->argument[1].toInt()) + " event handler to " +
                    _commandItem->argument[2].substring(1, _commandItem->argument[2].length() - 1), Command});
            } else
            if (_commandItem->argument[0] == "pat") {
                *configArray[currentConfig].polyAftertouch = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                commandResponses->push_back({"Setting polyAfterTouch event handler to " + *configArray[currentConfig].polyAftertouch, Command});
            } else
            if (_commandItem->argument[0] == "cat") {
                *configArray[currentConfig].channelAftertouch = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                commandResponses->push_back({"Setting channelAfterTouch event handler to " + *configArray[currentConfig].channelAftertouch, Command});
            } else
            if (_commandItem->argument[0] == "pb") {
                *configArray[currentConfig].pitchBend = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                commandResponses->push_back({"Setting pitchBend event handler to " + *configArray[currentConfig].pitchBend, Command});
            } else
            if (_commandItem->argument[0] == "pc") {
                *configArray[currentConfig].programChange = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                commandResponses->push_back({"Setting programChange event handler to " + *configArray[currentConfig].programChange, Command});
            } else {
                commandResponses->push_back({"Unknown event " + _commandItem->argument[0], Error});
            }
        }
    } else
    if (_commandItem->command == "midieventhandlerccremove") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (!validateNumber(_commandItem->argument[0].toInt(), 0, 127)) { return false; }
        if (configArray[currentConfig].removeCC(_commandItem->argument[0].toInt())) {
            commandResponses->push_back({"Event handler for CC " + _commandItem->argument[0] + " removed", Command});
        } else {
            commandResponses->push_back({"Unknown CC " + _commandItem->argument[0], Error});
        }
    } else
    if (_commandItem->command == "globalsaveallparameters") {
        saveAllParams();
    } else
    if (_commandItem->command == "globalloadallparameters") {
        loadAllParams();
    } else
    if (_commandItem->command == "test") {
    } else
    if (_commandItem->command == "modulecount") {
        commandResponses->push_back({ "mc:" + String(stringModuleArray.size()), InfoRequest });
    } else
    if (_commandItem->command == "midisetconfiguration") {
        if (request) {
            commandResponses->push_back({ "mscf:" + String(currentConfig), InfoRequest});
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            //if (!validateNumber(_commandItem->argument[0].toInt(), 0, configArray.size() - 1)) { return false; }
            currentConfig = _commandItem->argument[0].toInt();
            String response = "";
            while (currentConfig > (configArray.size() - 1)) {
                configArray.push_back(configuration());
                response += "Added configuration no " + String(configArray.size() - 1) + "\n";
            }
            response += "Setting current configuration to " + String(currentConfig);
            commandResponses->push_back({ response, debugPrintType::Command});
        }
    } else
    if (_commandItem->command == "midiaddconfiguration") {
        configArray.push_back(configuration());
        commandResponses->push_back({ "Added configuration no " + String(configArray.size() - 1), InfoRequest });
    } else
    if (_commandItem->command == "midiremoveconfiguration") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (!validateNumber(_commandItem->argument[0].toInt(), 0, configArray.size() - 1)) {
            commandResponses->push_back({ "Configuration out of range", debugPrintType::Error});
            return false;
        }
        if (configArray.size() == 1) {
            commandResponses->push_back({ "Cannot remove all configurations", debugPrintType::Error});
            return false;
        }
        uint8_t removeConfig = _commandItem->argument[0].toInt();
        if (removeConfig <= currentConfig) { currentConfig -=1; }

        configArray.erase(configArray.begin() + removeConfig);
        commandResponses->push_back({ "Removed configuration " + String(removeConfig), debugPrintType::Command});
    } else
    if (_commandItem->command == "midiconfigurationcount") {
        commandResponses->push_back({ "mcc:" + String(configArray.size()), InfoRequest });
    } else
    if (_commandItem->command == "free") {
        //printHeapStats();
        debugPrintln("RAM free " + String(freeram()), Command);
    } else
    if (_commandItem->command == "midireceivechannel") {
        if (request) {
            commandResponses->push_back({ "mrc:" + String(configArray[currentConfig].midiRxChannel), InfoRequest});
        } else {
                if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!validateNumber(_commandItem->argument[0].toInt(), 0, 255)) {
                return false;
            }
            configArray[currentConfig].midiRxChannel = _commandItem->argument[0].toInt();
            commandResponses->push_back({ "Setting midi receive channel to " + String(configArray[currentConfig].midiRxChannel), debugPrintType::Command});
        }
    } else
    if (_commandItem->command == "adccommandmap") {
        if (!checkArgumentsMin(_commandItem, commandResponses, 1)) { return false; }
        uint8_t channel = _commandItem->argument[0].toInt();

        if (request) {
            commandResponses->push_back({ "acm:" + String(controlRead->cvInputCommands[channel]), InfoRequest});
        } else {
            if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
            controlRead->setADCCommands(channel, _commandItem->argument[1]);
            commandResponses->push_back({ "Setting adc channel " + String(channel) + " command string to " + String(controlRead->cvInputCommands[channel]), debugPrintType::Command});
        }
    } else {
        return false;
    }

    return true;
}

bool processRequestCommand(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool delegated) {
    if (!checkArgumentsMin(_commandItem, commandResponses, 1)) { return false; }
    String command = _commandItem->argument[0];
    for (int i = 0; i < int(sizeof(serialCommandsMain) / sizeof(serialCommandItem)); i++) {
        if (serialCommandsMain[i].shortCommand == command) {
            command = serialCommandsMain[i].longCommand;
            break;
        }
    }

    commandItem requestItem(command);
    for (uint8_t i = 1; i < _commandItem->argument.size(); i++) {
        requestItem.argument.push_back(_commandItem->argument[i]);
    }

    if (processMainCommands(&requestItem, commandResponses, true, delegated)) {
    } else
    {
        return false;
    }

    return true;
}


void processSerialCommands() {
    if (ssReader.read()) {
        commands->addCommands(ssReader.c_str());
    }

    std::vector <commandResponse> commandResponses;

    uint16_t i = 0;
    while (i < commands->item.size()) {
        commandItem *_commandItem = &commands->item[i];
        processCommandItems(_commandItem, serialCommandsMain, sizeof(serialCommandsMain) / sizeof(serialCommandItem));

        if (processMainCommands(_commandItem, &commandResponses, false)) {
        } else
        if (_commandItem->command == "requestinfo") {
            if (!checkArgumentsMin(_commandItem, &commandResponses, 1)) { break; }

            if (!processRequestCommand(_commandItem, &commandResponses, false)) {
                std::vector<commandResponse> moduleResponses;

                if (!stringModuleArray[currentStringModule].processSerialCommand(commands, &i, &moduleResponses, false)) {
                    String error = "Unknown sequence " + String(_commandItem->command);
                    for (uint8_t i = 0; i < _commandItem->argument.size(); i++) {
                        error += ":" + _commandItem->argument[i];
                    }
                    debugPrintln(error, Error);
                    //debugPrintln("Unknown command " + String(_commandItem->command), Error);
                } else {

                }
                for (int i = 0; i < int(moduleResponses.size()); i++) {
                    commandResponses.push_back({"m:" + String(currentStringModule) + "," + moduleResponses[i].response, moduleResponses[i].responseType});
                }
            }
        } else {
            std::vector<commandResponse> moduleResponses;

            if (_commandItem->command == "help") {
                String help = ":\"Below is a list of avaliable commands given in both short and long form, either works. The number and type(s) of arguments are listed afterwards and a brief description. Command usage: command:argument1:argument2 [etc]\"";
                commandResponses.push_back({help, debugPrintType::Help});
                //help = printCommandHelp(serialCommandsMain, sizeof(serialCommandsMain) / sizeof(serialCommandItem));
                addCommandHelp(serialCommandsMain, sizeof(serialCommandsMain) / sizeof(serialCommandItem), &commandResponses,"[glo]:");
                //commandResponses.push_back({help, debugPrintType::Help});

                if (!stringModuleArray[currentStringModule].processSerialCommand(commands, &i, &commandResponses, false)) {
                    String error = "Unknown sequence " + String(_commandItem->command);
                    for (uint8_t i = 0; i < _commandItem->argument.size(); i++) {
                        error += ":" + _commandItem->argument[i];
                    }
                    debugPrintln(error, Error);
//                    debugPrintln("Unknown command " + String(_commandItem->command), Error);
                }
/*
                if (!stringModuleArray[currentStringModule].processSerialCommand(commands, &i, &moduleResponses, false)) {
                    debugPrintln("Unknown command " + String(_commandItem->command), Error);
                } else {

                }
                for (int i = 0; i < int(moduleResponses.size()); i++) {
                    commandResponses.push_back({"m:" + String(currentStringModule) + ":" + moduleResponses[i].response, moduleResponses[i].responseType});
                    //help += "m:" + String(currentStringModule) + "," + moduleResponses[i].response;
                }
*/
                //help += "\"";
                //commandResponses.push_back({help, InfoRequest});
            } else
            if (stringModuleArray[currentStringModule].processSerialCommand(commands, &i, &moduleResponses, false)) {
//                debugPrintln("Check string module", Debug);
                for (int i = 0; i < int(moduleResponses.size()); i++) {
                    commandResponses.push_back({"m:" + String(currentStringModule) + "," + moduleResponses[i].response, moduleResponses[i].responseType});
                }
            } else {
                debugPrintln("Command not found '" + _commandItem->command + "'", Error);
            }
        }
        i++;
    }
    commands->item.clear();

    for (i=0; i<commandResponses.size(); i++) {
        debugPrintln(commandResponses[i].response, commandResponses[i].responseType);
    }
}


