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
#ifndef STRINGMODULE_C
#define STRINGMODULE_C

#include "avr_functions.h"
#include <vector>
#include "bowio.hpp"

#include "stringmodule.hpp"

bool stringModule::addBow(char motorRevPin, char motorVoltagePin, char motorDCDCEn, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort,
    char stepHomeSensorPin, char stepCorrectionSensorPin) {

    CalibrationData* calibrationData = new CalibrationData();
    calibrationDataArray.push_back(*calibrationData);

    bowIO* _bowIO = new bowIO(motorRevPin, motorVoltagePin, motorDCDCEn, tachoPin, currentSensePin, motorFaultPin, stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin, stepCorrectionSensorPin);
//    _bowIO->tmc2209ServoStepper->stepServoStepper->stepperID = 1;
    _bowIO->bowPressure->setStepperID(1);
    bowIOArray.push_back(*_bowIO);

    int arrayIndex = bowIOArray.size() - 1;
    ///\todo fix pointers here, major irritation

    bowControl* _bowControl = new bowControl(bowIOArray[arrayIndex], calibrationDataArray[arrayIndex]);
    bowControlArray.push_back(*_bowControl);

    bowControlArray[arrayIndex].commandsMotorFault = "bmr:0,bpr:1";
    bowControlArray[arrayIndex].commandsOverPowerCurrent = "bmr:0,bpr:1,bmes:1000";

    BowControl *bowControlX = new BowControl(motorRevPin, motorVoltagePin, motorDCDCEn, tachoPin, currentSensePin, motorFaultPin, stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
    bowControlArrayX.push_back(*bowControlX);

    bowControlArrayX[arrayIndex].setCommandsMotorFault("bmr:0,bpr:1");
    bowControlArrayX[arrayIndex].setCommandsOverPowerCurrent("bmr:0,bpr:1,bmes:1000");

    calibrate* _calibrate = new calibrate(bowIOArray[arrayIndex], calibrationDataArray[arrayIndex], bowControlArray[arrayIndex]);
    calibrateArray.push_back(*_calibrate);

    return true;
}

bool stringModule::addSolenoid(char _solenoidPin) {
    Solenoid* _solenoid = new Solenoid(_solenoidPin);
    solenoidArray.push_back(*_solenoid);

    _solenoid->solenoidDisengage();

    return true;
}

bool stringModule::addMute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
    MuteControl *muteControl = new MuteControl(stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
    muteArray.push_back(*muteControl);

    muteControl->setPosition(0);
    muteControl->setStepperID(2);

    int arrayIndex = bowIOArray.size() - 1;

    calibrateMute* muteCalibration = new calibrateMute(muteArray[muteArray.size() - 1], bowIOArray[arrayIndex], bowControlArray[arrayIndex]);
    calibrateMuteArray.push_back(*muteCalibration);

    return true;
}

bool stringModule::processSerialCommand_GeneralControl(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegated, commandList *delegatedCommands) {
    if (_commandItem->command == "bow") {
        if (request) {
            commandResponses->push_back({ "b:" + String(currentBowSerial), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            currentBowSerial = clamp(_commandItem->argument[0].toInt(), 0, bowIOArray.size() - 1);
            commandResponses->push_back({"b:" + String(currentBowSerial), InfoRequest});
        }
    }  else
    if (_commandItem->command == "solenoid") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        currentSolenoidSerial = _commandItem->argument[0].toInt();
        commandResponses->push_back({"s:" + String(currentSolenoidSerial), InfoRequest});
/*    } else
    if (_commandItem->command == "bowcontrolfrequency") {
        if (request) {
            commandResponses->push_back({ "bcf:" + String(bowControlArray[currentBowSerial].getPIDTarget()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setPIDTarget(_commandItem->argument[0].toFloat()); //String(serialCommand.substring(1,serialCommand.length())).toFloat());
            commandResponses->push_back({ "bcf:" + String(bowControlArray[currentBowSerial].getPIDTarget()), InfoRequest });
        }
    }else
    if (_commandItem->command == "bowmotorrun") {
        if (request) {
            commandResponses->push_back({ "bmr:" + String(bowControlArray[currentBowSerial].run), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (_commandItem->argument[0].toInt() > 0) {
                bowControlArray[currentBowSerial].run = 1;
                bowIOArray[currentBowSerial].dcMotorControl->enableMotorPower();
            } else {
                bowControlArray[currentBowSerial].run = 0;
                bowIOArray[currentBowSerial].dcMotorControl->disableMotorPower();
            }
            commandResponses->push_back({"bmr:" + String(bowControlArray[currentBowSerial].run), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpid") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0].toInt() > 0) { bowControlArray[currentBowSerial].PIDon = true; } else { bowControlArray[currentBowSerial].PIDon = false;}
        commandResponses->push_back({"bpid:" + String(bowControlArray[currentBowSerial].PIDon), InfoRequest});
    } else
    if (_commandItem->command == "bowcontrolharmonic") {
        if (request) {
            commandResponses->push_back({ "bch:" + String(bowControlArray[currentBowSerial].getHarmonic()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!bowControlArray[currentBowSerial].setHarmonic(_commandItem->argument[0].toInt())) {
                commandResponses->push_back({"ERROR setting harmonic to " + String(_commandItem->argument[0].toInt()), Error});
            } else {
                commandResponses->push_back({ "bch:" + String(bowControlArray[currentBowSerial].getHarmonic()), InfoRequest });
            }
        }
    } else
    if (_commandItem->command == "bowcontrolharmonicadd") {
        if (request) {
            commandResponses->push_back({ "bcha:" + String(bowControlArray[currentBowSerial].getHarmonicAdd()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!bowControlArray[currentBowSerial].setHarmonicAdd(_commandItem->argument[0].toInt())) {
                commandResponses->push_back({"ERROR setting harmonic add to " + String(_commandItem->argument[0].toInt()), Error});
            } else {
                commandResponses->push_back({ "bcha:" + String(_commandItem->argument[0].toInt()), InfoRequest });
            }
        }
    } else
    if (_commandItem->command == "bowcontrolharmonicbase") {
        if (request) {
            commandResponses->push_back({ "bchb:" + String(bowControlArray[currentBowSerial].getHarmonic() + bowControlArray[currentBowSerial].baseNote), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonic(_commandItem->argument[0].toInt() - bowControlArray[currentBowSerial].baseNote);
            commandResponses->push_back({ "bchb:" + String(bowControlArray[currentBowSerial].getHarmonic() + bowControlArray[currentBowSerial].baseNote), InfoRequest });
            commandResponses->push_back({ "bch:" + String(bowControlArray[currentBowSerial].getHarmonic()), InfoRequest });
        }
    } else
    if (_commandItem->command == "bowmotorpower") {
        if (request) {
            commandResponses->push_back({ "bmp:" + String(bowControlArray[currentBowSerial].getBowPower()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setBowPower(_commandItem->argument[0].toInt());
            commandResponses->push_back({"bmp:" + String(bowControlArray[currentBowSerial].manualSpeedPWM), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowcontrolspeedmode") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if ((_commandItem->argument[0].toInt() >= 0) && (_commandItem->argument[0].toInt() <= 1)) {
            bowControlArray[currentBowSerial].speedMode = (_speedMode) (_commandItem->argument[0].toInt());
        }
        commandResponses->push_back({"bcms:" + String(int(bowControlArray[currentBowSerial].speedMode)), InfoRequest});
    } else
    if (_commandItem->command == "bowcontrolharmonicshift") {
        if (request) {
            commandResponses->push_back({ "bchsh:" + String(bowControlArray[currentBowSerial].getHarmonicShift()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShift(_commandItem->argument[0].toInt());
            commandResponses->push_back({ "bchsh:" + String(bowControlArray[currentBowSerial].getHarmonicShift()), InfoRequest });
        }
    } else
    if (_commandItem->command == "bowcontrolharmonicshift5") {
        if (request) {
            commandResponses->push_back({ "bchs5:" + String(bowControlArray[currentBowSerial].getHarmonicShift5()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShift5(_commandItem->argument[0].toInt());
            commandResponses->push_back({ "bchs5:" + String(bowControlArray[currentBowSerial].getHarmonicShift5()), InfoRequest });
        }*/
    } else {
        return false;
    }
    return true;
}

bool stringModule::processSerialCommand_CalibrationsSettings(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegated, commandList *delegatedCommands) {
/*    if (_commandItem->command == "bowpidki") {
        if (request) {
            commandResponses->push_back({ "bpki:" + String(bowControlArray[currentBowSerial].pidController->Ki), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].pidController->Ki = _commandItem->argument[0].toFloat();
            commandResponses->push_back({"bpki:" + String(bowControlArray[currentBowSerial].pidController->Ki), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpidkp") {
        if (request) {
            commandResponses->push_back({ "bpkp:" + String(bowControlArray[currentBowSerial].pidController->Kp), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].pidController->Kp = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            commandResponses->push_back({"bpkp:" + String(bowControlArray[currentBowSerial].pidController->Kp), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpidkd") {
        if (request) {
            commandResponses->push_back({ "bpkd:" + String(bowControlArray[currentBowSerial].pidController->Kd), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].pidController->Kd = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            commandResponses->push_back({"bpkd:" + String(bowControlArray[currentBowSerial].pidController->Kd), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpidr") {
        commandResponses->push_back({"Resetting PID", Command});
        bowControlArray[currentBowSerial].pidController->previousError = 0;
        bowControlArray[currentBowSerial].pidController->pidReset();
    }  else
    if (_commandItem->command == "bowpidintegratorerror") {
        if (request) {
            commandResponses->push_back({ "bpie:" + String(bowControlArray[currentBowSerial].pidController->integratorIgnoreBelow), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].pidController->integratorIgnoreBelow = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            commandResponses->push_back({"bpie:" + String(bowControlArray[currentBowSerial].pidController->integratorIgnoreBelow), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowcontrolfundamental") {
        if (request) {
            commandResponses->push_back({ "bcu:" + String(calibrationDataArray[currentBowSerial].fundamentalFrequency), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].fundamentalFrequency = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"bcu:" + String(calibrationDataArray[currentBowSerial].fundamentalFrequency), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpidmaxerror") {
        if (request) {
            commandResponses->push_back({ "bpme:" + String(bowControlArray[currentBowSerial].pidController->pidMaxError), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].pidController->pidMaxError = _commandItem->argument[0].toInt();
            commandResponses->push_back({"bpme:" + String(bowControlArray[currentBowSerial].pidController->pidMaxError), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotortimeout") {
        if (request) {
            commandResponses->push_back({ "bmt:" + String(bowControlArray[currentBowSerial].bowShutoffTimeout), InfoRequest });
        } else {
            bowControlArray[currentBowSerial].bowShutoffTimeout = _commandItem->argument[0].toInt();
            commandResponses->push_back({"bmt:" + String( bowControlArray[currentBowSerial].bowShutoffTimeout), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotorspeedmax") {
        if (request) {
            commandResponses->push_back({ "bmsx:" + String(calibrationDataArray[currentBowSerial].maxHz), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].maxHz = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"bmsx:" + String(calibrationDataArray[currentBowSerial].maxHz), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotorspeedmin") {
        if (request) {
            commandResponses->push_back({ "bmsi:" + String(calibrationDataArray[currentBowSerial].minHz), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].minHz = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"bmsi:" + String(calibrationDataArray[currentBowSerial].minHz), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotormininertialpwm") {
        if (request) {
            commandResponses->push_back({ "bmmip:" + String(calibrationDataArray[currentBowSerial].minInertialPWM), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].minInertialPWM = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"bmip:" + String(calibrationDataArray[currentBowSerial].minInertialPWM), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpressurepositionmax") {
        if (request) {
            commandResponses->push_back({ "bppx:" + String(calibrationDataArray[currentBowSerial].stallPressure), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].stallPressure = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"bppx:" + String(calibrationDataArray[currentBowSerial].stallPressure), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpressurepositionengage") {
        if (request) {
            commandResponses->push_back({ "bppe:" + String(calibrationDataArray[currentBowSerial].firstTouchPressure), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].firstTouchPressure = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"bppe:" + String(calibrationDataArray[currentBowSerial].firstTouchPressure), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpressurepositionrest") {
        if (request) {
            commandResponses->push_back({ "bppr:" + String(calibrationDataArray[currentBowSerial].restPosition), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].restPosition = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({ "bppr:" + String(calibrationDataArray[currentBowSerial].restPosition), InfoRequest });
        }
    } else
    if (_commandItem->command == "bowharmonicseries") {
        if (request) {
            commandResponses->push_back({ "bhs:" + String(bowControlArray[currentBowSerial].currentHarmonicSeries), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!validateNumber(_commandItem->argument[0].toInt(), 0, bowControlArray[currentBowSerial].harmonicSeriesList.series.size())) { return false; }
            uint8_t hs = _commandItem->argument[0].toInt();
            if (hs > (bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1)) {
                commandResponses->push_back({"Harmonic list doesn't exist " + String(hs), Error});
                return true;
            }
            bowControlArray[currentBowSerial].loadHarmonicSeries(hs);
            commandResponses->push_back({ "bhs:" + String(bowControlArray[currentBowSerial].currentHarmonicSeries), InfoRequest });
        }
    } else
    if (_commandItem->command == "bowharmonicseriesdata") {
        if (request) {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            int ser = _commandItem->argument[0].toInt();
            if (!validateNumber(ser, 0, bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1)) {
                commandResponses->push_back({"Harmonic series out of range", Error});
                return false;
            }
            int j=0;
            String response = "bhsd";
            response += ":" + String(ser) + ":" + bowControlArray[currentBowSerial].harmonicSeriesList.series[ser].Id;
            while (j < int(bowControlArray[currentBowSerial].harmonicSeriesList.series[ser].ratio.size())) {
                response += ":" + String(bowControlArray[currentBowSerial].harmonicSeriesList.series[ser].ratio[j], 4);
                j++;
            }
            commandResponses->push_back({response, InfoRequest});
        } else {
            if (!checkArgumentsMin(_commandItem, commandResponses, 2)) { return false; }
            String response = "Setting harmonic series data:" + String(_commandItem->argument[0].toInt());

            while (_commandItem->argument[0].toInt() > bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1) {
                bowControlArray[currentBowSerial].harmonicSeriesList.addHarmonicSeries();
            }

            bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].ratio.clear();
            bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].Id = String(_commandItem->argument[1]);

            for (int i = 1; i < (int(_commandItem->argument.size()) - 1); i++) {
                response += ":" + _commandItem->argument[i + 1];
                bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].setHarmonic(i - 1, _commandItem->argument[i + 1].toFloat());
            }

            if (bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio.size() == 0) {
                debugPrintln("Current harmonic series is zero, reloading", debugPrintType::Debug);
                bowControlArray[currentBowSerial].loadHarmonicSeries(bowControlArray[currentBowSerial].currentHarmonicSeries);
            }

            bowControlArray[currentBowSerial].updateHarmonicData();
            commandResponses->push_back({response, InfoRequest});
        }
    } else
    if (_commandItem->command == "bowharmonicseriesratio") {
        if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
        bowControlArray[currentBowSerial].currentHarmonicSeriesData.setHarmonic(
            _commandItem->argument[0].toInt(), _commandItem->argument[1].toFloat());

        bowControlArray[currentBowSerial].updateHarmonicData();

        commandResponses->push_back({"bhsr:" + String(_commandItem->argument[0].toInt()) + ":" + String(_commandItem->argument[1].toFloat()), InfoRequest});
    } else
    if (_commandItem->command == "bowharmonicseriesratioremove") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio.size() < 2) {
            commandResponses->push_back({"Cannot remove all harmonics", Error});
            return false;
        }

        int i = _commandItem->argument[0].toInt();
        bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio.erase(bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio.begin() + i);
        bowControlArray[currentBowSerial].updateHarmonicData();

        commandResponses->push_back({"bhsrr:" + String(i), InfoRequest});
    } else
    if (_commandItem->command == "bowharmonicseriescount") {
        String response = "bhsc:" + String(bowControlArray[currentBowSerial].harmonicSeriesList.series.size());
        for (int i=0; i<bowControlArray[currentBowSerial].harmonicSeriesList.series.size(); i++) {
            response += ":" + bowControlArray[currentBowSerial].harmonicSeriesList.series[i].Id;
        }
        commandResponses->push_back({ response, InfoRequest });
    } else
    if (_commandItem->command == "bowharmonicseriessave") {
        if (request) {
            commandResponses->push_back({ "bhss:" + String(bowControlArray[currentBowSerial].harmonicSeriesList.series.size()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
            if (!validateNumber(_commandItem->argument[0].toInt(), 0, bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1, true)) {
                bowControlArray[currentBowSerial].harmonicSeriesList.addHarmonicSeries(_commandItem->argument[1], bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio);
                bowControlArray[currentBowSerial].currentHarmonicSeries = bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1;
            } else {
                bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()] = bowControlArray[currentBowSerial].currentHarmonicSeriesData;
                bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].Id = _commandItem->argument[1];
                bowControlArray[currentBowSerial].currentHarmonicSeries = _commandItem->argument[0].toInt();
            }
            commandResponses->push_back({ "bhss:" + String(bowControlArray[currentBowSerial].currentHarmonicSeries), InfoRequest });
        }
    } else
    if (_commandItem->command == "bowharmonicseriesremove") {
        if (request) {
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            int listNo = _commandItem->argument[0].toInt();
            debugPrintln("Attempting to remove list " + String(listNo), debugPrintType::Debug);
            if (!validateNumber(listNo, 0, bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1, true)) { return false; }

            bowControlArray[currentBowSerial].harmonicSeriesList.series.erase(
                bowControlArray[currentBowSerial].harmonicSeriesList.series.begin() + listNo);

            if ((bowControlArray[currentBowSerial].currentHarmonicSeries >= listNo) && (bowControlArray[currentBowSerial].currentHarmonicSeries > 0)) {
                bowControlArray[currentBowSerial].currentHarmonicSeries--;
            }

            commandResponses->push_back({ "bhsrm:" + String(bowControlArray[currentBowSerial].currentHarmonicSeries), InfoRequest });
        }
    } else
    if (_commandItem->command == "bowmotorvoltage") {
        if (request) {
            commandResponses->push_back({ "bmv:" + String(bowIOArray[currentBowSerial].dcMotorControl->getBowMotorVoltage()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].setBowMotorVoltage(_commandItem->argument[0].toFloat());
            commandResponses->push_back({"bmv:" + String(bowIOArray[currentBowSerial].getBowMotorVoltage()), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotorcurrent") {
        if (request) {
            commandResponses->push_back({ "bmc:" + String(bowIOArray[currentBowSerial].getBowCurrent()), InfoRequest });
        } else {
        }
    } else
    if (_commandItem->command == "bowcontrolharmonicbasenote") {
        if (request) {
            commandResponses->push_back({ "bchbn:" + String(bowControlArray[currentBowSerial].baseNote), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setBaseNote(_commandItem->argument[0].toInt());
            //commandResponses->push_back({"Setting base note to " + String(bowControlArray[currentBowSerial].baseNote), Command});
            commandResponses->push_back({ "bchbn:" + String(bowControlArray[currentBowSerial].baseNote), InfoRequest });
        }
    } else
    if (_commandItem->command == "bowmotorcurrentlimit") {
        if (request) {
            commandResponses->push_back({ "bmcl:" + String(bowIOArray[currentBowSerial].bowMotorCurrentLimit), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].bowMotorCurrentLimit = _commandItem->argument[0].toFloat();
            commandResponses->push_back({"bmcl" + String(bowIOArray[currentBowSerial].bowMotorCurrentLimit), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotorpowerlimit") {
        if (request) {
            commandResponses->push_back({ "bmpl:" + String(bowIOArray[currentBowSerial].bowMotorWattage), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].bowMotorWattage = _commandItem->argument[0].toFloat();
            commandResponses->push_back({"bmpl:" + String(bowIOArray[currentBowSerial].bowMotorCurrentLimit), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotorfaultcommands") {
        if (request) {
            commandResponses->push_back({ "bmfc:" + String(bowControlArray[currentBowSerial].commandsMotorFault), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].commandsMotorFault = _commandItem->argument[0];
            commandResponses->push_back({"bmfc:" + String(bowControlArray[currentBowSerial].commandsMotorFault), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotoroverpowercommands") {
        if (request) {
            commandResponses->push_back({ "bmopc:" + String(bowControlArray[currentBowSerial].commandsOverPowerCurrent), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].commandsOverPowerCurrent = _commandItem->argument[0];
            commandResponses->push_back({"bmopc:" + String(bowControlArray[currentBowSerial].commandsOverPowerCurrent), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowmotoremergencystop") {
        if (request) {
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].emergencyBowDisable(_commandItem->argument[0].toInt());
            commandResponses->push_back({"bmes:" + _commandItem->argument[0], InfoRequest});
            commandResponses->push_back({"EMERGENCY STOP! Cooling down for " + _commandItem->argument[0] + " ms", Command});
        }
    } else
    if (_commandItem->command == "bowcontrolharmonicshiftrange") {
        if (request) {
            commandResponses->push_back({ "bchsr:" + String(bowControlArray[currentBowSerial].getHarmonicShiftRange()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShiftRange(_commandItem->argument[0].toFloat());
            //commandResponses->push_back({"Setting harmonic shift range to " + String(bowControlArray[currentBowSerial].getHarmonicShiftRange()), Command});
            commandResponses->push_back({ "bchsr:" + String(bowControlArray[currentBowSerial].getHarmonicShiftRange()), InfoRequest });
        }
    } else
    if (_commandItem->command == "bowpressuremodulationspeed") {
        if (request) {
            commandResponses->push_back({ "bpms:" + String(bowControlArray[currentBowSerial].bowSpeedWhileEngaged), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            uint16_t bes = _commandItem->argument[0].toInt();
            if (bes > bowControlArray[currentBowSerial].bowSpeedToEngage) { bes = bowControlArray[currentBowSerial].bowSpeedToEngage; }
            bowControlArray[currentBowSerial].bowSpeedWhileEngaged = bes;
            commandResponses->push_back({"bpms:" + String(bowControlArray[currentBowSerial].bowSpeedWhileEngaged), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowpressureengagespeed") {
        if (request) {
            commandResponses->push_back({ "bpes:" + String(bowControlArray[currentBowSerial].bowSpeedToEngage), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            uint16_t bes = _commandItem->argument[0].toInt();
            if (bes > 100) { bes = 100; }
            bowControlArray[currentBowSerial].bowSpeedToEngage = bes;
            commandResponses->push_back({"bpes:" + String(bowControlArray[currentBowSerial].bowSpeedToEngage), InfoRequest});
        }
    } else
    if (_commandItem->command == "bowhome") {
        if (!bowControlArray[currentBowSerial].bowIOConnect->homeBow()) {
            commandResponses->push_back({"bowhome:error", Error});
        } else {
            commandResponses->push_back({"bowhome:ok", InfoRequest});
        }
    } else {
        return false;
    }*/
    return true;
}

bool stringModule::processSerialCommand_StatusTesting(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegated, commandList *delegatedCommands) {
/*    if (_commandItem->command == "bowstatus") {
        commandResponses->push_back({"Set frequency " + String(bowControlArray[currentBowSerial].getPIDTarget()) + " Hz, ", TextInfo});
        commandResponses->push_back({"Read frequency " + String(bowIOArray[currentBowSerial].averageFreq()) + " Hz", TextInfo});
        commandResponses->push_back({" P gain " + String(bowControlArray[currentBowSerial].pidController->Kp), TextInfo});
        commandResponses->push_back({" I gain " + String(bowControlArray[currentBowSerial].pidController->Ki), TextInfo});
        commandResponses->push_back({" D gain " + String(bowControlArray[currentBowSerial].pidController->Kd), TextInfo});
        commandResponses->push_back({" ignore below " + String(bowControlArray[currentBowSerial].pidController->integratorIgnoreBelow), TextInfo});
        commandResponses->push_back({" max error " + String(bowControlArray[currentBowSerial].pidController->pidMaxError), TextInfo});
    } else
    if (_commandItem->command == "bowdebugmeasuretimetotarget") {
        bowControlArray[currentBowSerial].pidController->measureTimeToTarget(_commandItem->argument[0].toInt());
    }  else*/
    if (_commandItem->command == "pickupstringfrequency") {
        if (audioFrequencyAvaliable()) {
            commandResponses->push_back({ "psf:" + String(audioFrequency(),1), InfoRequest });
        } else {
            commandResponses->push_back({ "psf: 0", InfoRequest });
        }
    }  else
    if (_commandItem->command == "pickupaudiopeak") {
        commandResponses->push_back({ "pap:" + String(audioPeakAmplitude()), InfoRequest });
    }  else
    if (_commandItem->command == "pickupaudiorms") {
        commandResponses->push_back({ "par:" + String(audioRMSAmplitude()), InfoRequest });
    }  else
/*    if (_commandItem->command == "bowmotorfrequency") {
        if (request) {
            commandResponses->push_back({ "bmf:" + String(bowIOArray[currentBowSerial].averageFreq()), InfoRequest });
        }
    }  else
    if (_commandItem->command == "bowpidpeakerror") {
        if (request) {
            commandResponses->push_back({ "bpperr:" + String(bowControlArray[currentBowSerial].pidController->getPIDPeakError()), InfoRequest });
        }
    }  else*/
    if (_commandItem->command == "nooperation") {
        commandResponses->push_back({ "nop", InfoRequest });
    } else {
        return false;
    }
    return true;
}
/*
bool stringModule::processSerialCommand_BowActuator(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegated,
    commandList *delegatedCommands) {

    if (_commandItem->command == "bowactuator") {
        if (request) {
            commandResponses->push_back({ "ba:" + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!bowControlArray[currentBowSerial].bowActuators->loadBowActuator(_commandItem->argument[0].toInt())) {
                return false;
            }
            commandResponses->push_back({"ba:" + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), InfoRequest});
        }

    } else
    if (_commandItem->command == "bowactuatorsave") {
        if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
        if (!bowControlArray[currentBowSerial].bowActuators->saveBowActuator(_commandItem->argument[0].toInt(), _commandItem->argument[1])) {
            return false;
        }
        uint8_t actuator = bowControlArray[currentBowSerial].bowActuators->getBowActuator();
        commandResponses->push_back({"bas:" + String(actuator) + ":" + bowControlArray[currentBowSerial].bowActuators->getBowActuatorID(actuator), InfoRequest});
        //commandResponses->push_back({"Saving data for actuator " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), Command});
    }  else
    if (_commandItem->command == "bowactuatordata") {
        int8_t bowIndex;
        if (request) {
            bowIndex = bowControlArray[currentBowSerial].bowActuators->getBowActuator();
            if (checkArguments(_commandItem, commandResponses, 1, true)) {
                bowIndex = _commandItem->argument[0].toInt();
                if (bowIndex > (bowControlArray[currentBowSerial].bowActuators->getBowActuatorCount() - 1)) {
                    return false;
                }
            }
        } else {
            if (!checkArguments(_commandItem, commandResponses, 5)) { return false; }
            bowControlArray[currentBowSerial].bowActuators->setBowActuatorData(_commandItem->argument[0].toInt(), _commandItem->argument[1].toInt(),
                _commandItem->argument[2].toInt(), _commandItem->argument[3].toInt(), _commandItem->argument[4]);
            bowIndex = _commandItem->argument[0].toInt();
        }
        commandResponses->push_back({ "bad:" + String(bowIndex) + ":" +
            String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorFirstTouchPressure(bowIndex)) + ":"  +
            String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorStallPressure(bowIndex)) + ":" +
            String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorRestPosition(bowIndex)) + ":" +
            bowControlArray[currentBowSerial].bowActuators->getBowActuatorID(bowIndex), InfoRequest });
    }  else
    if (_commandItem->command == "bowactuatorcount") {
        commandResponses->push_back({ "bac:" + String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorCount()), InfoRequest });
    } else
    if (_commandItem->command == "bowactuatorremove") {
        if (request) {
            return false;
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!bowControlArray[currentBowSerial].bowActuators->removeBowActuator(_commandItem->argument[0].toInt())) { return false; }
            commandResponses->push_back({"bar:" + String(_commandItem->argument[0].toInt()), InfoRequest});
        }
    } else {
        return false;
    }
    return true;
}
*/
bool stringModule::processSerialCommand_Calibrate(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request, bool delegated, commandList *delegatedCommands) {

    if (_commandItem->command == "mutecalibrate") {
        if (!calibrateMuteArray[currentBowSerial].calibrateAll()) {
            commandResponses->push_back({"mca:error", InfoRequest});
        } else {
            commandResponses->push_back({"mca:ok", InfoRequest});
        }
    } else     if (_commandItem->command == "bowcalibrateall") {
        if (calibrateArray[currentBowSerial].calibrateAll() & calibrateMuteArray[currentBowSerial].calibrateAll()) {
            commandResponses->push_back({"bca:ok", InfoRequest});
        } else {
            commandResponses->push_back({"bca:error", InfoRequest});
        }
    } else
    if (_commandItem->command == "bowcalibratepressure") {
        if (!calibrateArray[currentBowSerial].findMinMaxPressure()) {
            commandResponses->push_back({"bcp:error", debugPrintType::Error});
        } else {
            commandResponses->push_back({"bcp:ok", debugPrintType::InfoRequest});
        }
    } else
    if (_commandItem->command == "bowcalibratespeed") {
        if (!calibrateArray[currentBowSerial].findMinMaxSpeedPWM()) {
            commandResponses->push_back({"bcs:error", Error});
        } else {
            commandResponses->push_back({"bcs:ok", InfoRequest});
        }
    } else
    if (_commandItem->command == "bowcalibratespeedpid") {
        commandResponses->push_back({"Finding min/max speed with the PID", Command});
        if (!calibrateArray[currentBowSerial].findMinMaxSpeedPID()) {
            commandResponses->push_back({"bcs:error", Error});
        } else {
            commandResponses->push_back({"bcs:ok", InfoRequest});
        }
    } else {
        return false;
    }
    return true;
}

void stringModule::writeToSlave(String command) {
    if (slaveSerialOut != nullptr) {
        slaveSerialOut->println(command.c_str());
    } else {
        debugPrintln("Slave serial device is NULL", Error);
    }
}

bool stringModule::processSerialCommand(commandList *commands, uint16_t *index, std::vector<commandResponse> *commandResponses, bool delegated) {
    commandItem *_commandItem = &commands->item[*index];

    // Replace all short form commands with long for uniform processing later
    processCommandItems(_commandItem, serialCommandsStringModule, sizeof(serialCommandsStringModule) / sizeof(serialCommandItem));

    bool request = false;

    // If request, strip 'rqi' and make argument[0] command then shift all arguments down 1
    if (_commandItem->command == "requestinfo") {
        if (!checkArgumentsMin(_commandItem, commandResponses, 1)) { return false; }
//        String command = _commandItem->argument[0];
/*        for (int i = 0; i < int(sizeof(serialCommandsStringModule) / sizeof(serialCommandItem)); i++) {
            if (serialCommandsStringModule[i].shortCommand == command) {
                command = serialCommandsStringModule[i].longCommand;
                break;
            }
        }
        _commandItem = new commandItem(command);
*/
        commandItem *newCommandItem = new commandItem(_commandItem->argument[0]);
        String debugText = "Rebuilding rqi command as " + newCommandItem->command;
        processCommandItems(newCommandItem, serialCommandsStringModule, sizeof(serialCommandsStringModule) / sizeof(serialCommandItem));
        for (uint8_t i = 1; i<_commandItem->argument.size();i++) {
            newCommandItem->argument.push_back(_commandItem->argument[i]);
            debugText += ":" + _commandItem->argument[i];
        }
        _commandItem = newCommandItem;
        debugPrintln(debugText, debugPrintType::Debug);

        request = true;
    }

    // Check help first to save time
    if (_commandItem->command == "help") {
        commandResponses->push_back({":\"These commands are specific for the currently selected string module\"", debugPrintType::Help});
        addCommandHelp(serialCommandsStringModule, sizeof(serialCommandsStringModule) / sizeof(serialCommandItem), commandResponses, "[str]:");
        addCommandHelp(serialCommandsMute, sizeof(serialCommandsMute) / sizeof(serialCommandItem), commandResponses, "[str]:");
        addCommandHelp(serialCommandsSolenoid, sizeof(serialCommandsSolenoid) / sizeof(serialCommandItem), commandResponses, "[str]:");
    }
    if (_commandItem->command == "helphidden") {
//        commandResponses->push_back({":\"These commands are for the Mute\"", debugPrintType::Help});
        addCommandHelp(serialCommandsMuteHidden, sizeof(serialCommandsMuteHidden) / sizeof(serialCommandItem), commandResponses, "[str]:");
    } else/*
    if (processSerialCommand_GeneralControl(_commandItem, commandResponses, request, delegated)) {
    } else
    if (processSerialCommand_CalibrationsSettings(_commandItem, commandResponses, request, delegated)) {
    } else
//    if (processSerialCommand_MuteControl(_commandItem, commandResponses, request, delegated)) {
//    } else
    if (processSerialCommand_StatusTesting(_commandItem, commandResponses, request, delegated)) {
    } else
    if (processSerialCommand_BowActuator(_commandItem, commandResponses, request, delegated)) {
    } else
    if (processSerialCommand_Calibrate(_commandItem, commandResponses, request, delegated)) {
    } else*/
    if (harmonicSeriesHandler->processSerialCommand(_commandItem, commandResponses, request, delegated)) {
    } else
    if (muteArray[currentBowSerial].processSerialCommand(_commandItem, commandResponses, request, delegated)) {
    } else
    if (solenoidArray[currentBowSerial].processSerialCommand(_commandItem, commandResponses, request, delegated)) {
    } else
    if (muteArray[currentBowSerial].processSerialCommandHidden(_commandItem, commandResponses, request, delegated)) {
    /*else
    if (_commandItem->command == "help") {
        commandResponses->push_back({":\"These commands are specific for the currently selected string module\"", debugPrintType::Help});
        addCommandHelp(serialCommandsStringModule, sizeof(serialCommandsStringModule) / sizeof(serialCommandItem), commandResponses, "[str]:");
    } else
    if (_commandItem->command == "requestinfo") {
        if (!processRequestCommand(_commandItem, commandResponses, delegated)) { return false; }*/
    } else
    if (bowControlArrayX[currentBowSerial].processSerialCommand(_commandItem, commandResponses, request, delegated)) {
    } else {
        return false;
    }
    return true;
};

stringModule::stringModule() {
    bowIOArray.reserve(1);
    bowControlArray.reserve(1);
    calibrationDataArray.reserve(1);
    harmonicSeriesHandler = new HarmonicSeriesHandler( ((void*) &frequencyChanged) );
};

void stringModule::frequencyChanged() {
    debugPrintln("Frequency changed", debugPrintType::Debug);
}

void stringModule::updateString() {
    if (bowControlArray.size() > 0) {
        for (int i = 0; i < int(bowControlArray.size()); i++) {
            bowControlArray[i].updateString();
        }
    }
    if (solenoidArray.size() > 0) {
        for (int i = 0; i < int(solenoidArray.size()); i++) {
            solenoidArray[i].updateSolenoid();
        }
    }
    if (muteArray.size() > 0) {
        for (int i = 0; i < int(muteArray.size()); i++) {
//            muteArray[i].mute->updateMute();
            muteArray[i].update();
        }
    }
}


#endif
