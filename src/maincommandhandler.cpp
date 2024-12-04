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
serialCommandItem serialCommandsMain[] = {
  { "requestinfo", "rqi", "command", "Retrives rather than sets data associated with a command, if appliccable"},
  { "module", "m", "0-15", "Sets the currently active string module"},
  { "modulecount", "mc", "-", "Returns the number of string modules detected"},

  //{ "debugrunningstatus", "drs", "0|1", "Turns on or off continuous status updates" },  // runningstatus, rs
  { "debugprint", "dp", "command|usb|hardware|undefined|priority|error|inforequest|expressionparser|debug:1|0", "Turns on or off serial feedback for the given item"},
//  { "debugfreqreport", "dfr", "0|1", "Sets string frequency reporting on off" },    // freqreport, fr
  { "version", "ver", "-", "Gets the current firmware version"},
  { "globalsaveallparameters", "gsap", "-", "Saves all avaliable parameters"},  // saveallparameters
  { "globalloadallparameters", "glap", "-", "Loads all avaliable parameters"},  // loadallparameters
  { "globalresetallparameters", "grap", "-", "Resets all saved parameters"},
  { "globaluservariable", "guv", "variable(0-9):value", "Set user variable 0-9 to value"},  // uservariable, uv

/*
    Saving custom data:
        litterals - example:
            gcsl:"adcsettings:5:2:10:3:10, adcsettings:0:2:10:3:10"
            - when invoked:
                litteralvariable = "adcsettings:5:2:10:3:10, adcsettings:0:2:10:3:10"
                parse the command list in litteralvariable and execute all commands
            - when saved
                save as 'gscl:"' + litteralvariable + '"'
            - at startup:
                -> when invoked

        rqis - examples
            gscr:"adcsetting:5, adcsetting:0"
            - when invoked
                rqivariable = argument[0] = "adcsetting:5, adcsetting:0"
                if argument[1] exists and isn't null
                    parse argument[1] as a commandlist and execute all commands

            - when saved
                    parse rqivariable command list and do an rqi on all commands and save the outcome:
                        rqi:adcsettings:5   -> rqivariableoutput = "adcsettings:5:2:10:3:10"
                        rqi:adcsettings:0   -> rqivariableoutput += ", " + "adcsettings:0:2:10:3:10"

                    save as 'gscr: "' + rqivariable + '", "' + rqivariableoutput + '"' which becomes
                     'gscr:"adcsetting:5, adcsetting:0":"adcsettings:5:2:10:3:10, adcsettings:0:2:10:3:10"'

        rqis - example 2, with saved data 'gscr:"adcsetting:5":"adcsettings:5:2:10:3:10"'
            - startup:
                rqivariable = "adcsetting:5"
                execute "adcsettings:5:2:10:3:10"
            - when working
                executing "adcsettings:5:1:5:2:10"
            - later saving
                parsing rqivariable and doing an rqi on the outcome:
                    rqi:adcsetting:5    -> rqivariableoutput = "adcsettings:5:1:5:2:10"
                save as 'gscr:"adcsetting:5":"adcsettings:5:1:5:2:10"'
            - rebooting
                rqivariable = "adcsetting:5"
                executing "adcsetting:5:1:5:2:10"

*/

//  { "globalcustomstartuplitterals", "gcsl", "string", "Save the command string and execute at startup" },
//  { "globalcustomstartuprqis", "gcsr", "string", "A list of commands to, when saving parameters, perform the 'requestinfo' command on and add to the " },

  { "midiconfiguration", "mcf", "int", "Sets the current MIDI configuration" }, // setconfiguration, scf
  { "midiconfigurationadd", "mcfa", "-", "Adds a new MIDI configuration" }, // addconfiguration, acf
  { "midiconfigurationremove", "mcfr", "int", "Remove the specified MIDI configuration" },  // removeconfiguration, rcf
  { "midiconfigurationcount", "mcfc", "-", "Returns the number of MIDI configurations" },   // numberofconfigurations, ?
  { "midiconfigurationname", "mcfn", "string", "Set the name of the MIDI configuration (for request, argument is index of configuration to return name for (optional))" },
  { "midieventhandler", "mev", "noteon|noteoff|pat|cc:(0-127)|cat|pb|pc", "Set the MIDI event handling string in the current configuration"},   // eventhandler, ev
  { "midieventhandlerccremove", "mevcr", "cc(0-127)", "remove CC from list"},   // eventhandlerccremove, evccr
  { "midiconfigurationdefaults", "mcfd", "-", "Reverts the current configuration to default values and CCs"},
  { "midireceivechannel", "mrc", "-", "Sets the MIDI receive channel of the current configuration. 1-16 sets specific channel, any other value for OMNI"},
//  { "midisustain", "msu", "1|0", "Turn sustain on for MIDI notes, aka ignore NOTE OFF messages and whatever commands they have"},
  { "midiallnotesoff", "mano", "1|0", "Clear the entire buffer of MIDI notes held"},

  { "adccommandmap", "acm", "channel:command string", "Sets the command string invoked when the value on ADC channel [channel] changes"},
  { "adcdefaults", "acd", "Reverts all ADC command strings to default values" },
  { "adcread", "adcr", "channel:value", "Sent when a new value is presented on one of the ADC channels, cannot be invoked" },
  { "adcsettings", "adcs", "channel:averages:interrupterrorthreshold:continuouserrorthreshold:continuoustimeout", "Explain ADC settings here"},

  { "expressionparserevaluate", "epev", "expression", "Evaluates an arithmetric expression and sends back the output"},
  { "expressionparserdeadbandthreshold", "epdbt", "float", "Sets the threshold for the deadband function in the expression parser"},

  { "testadclatency", "tal", "0-65535", "Test ADC Latency" },
  { "testadclatencyreturn", "talr", "-", "Return from test" },
  { "testadcminmax", "tamm", "channel", "Measure min/max value for a given channel and resets the counter" },
};

bool processMainCommands(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
    if (_commandItem->command == "module") {
        if (request) {
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!validateNumber(_commandItem->argument[0].toInt(), 0, stringModuleArray.size())) { return false; }
            currentStringModule = _commandItem->argument[0].toInt();
//            freqReportChannel = currentStringModule;
            commandResponses->push_back({"Setting current string module to " + String(currentStringModule), Command});
        }
    } else
/*    if (_commandItem->command == "debugrunningstatus") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0] == 1) { fContinuous = true; } else { fContinuous = false; }
        commandResponses->push_back({"Setting show running status to " + String(fContinuous), Command});
    }  else*/
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
        if (!found) {
            commandResponses->push_back({"Print type not found", Command});
        }
    } else
/*    if (_commandItem->command == "freqreport") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0] == 1) { freqReport = true; } else { freqReport = false; }
    } else*/
    if (_commandItem->command == "version") {
        if (request) {
            commandResponses->push_back({"ver:" + currentFirmwareVersion, InfoRequest});
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if ((currentFirmwareVersion != _commandItem->argument[0])) {
                firmwareChanged = true;
                commandResponses->push_back({"Firmware version changed! Previous version: " + _commandItem->argument[0], InfoRequest});
            }
            commandResponses->push_back({"Current firmware version: " + currentFirmwareVersion, InfoRequest});
        }
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
                //response += "mev:noteon:\"" + (*configArray[currentConfig].noteOn) + "\"";
                response += "mev:noteon:" + delimitExpression(*configArray[currentConfig].noteOn, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 2) || (check == 10)) {
                //response += "mev:noteoff:\"" + (*configArray[currentConfig].noteOff) + "\"";
                response += "mev:noteoff:" + delimitExpression(*configArray[currentConfig].noteOff, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 3) || (check == 10)) {
                for (int i = 0; i < int(configArray[currentConfig].controlChange.size()); i++) {
                    //response += "mev:cc:" + String(configArray[currentConfig].controlChange[i].control) + ":\"" + (configArray[currentConfig].controlChange[i].command) + "\"";
                    response += "mev:cc:" + String(configArray[currentConfig].controlChange[i].control) + ":" + delimitExpression(configArray[currentConfig].controlChange[i].command, true) + "";
                    response += ",";
                }
            }
            if ((check == 4) || (check == 10)) {
                //response += "mev:pat:\"" + (*configArray[currentConfig].polyAftertouch) + "\"";
                response += "mev:pat:" + delimitExpression(*configArray[currentConfig].polyAftertouch, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 5) || (check == 10)) {
                //response += "mev:pb:\"" + (*configArray[currentConfig].pitchBend) + "\"";
                response += "mev:pb:" + delimitExpression(*configArray[currentConfig].pitchBend, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 6) || (check == 10)) {
                //response += "mev:cat:\"" + (*configArray[currentConfig].channelAftertouch) + "\"";
                response += "mev:cat:" + delimitExpression(*configArray[currentConfig].channelAftertouch, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 7) || (check == 10)) {
                //response += "mev:pc:\"" + (*configArray[currentConfig].programChange) + "\"";
                response += "mev:pc:" + delimitExpression(*configArray[currentConfig].programChange, true) + "";
                if (check == 10) { response += ","; }
            }

            if (response == "") { return true; }
            commandResponses->push_back({response, InfoRequest});
        } else {
            if (!checkArgumentsMin(_commandItem, commandResponses, 2)) { return false; }
            if (_commandItem->argument[0] == "noteon") {
//                *configArray[currentConfig].noteOn = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                *configArray[currentConfig].noteOn = stripQuotes(_commandItem->argument[1]);
                commandResponses->push_back({"Setting noteOn event handler to " + *configArray[currentConfig].noteOn, Command});
            } else
            if (_commandItem->argument[0] == "noteoff") {
                //*configArray[currentConfig].noteOff = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                *configArray[currentConfig].noteOff = stripQuotes(_commandItem->argument[1]);
                commandResponses->push_back({"Setting noteOff event handler to " + *configArray[currentConfig].noteOff, Command});
            } else
            if (_commandItem->argument[0] == "cc") {
                if (!checkArguments(_commandItem, commandResponses, 3)) { return false; }
                if (!validateNumber(_commandItem->argument[1].toInt(), 0, 127)) { return false; }
                //String *sTemp = new String(_commandItem->argument[2].substring(1, _commandItem->argument[2].length() - 1));
                String *sTemp = new String(stripQuotes(_commandItem->argument[2]));
                configArray[currentConfig].setCC(_commandItem->argument[1].toInt(), sTemp);
//                commandResponses->push_back({"Setting CC " + String(_commandItem->argument[1].toInt()) + " event handler to " +
//                    _commandItem->argument[2].substring(1, _commandItem->argument[2].length() - 1), Command});
                commandResponses->push_back({"Setting CC " + String(_commandItem->argument[1].toInt()) + " event handler to " +
                    *sTemp, Command});
            } else
            if (_commandItem->argument[0] == "pat") {
                //*configArray[currentConfig].polyAftertouch = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                *configArray[currentConfig].polyAftertouch = stripQuotes(_commandItem->argument[1]);
                commandResponses->push_back({"Setting polyAfterTouch event handler to " + *configArray[currentConfig].polyAftertouch, Command});
            } else
            if (_commandItem->argument[0] == "cat") {
                //*configArray[currentConfig].channelAftertouch = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                *configArray[currentConfig].channelAftertouch = stripQuotes(_commandItem->argument[1]);
                commandResponses->push_back({"Setting channelAfterTouch event handler to " + *configArray[currentConfig].channelAftertouch, Command});
            } else
            if (_commandItem->argument[0] == "pb") {
                //*configArray[currentConfig].pitchBend = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                *configArray[currentConfig].pitchBend =  stripQuotes( _commandItem->argument[1]);
                commandResponses->push_back({"Setting pitchBend event handler to " + *configArray[currentConfig].pitchBend, Command});
            } else
            if (_commandItem->argument[0] == "pc") {
                //*configArray[currentConfig].programChange = _commandItem->argument[1].substring(1, _commandItem->argument[1].length() - 1);
                *configArray[currentConfig].programChange = stripQuotes(_commandItem->argument[1]);
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
    if (_commandItem->command == "globalresetallparameters") {
        resetAllParams();
    } else
    if (_commandItem->command == "test") {
    } else
    if (_commandItem->command == "testadclatency") {
  //      commandResponses->push_back({"Starting ADC latency test", InfoRequest});
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
//        if (!validateNumber(_commandItem->argument[0].toInt(), 0, 65535)) { return false; }
        testMeasurement = 0;
        testMeasurementOngoing = true;
        analogWrite(3, int(_commandItem->argument[0].toInt()));
        commandResponses->push_back({"Starting ADC latency test", InfoRequest});
    } else
    if (_commandItem->command == "testadclatencyreturn") {
        testMeasurementOngoing = false;
        commandResponses->push_back({"Returned from ADC latency test in " + String(testMeasurement) + " uS", InfoRequest});
    } else
    if (_commandItem->command == "testadcminmax") {
        if (request) {
            commandResponses->push_back({ "tamm:" + String(controlRead->testChannel) + ":" + String(controlRead->testMin) + ":" + String(controlRead->testMax), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            int testChannel = _commandItem->argument[0].toInt();
            if ((testChannel < 0) || (testChannel > 7)) { return false; }
            controlRead->setADCMinMaxTestChannel(testChannel);
            commandResponses->push_back({ "Starting ADC min/max test on channel " + String(controlRead->testChannel), InfoRequest });
        }
    } else


    if (_commandItem->command == "modulecount") {
        commandResponses->push_back({ "mc:" + String(stringModuleArray.size()), InfoRequest });
    } else
    if (_commandItem->command == "midiconfigurationname") {
        if (request) {
            int8_t conf = currentConfig;
            if (checkArguments(_commandItem, commandResponses, 1, true)) {
                conf = _commandItem->argument[0].toInt();
                if ((conf < 0) || (conf >= configArray.size())) { return false; }
            }
            commandResponses->push_back({ "mcfn:" + String(conf) + ":" + *(configArray[conf].name), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
//            _commandItem->argument[0].replace("\"", "");
//            _commandItem->argument[0].replace("'", "");
//            *(configArray[currentConfig].name) = _commandItem->argument[0];
            *(configArray[currentConfig].name) = stripQuotes(_commandItem->argument[0]);
            commandResponses->push_back({ "Set configuration name to " + *(configArray[currentConfig].name), InfoRequest });
        }
    } else
    if (_commandItem->command == "midiconfiguration") {
        if (request) {
            commandResponses->push_back({ "mcf:" + String(currentConfig), InfoRequest});
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
    if (_commandItem->command == "midiconfigurationadd") {
        configArray.push_back(configuration());
        commandResponses->push_back({ "Added configuration no " + String(configArray.size() - 1), InfoRequest });
    } else
    if (_commandItem->command == "midiconfigurationremove") {
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
        commandResponses->push_back({ "mcfc:" + String(configArray.size()), InfoRequest });
    } else
    if (_commandItem->command == "free") {
        //printHeapStats();
        commandResponses->push_back({ "free:" + String(freeram()), InfoRequest });
    } else
    if (_commandItem->command == "midiconfigurationdefaults") {
        configArray[currentConfig].setDefaults();
        commandResponses->push_back({ "Configuration reverted to defaults", debugPrintType::Command});
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
            commandResponses->push_back({ "Setting MIDI receive channel to " + String(configArray[currentConfig].midiRxChannel), debugPrintType::Command});
        }
    } else
/*    if (_commandItem->command == "midisustain") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0].toInt() == 1) {
            sustain = true;
        } else {
            sustain = false;
        }
        commandResponses->push_back({ "MIDI sustain set to " + String(sustain), debugPrintType::Command});
    } else*/
    if (_commandItem->command == "midiallnotesoff") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0].toInt() == 1) {
            midiAllNotesOff();
            commandResponses->push_back({ "All MIDI notes off", debugPrintType::Command});
        }
    } else
    if (_commandItem->command == "adcread") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        int i = _commandItem->argument[0].toInt();
        if ((i < 0) || (i > 7)) { return false; }

        int32_t convertedValue;
        if (i < 5) {
            convertedValue = (int32_t) ((float) controlRead->getData(i) * ((float) 65536 / 32767));
        } else {
            convertedValue = (int32_t) ((float) controlRead->getData(i) * ((float) 65536 / 2048));
        }
        if (convertedValue > 65535) { convertedValue = 65535; }
        commandResponses->push_back({ "adcr:" + String(i) + ":" + String(convertedValue) + ":" + String(controlRead->getData(i)), debugPrintType::InfoRequest});
    } else
    if (_commandItem->command == "adccommandmap") {
        if (!checkArgumentsMin(_commandItem, commandResponses, 1)) { return false; }
        uint8_t channel = _commandItem->argument[0].toInt();

        if (request) {
            //commandResponses->push_back({ "acm:" + String(channel) + ":'" + String(controlRead->cvInputCommands[channel]) + "'", InfoRequest});
            commandResponses->push_back({ "acm:" + String(channel) + ":" + delimitExpression(controlRead->cvInputCommands[channel], true), InfoRequest});
        } else {
            if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
            controlRead->setADCCommands(channel, stripQuotes(_commandItem->argument[1]));
            commandResponses->push_back({ "Setting adc channel " + String(channel) + " command string to " + controlRead->cvInputCommands[channel], debugPrintType::Command});
        }
    } else
    if (_commandItem->command == "adcdefaults") {
        controlRead->setDefaults();
        commandResponses->push_back({ "Reverted adc channel commands", debugPrintType::Command});
    } else

// { "adcsettings", "adcs", "channel:averages:interrupterrorthreshold:continuouserrorthreshold:continuoustimeout"
    if (_commandItem->command == "adcsettings") {
        if (!checkArgumentsMin(_commandItem, commandResponses, 1)) { return false; }
        uint8_t channel = _commandItem->argument[0].toInt();
        if (channel > 7) { return false; }

        if (request) {
            commandResponses->push_back({ "adcs:" + String(channel) + ":" + controlRead->getADCAveragerSettings(channel), InfoRequest});
        } else {
            if (!checkArguments(_commandItem, commandResponses, 5)) { return false; }
            if (!controlRead->setADCAveragerSettings(_commandItem->argument[0].toInt(), _commandItem->argument[1].toInt(), _commandItem->argument[2].toInt(), _commandItem->argument[3].toInt(),
                _commandItem->argument[4].toInt())) { return false; }
            commandResponses->push_back({ "Setting adc channel " + String(channel) + " settings to " + controlRead->getADCAveragerSettings(channel), debugPrintType::Command});
        }
    } else

    if (_commandItem->command == "expressionparserevaluate") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        updateLocalVariables();

        commandList testCommands("ev:" + _commandItem->argument[0]);
/*
        String expression =_commandItem->argument[0];
        debugPrintln("index of \" " + String(expression.indexOf("\"")) + ", ' " + String(expression.indexOf("'")), debugPrintType::Debug);
        if ((expression.indexOf("\"") != -1) || (expression.indexOf("'") != -1)) {
            expression = expression.substring(1, expression.length() - 1);
            debugPrintln("Cleaned expression " + expression, debugPrintType::Debug);
        }
        commandList testCommands("ev:" + expression);
*/
        //testCommands.addCommands;
        testCommands.parseCommandExpressions(expFunctions, expFunctionCount);
        commandResponses->push_back({ "Evaluation result: " + String(testCommands.item[0].argument[0]), debugPrintType::InfoRequest});
    } else
    if (_commandItem->command == "expressionparserdeadbandthreshold") {
        if (request) {
            commandResponses->push_back({ "epdbt:" + String(epDeadbandThreshold), InfoRequest});
        } else {
            if (!checkArgumentsMin(_commandItem, commandResponses, 1)) { return false; }
            //controlRead->setADCCommands(channel, _commandItem->argument[1]);
            epDeadbandThreshold = _commandItem->argument[0].toFloat();
            commandResponses->push_back({ "Setting expression parser deadband thershold to " + String(epDeadbandThreshold), debugPrintType::Command});
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
                    commandResponses.push_back({error, debugPrintType::Error});
                    //debugPrintln(error, Error);
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
                addCommandHelp(serialCommandsMain, sizeof(serialCommandsMain) / sizeof(serialCommandItem), &commandResponses,"[glo]:");

                if (!stringModuleArray[currentStringModule].processSerialCommand(commands, &i, &commandResponses, false)) {
                    String error = "Unknown sequence " + String(_commandItem->command);
                    for (uint8_t i = 0; i < _commandItem->argument.size(); i++) {
                        error += ":" + _commandItem->argument[i];
                    }
                    commandResponses.push_back({error, debugPrintType::Error});
                }
            } else
            if (stringModuleArray[currentStringModule].processSerialCommand(commands, &i, &moduleResponses, false)) {
                for (int i = 0; i < int(moduleResponses.size()); i++) {
                    commandResponses.push_back({"m:" + String(currentStringModule) + "," + moduleResponses[i].response, moduleResponses[i].responseType});
                }
            } else {
                commandResponses.push_back({"Command not found -" + _commandItem->command + "-", debugPrintType::Error});
            }
        }
        i++;
    }
    commands->item.clear();

    for (i=0; i<commandResponses.size(); i++) {
        debugPrintln(commandResponses[i].response, commandResponses[i].responseType);
    }
}


