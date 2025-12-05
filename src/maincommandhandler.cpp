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

  { "debugprint", "dp", "command|usb|hardware|undefined|priority|error|inforequest|expressionparser|debug:1|0", "Turns on or off serial feedback for the given item"},
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
  { "midiallnotesoff", "mano", "1|0", "Clear the entire buffer of MIDI notes held"},

  { "adccommandmap", "acm", "channel:command string", "Sets the command string invoked when the value on ADC channel [channel] changes"},
  { "adcdefaults", "acd", "-" "Reverts all ADC command strings to default values" },
  { "adcread", "adcr", "channel:value", "Sent when a new value is presented on one of the ADC channels, cannot be invoked" },
  { "adcsettings", "adcs", "channel:averages:interrupterrorthreshold:continuouserrorthreshold:continuoustimeout", "Explain ADC settings here"},

  { "expressionparserevaluate", "epev", "expression", "Evaluates an arithmetric expression and sends back the output"},

  { "testadclatency", "tal", "0-65535", "Test ADC Latency" },
  { "testadclatencyreturn", "talr", "-", "Return from test" },
  { "testadcminmax", "tamm", "channel", "Measure min/max value for a given channel and resets the counter" },
  { "reset", "rst", "0|1", "Resets the Ekdahl FAR, conditional"},
  { "nick", "nick", "string", "Sets the nickname of this unit" },
  { "nooperation", "nop", "-", "Do absolutely, positively, nothing"}
};

eProcessResult processMainCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
/*    if (inCommandItem->command == "module") {
        if (request) {
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return false; }
            if (!validateNumber(inCommandItem->argument[0].toInt(), 0, stringModuleArray.size())) { return false; }
            currentStringModule = inCommandItem->argument[0].toInt();
//            freqReportChannel = currentStringModule;
            commandResponses->push_back({"m:" + String(currentStringModule), InfoRequest});
        }
    } else*/
/*    if (inCommandItem->command == "modulecount") {
        commandResponses->push_back({ "mc:" + String(stringModuleArray.size()), InfoRequest });
    } else*/
    if (inCommandItem->command == "debugprint") {
        if (!checkArguments(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        bool found = false;
        for (int i=0; i<debugPrintTypes; i++ ) {
            if (debugPrintTypeName[i] == inCommandItem->argument[0]) {
                debugPrintEnabled[i] = inCommandItem->argument[1].toInt();
                commandResponses->push_back({"dp" + debugPrintTypeName[i] + ":" + String(debugPrintEnabled[i]), InfoRequest});
                found = true;
                break;
            }
        }
        if (!found) { return eProcessResult::WrongArgumentValue; }
    } else
    if (inCommandItem->command == "version") {
        if (request) {
            commandResponses->push_back({"ver:" + currentFirmwareVersion, InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if ((currentFirmwareVersion != inCommandItem->argument[0])) {
                firmwareChanged = true;
                commandResponses->push_back({"Firmware version changed! Previous version: " + inCommandItem->argument[0], InfoRequest});
            }
            commandResponses->push_back({"version:" + currentFirmwareVersion, InfoRequest});
        }
    } else
    if (inCommandItem->command == "globaluservariable") {
        if (!checkArguments(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        if (!validateNumber(inCommandItem->argument[0].toInt(), 0, userVariableMax)) { return eProcessResult::WrongArgumentValue; }

        int userVariable = inCommandItem->argument[0].toInt();
        if (!request) { duv[userVariable] = inCommandItem->argument[1].toFloat(); }
        commandResponses->push_back({"guv:" + String(userVariable) + ":" + String(duv[userVariable]), InfoRequest});
    } else
    if (inCommandItem->command == "midieventhandler") {
        if (request) {
            int check = 10;
            if (checkArguments(inCommandItem, commandResponses, 1, true)) {
                if (inCommandItem->argument[0] == "noteon") {
                    check = 1;
                } else
                if (inCommandItem->argument[0] == "noteoff") {
                    check = 2;
                } else
                if (inCommandItem->argument[0] == "cc") {
                    check = 3;
                } else
                if (inCommandItem->argument[0] == "pat") {
                    check = 4;
                } else
                if (inCommandItem->argument[0] == "pb") {
                    check = 5;
                } else
                if (inCommandItem->argument[0] == "cat") {
                    check = 6;
                } else
                if (inCommandItem->argument[0] == "pc") {
                    check = 7;
                };
            }

            String response = "";

            if ((check == 1) || (check == 10)) {
                response += "mev:noteon:" + delimitExpression(*configArray[currentConfig].noteOn, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 2) || (check == 10)) {
                response += "mev:noteoff:" + delimitExpression(*configArray[currentConfig].noteOff, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 3) || (check == 10)) {
                for (int i = 0; i < int(configArray[currentConfig].controlChange.size()); i++) {
                    response += "mev:cc:" + String(configArray[currentConfig].controlChange[i].control) + ":" + delimitExpression(configArray[currentConfig].controlChange[i].command, true) + "";
                    response += ",";
                }
            }
            if ((check == 4) || (check == 10)) {
                response += "mev:pat:" + delimitExpression(*configArray[currentConfig].polyAftertouch, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 5) || (check == 10)) {
                response += "mev:pb:" + delimitExpression(*configArray[currentConfig].pitchBend, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 6) || (check == 10)) {
                response += "mev:cat:" + delimitExpression(*configArray[currentConfig].channelAftertouch, true) + "";
                if (check == 10) { response += ","; }
            }
            if ((check == 7) || (check == 10)) {
                response += "mev:pc:" + delimitExpression(*configArray[currentConfig].programChange, true) + "";
                if (check == 10) { response += ","; }
            }

            if (response == "") { return eProcessResult::Ok; }
            commandResponses->push_back({response, InfoRequest});
        } else {
            if (!checkArgumentsMin(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentMinimum; }
            if (inCommandItem->argument[0] == "noteon") {
                *configArray[currentConfig].noteOn = stripQuotes(inCommandItem->argument[1]);
                commandResponses->push_back({"mev:noteon:" + delimitExpression(*configArray[currentConfig].noteOn, true), InfoRequest});
            } else
            if (inCommandItem->argument[0] == "noteoff") {
                *configArray[currentConfig].noteOff = stripQuotes(inCommandItem->argument[1]);
                commandResponses->push_back({"mev:noteoff:" + delimitExpression(*configArray[currentConfig].noteOff, true), InfoRequest});
            } else
            if (inCommandItem->argument[0] == "cc") {
                if (!checkArguments(inCommandItem, commandResponses, 3)) { return eProcessResult::WrongArgumentCount; }
                if (!validateNumber(inCommandItem->argument[1].toInt(), 0, 127)) { return eProcessResult::WrongArgumentValue; }
                String *sTemp = new String(stripQuotes(inCommandItem->argument[2]));
                configArray[currentConfig].setCC(inCommandItem->argument[1].toInt(), sTemp);
                commandResponses->push_back({"mev:cc:" + String(inCommandItem->argument[1].toInt()) + ":" + delimitExpression(*sTemp, true), InfoRequest});
            } else
            if (inCommandItem->argument[0] == "pat") {
                *configArray[currentConfig].polyAftertouch = stripQuotes(inCommandItem->argument[1]);
                commandResponses->push_back({"mev:pat:" + delimitExpression(*configArray[currentConfig].polyAftertouch, true), InfoRequest});
            } else
            if (inCommandItem->argument[0] == "cat") {
                *configArray[currentConfig].channelAftertouch = stripQuotes(inCommandItem->argument[1]);
                commandResponses->push_back({"mev:cat:" + delimitExpression(*configArray[currentConfig].channelAftertouch, true), InfoRequest});
            } else
            if (inCommandItem->argument[0] == "pb") {
                *configArray[currentConfig].pitchBend =  stripQuotes( inCommandItem->argument[1]);
                commandResponses->push_back({"mev:pb:" + delimitExpression(*configArray[currentConfig].pitchBend, true), InfoRequest});
            } else
            if (inCommandItem->argument[0] == "pc") {
                *configArray[currentConfig].programChange = stripQuotes(inCommandItem->argument[1]);
                commandResponses->push_back({"mev:pc:" + delimitExpression(*configArray[currentConfig].programChange, true), InfoRequest});
            } else {
                commandResponses->push_back({"Unknown event " + inCommandItem->argument[0], Error});
            }
        }
    } else
    if (inCommandItem->command == "midieventhandlerccremove") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (!validateNumber(inCommandItem->argument[0].toInt(), 0, 127)) { return eProcessResult::WrongArgumentValue; }
        if (configArray[currentConfig].removeCC(inCommandItem->argument[0].toInt())) {
            commandResponses->push_back({"mevcr:" + inCommandItem->argument[0], InfoRequest});
        } else {
            commandResponses->push_back({"Unknown CC " + inCommandItem->argument[0], Error});
        }
    } else
    if (inCommandItem->command == "globalsaveallparameters") {
        saveAllParams();
        commandResponses->push_back({"gsap:1", InfoRequest});
    } else
    if (inCommandItem->command == "globalloadallparameters") {
        loadAllParams();
        commandResponses->push_back({"glap:1", InfoRequest});
    } else
    if (inCommandItem->command == "globalresetallparameters") {
        resetAllParams();
        commandResponses->push_back({"grap:1", InfoRequest});
    } else
    if (inCommandItem->command == "test") {
    } else
    if (inCommandItem->command == "testadclatency") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        testMeasurement = 0;
        testMeasurementOngoing = true;
        analogWrite(3, int(inCommandItem->argument[0].toInt()));
        commandResponses->push_back({"Starting ADC latency test", InfoRequest});
    } else
    if (inCommandItem->command == "testadclatencyreturn") {
        testMeasurementOngoing = false;
        commandResponses->push_back({"Returned from ADC latency test in " + String(testMeasurement) + " uS", InfoRequest});
    } else
    if (inCommandItem->command == "testadcminmax") {
        if (request) {
            commandResponses->push_back({ "tamm:" + String(controlRead->testChannel) + ":" + String(controlRead->testMin) + ":" + String(controlRead->testMax), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            int testChannel = inCommandItem->argument[0].toInt();
            if ((testChannel < 0) || (testChannel > 7)) { return eProcessResult::WrongArgumentValue; }
            controlRead->setADCMinMaxTestChannel(testChannel);
            commandResponses->push_back({ "Starting ADC min/max test on channel " + String(controlRead->testChannel), InfoRequest });
        }
    } else
    if (inCommandItem->command == "midiconfigurationname") {
        if (request) {
            int8_t conf = currentConfig;
            if (checkArguments(inCommandItem, commandResponses, 1, true)) {
                conf = inCommandItem->argument[0].toInt();
                if ((conf < 0) || (conf >= configArray.size())) { return eProcessResult::WrongArgumentValue; }
            }
            commandResponses->push_back({ "mcfn:" + String(conf) + ":" + *(configArray[conf].name), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            *(configArray[currentConfig].name) = stripQuotes(inCommandItem->argument[0]);
            commandResponses->push_back({ "mcfn:" + *(configArray[currentConfig].name), InfoRequest });
        }
    } else
    if (inCommandItem->command == "midiconfiguration") {
        if (request) {
            commandResponses->push_back({ "mcf:" + String(currentConfig), InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            currentConfig = inCommandItem->argument[0].toInt();
            String response = "";
            while (currentConfig > (configArray.size() - 1)) {
                configArray.push_back(configuration());
                response += "Added configuration no " + String(configArray.size() - 1) + "\n";
            }
            commandResponses->push_back({ "mcf: " + String(currentConfig), InfoRequest});
        }
    } else
    if (inCommandItem->command == "midiconfigurationadd") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        configArray.push_back(configuration());
        uint16_t index = configArray.size() - 1;
        *configArray[index].name = inCommandItem->argument[0];
        commandResponses->push_back({ "mcfa:" + String(index), InfoRequest });
    } else
    if (inCommandItem->command == "midiconfigurationremove") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (!validateNumber(inCommandItem->argument[0].toInt(), 0, configArray.size() - 1)) { return eProcessResult::WrongArgumentValue; };
        if (configArray.size() == 1) {
            commandResponses->push_back({ "Cannot remove all configurations", debugPrintType::Error});
            return eProcessResult::CommandFailed;
        }
        uint8_t removeConfig = inCommandItem->argument[0].toInt();
        if (removeConfig <= currentConfig) { currentConfig -=1; }

        configArray.erase(configArray.begin() + removeConfig);
        commandResponses->push_back({ "mcfr:" + String(removeConfig), debugPrintType::InfoRequest});
    } else
    if (inCommandItem->command == "midiconfigurationcount") {
        commandResponses->push_back({ "mcfc:" + String(configArray.size()), InfoRequest });
    } else
    if (inCommandItem->command == "free") {
        commandResponses->push_back({ "free:" + String(freeram()), InfoRequest });
    } else
    if (inCommandItem->command == "midiconfigurationdefaults") {
        configArray[currentConfig].setDefaults();
        commandResponses->push_back({ "mcfd:1", debugPrintType::InfoRequest});
    } else
    if (inCommandItem->command == "midireceivechannel") {
        if (request) {
            commandResponses->push_back({ "mrc:" + String(configArray[currentConfig].midiRxChannel), InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (!validateNumber(inCommandItem->argument[0].toInt(), 0, 255)) { return eProcessResult::WrongArgumentValue; }
            configArray[currentConfig].midiRxChannel = inCommandItem->argument[0].toInt();
            commandResponses->push_back({ "mrc:" + String(configArray[currentConfig].midiRxChannel), debugPrintType::InfoRequest});
        }
    } else
    if (inCommandItem->command == "midiallnotesoff") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() == 1) {
            midiAllNotesOff();
            commandResponses->push_back({ "mano:1", debugPrintType::InfoRequest});
        }
    } else
    if (inCommandItem->command == "adcread") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        int i = inCommandItem->argument[0].toInt();
        if ((i < 0) || (i > 7)) { return eProcessResult::WrongArgumentValue; }

        int32_t convertedValue;
        if (i < 5) {
            convertedValue = (int32_t) ((float) controlRead->getData(i) * ((float) 65536 / 32767));
        } else {
            convertedValue = (int32_t) ((float) controlRead->getData(i) * ((float) 65536 / 2048));
        }
        if (convertedValue > 65535) { convertedValue = 65535; }
        commandResponses->push_back({ "adcr:" + String(i) + ":" + String(convertedValue) + ":" + String(controlRead->getData(i)), debugPrintType::InfoRequest});
    } else
    if (inCommandItem->command == "adccommandmap") {
        if (!checkArgumentsMin(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        uint8_t channel = inCommandItem->argument[0].toInt();

        if (request) {
            commandResponses->push_back({ "acm:" + String(channel) + ":" + delimitExpression(controlRead->cvInputCommands[channel], true), InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
            controlRead->setADCCommands(channel, stripQuotes(inCommandItem->argument[1]));
            commandResponses->push_back({ "acm:" + String(channel) + ":" + delimitExpression(controlRead->cvInputCommands[channel], true), InfoRequest});
        }
    } else
    if (inCommandItem->command == "adcdefaults") {
        controlRead->setDefaults();
        commandResponses->push_back({ "acd:1", debugPrintType::InfoRequest});
    } else

    if (inCommandItem->command == "adcsettings") {
        if (!checkArgumentsMin(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        uint8_t channel = inCommandItem->argument[0].toInt();
        if (channel > 7) { return eProcessResult::WrongArgumentValue; }

        if (request) {
            commandResponses->push_back({ "adcs:" + String(channel) + ":" + controlRead->getADCAveragerSettings(channel), InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 5)) { return eProcessResult::WrongArgumentCount; }
            if (!controlRead->setADCAveragerSettings(inCommandItem->argument[0].toInt(), inCommandItem->argument[1].toInt(), inCommandItem->argument[2].toInt(), inCommandItem->argument[3].toInt(),
                inCommandItem->argument[4].toInt())) { return eProcessResult::CommandFailed; }
            commandResponses->push_back({ "adcs:" + String(channel) + ":" + controlRead->getADCAveragerSettings(channel), debugPrintType::InfoRequest});
        }
    } else
    if (inCommandItem->command == "expressionparserevaluate") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        updateLocalVariables();

        commandList testCommands("ev:" + inCommandItem->argument[0]);
        testCommands.parseCommandExpressions(expFunctions, expFunctionCount);
        commandResponses->push_back({ "ev: " + String(testCommands.item[0].argument[0]), debugPrintType::InfoRequest});
    } else
    if (inCommandItem->command == "reset") {
        if (!request) {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            if (inCommandItem->argument[0].toInt() == 1) {
                reset();
            }
        }
    } else
    if (inCommandItem->command == "nick") {
        if (request) {
            commandResponses->push_back({ "nick:" + nickName, debugPrintType::InfoRequest});
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            nickName = String(inCommandItem->argument[0]).trim();
            commandResponses->push_back({ "nick:" + nickName, debugPrintType::InfoRequest});
        }
    } else
    if (inCommandItem->command == "nooperation") {
        commandResponses->push_back({ "nop", InfoRequest });
    } else {
        return eProcessResult::NotFound;
    }

    return eProcessResult::Ok;
}
/*
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
*/
/*
bool convertRequest(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses) {
    if (checkArgumentsMin(inCommandItem, commandResponses, 1)) {
        commandItem *newCommandItem = new commandItem(inCommandItem->argument[0]);
        processCommandItems(inCommandItem, serialCommandsMain, sizeof(serialCommandsMain) / sizeof(serialCommandItem));

        for (uint8_t i = 1; i<inCommandItem->argument.size();i++) {
            newCommandItem->argument.push_back(inCommandItem->argument[i]);
        }
        inCommandItem = newCommandItem;
        return true;
    }
    return false;
}
*/
void processSerialCommands() {
    if (ssReader.read()) { commands->addCommands(ssReader.c_str()); }

    if (commands->item.size() == 0) { return; }

    std::vector <commandResponse> commandResponses;

    uint16_t i = 0;
    eProcessResult processResult;

    while (i < commands->item.size()) {
        commandItem *tCommandItem = &commands->item[i];
        bool request = false;

        if ((tCommandItem->command == "requestinfo") || (tCommandItem->command == "rqi")) {
            if (checkArgumentsMin(tCommandItem, &commandResponses, 1)) {
                tCommandItem->command = tCommandItem->argument[0].trim();
                tCommandItem->argument.erase(tCommandItem->argument.begin());
/*
                String testO = "Command: " + tCommandItem->command;
                for (uint8_t i = 1; i<tCommandItem->argument.size();i++) {
                    testO + ":" + tCommandItem->argument[i];
                }
                debugPrintln(testO, debugPrintType::Debug);*/
            } else {
                debugPrintln("Error converting request", debugPrintType::Error);
            }
            request = true;
        }
        processCommandItems(tCommandItem, serialCommandsMain, sizeof(serialCommandsMain) / sizeof(serialCommandItem));

        processResult = eProcessResult::NotFound;

        if (tCommandItem->command == "help") {
            String help = ":\"Below is a list of avaliable commands given in both short and long form, either works. The number and type(s) of arguments are listed afterwards and a brief description. Command usage: command:argument1:argument2 [etc]\"";
            commandResponses.push_back({help, debugPrintType::Help});
            addCommandHelp(serialCommandsMain, sizeof(serialCommandsMain) / sizeof(serialCommandItem), &commandResponses,"");
            processResult = eProcessResult::PassThrough;
        } else
        if (tCommandItem->command == "helphidden") {
            String help = ":\"These commands are hidden for a reason and should not be used unless you REALLY know what you're doing\"";
            commandResponses.push_back({help, debugPrintType::Help});
            processResult = eProcessResult::PassThrough;
        } else {
            processResult = processMainCommands(tCommandItem, &commandResponses, request);
        }

        if ((processResult == eProcessResult::NotFound) || (processResult == eProcessResult::PassThrough)) {
            processResult = farSingle->processSerialCommand(tCommandItem, &commandResponses, request);
        }

        if (processResult == eProcessResult::NotFound) {
            commandResponses.push_back({"Command not found -" + tCommandItem->command + "-", debugPrintType::Error});
        }

        i++;
    }
    commands->item.clear();

    for (i=0; i<commandResponses.size(); i++) {
        debugPrintln(commandResponses[i].response, commandResponses[i].responseType);
    }
}


