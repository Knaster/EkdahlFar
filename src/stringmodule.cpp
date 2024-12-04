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
#include "bowio.h"

#include "stringmodule.hpp"

bool stringModule::addBow(char motorRevPin, char motorVoltagePin, char motorDCDCEn, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort,
    char stepHomeSensorPin, char stepCorrectionSensorPin) {

    CalibrationData* calibrationData = new CalibrationData();
    calibrationDataArray.push_back(*calibrationData);

    bowIO* _bowIO = new bowIO(motorRevPin, motorVoltagePin, motorDCDCEn, tachoPin, currentSensePin, motorFaultPin, stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin, stepCorrectionSensorPin);
    _bowIO->stepServoStepper->stepperID = 1;
    bowIOArray.push_back(*_bowIO);

    int arrayIndex = bowIOArray.size() - 1;
    ///\todo fix pointers here, major irritation

    bowControl* _bowControl = new bowControl(bowIOArray[arrayIndex], calibrationDataArray[arrayIndex]);
    bowControlArray.push_back(*_bowControl);

    bowControlArray[arrayIndex].commandsMotorFault = "bmr:0,bpr:1";
    bowControlArray[arrayIndex].commandsOverPowerCurrent = "bmr:0,bpr:1";

    calibrate* _calibrate = new calibrate(bowIOArray[arrayIndex], calibrationDataArray[arrayIndex], bowControlArray[arrayIndex]);
    calibrateArray.push_back(*_calibrate);

    return true;
}

bool stringModule::addSolenoid(char _solenoidPin) {
    solenoid* _solenoid = new solenoid(_solenoidPin);
    solenoidArray.push_back(*_solenoid);

    _solenoid->solenoidDisengage();

    return true;
}

bool stringModule::addMute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
    mute* mmute = new mute(stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
    muteArray.push_back(*mmute);

    mmute->setTilt(0);
    mmute->stepServoStepper->stepperID = 2;

    int arrayIndex = bowIOArray.size() - 1;

    //calibrateMute* muteCalibration = new calibrateMute(*mmute, bowIOArray[arrayIndex], bowControlArray[arrayIndex]);
    calibrateMute* muteCalibration = new calibrateMute(muteArray[muteArray.size() - 1], bowIOArray[arrayIndex], bowControlArray[arrayIndex]);
    calibrateMuteArray.push_back(*muteCalibration);
/*
    int arrayIndex = bowIOArray.size() - 1;
    if (arrayIndex < 0) {
        debugPrintln("arrayIndex below zero in addMute", debugPrintType::Error);
        return false;
    }
*/
    return true;
}

bool stringModule::processSerialCommand_GeneralControl(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
    if (_commandItem->command == "bow") {
        if (request) {
            commandResponses->push_back({ "b:" + String(currentBowSerial), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            currentBowSerial = clamp(_commandItem->argument[0].toInt(), 0, bowIOArray.size() - 1);
            commandResponses->push_back({"Setting current bow to " + String(currentBowSerial), Command});
        }
    }  else
    if (_commandItem->command == "bowcontrolfrequency") {
        if (request) {
            commandResponses->push_back({ "bcf:" + String(bowControlArray[currentBowSerial].getPIDTarget()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setPIDTarget(_commandItem->argument[0].toFloat()); //String(serialCommand.substring(1,serialCommand.length())).toFloat());
            commandResponses->push_back({"Setting frequency " + String(bowControlArray[currentBowSerial].getPIDTarget()) + "Hz", Command});
        }
    }else
    if (_commandItem->command == "bowmotorrun") {
        if (request) {
            commandResponses->push_back({ "bmr:" + String(bowControlArray[currentBowSerial].run), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (_commandItem->argument[0].toInt() > 0) {
                bowControlArray[currentBowSerial].run = 1;
                bowIOArray[currentBowSerial].enableBowPower();
            } else {
                bowControlArray[currentBowSerial].run = 0;
                bowIOArray[currentBowSerial].disableBowPower();
            }
            //debugPrintln("Setting run to " + String(bowControlArray[currentBowSerial].run), Command);
            commandResponses->push_back({"Setting run to " + String(bowControlArray[currentBowSerial].run), Command});
        }
    } else
    if (_commandItem->command == "bowpid") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0].toInt() > 0) { bowControlArray[currentBowSerial].PIDon = true; } else { bowControlArray[currentBowSerial].PIDon = false;}
        commandResponses->push_back({"Setting PID On to " + String(bowControlArray[currentBowSerial].PIDon), Command});
    } else
    if (_commandItem->command == "bowcontrolharmonic") {
        if (request) {
            commandResponses->push_back({ "bch:" + String(bowControlArray[currentBowSerial].getHarmonic()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!bowControlArray[currentBowSerial].setHarmonic(_commandItem->argument[0].toInt())) {
                commandResponses->push_back({"ERROR setting harmonic to " + String(_commandItem->argument[0].toInt()), Error});
            } else {
                commandResponses->push_back({"Setting harmonic to " + String(_commandItem->argument[0].toInt()), Command});
            }
        }
    } else
    if (_commandItem->command == "bowcontrolharmonicadd") {
        if (request) {
            commandResponses->push_back({ "bcha:" + String(bowControlArray[currentBowSerial].getHarmonic()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!bowControlArray[currentBowSerial].setHarmonicAdd(_commandItem->argument[0].toInt())) {
                commandResponses->push_back({"ERROR setting harmonic add to " + String(_commandItem->argument[0].toInt()), Error});
            } else {
                commandResponses->push_back({"Setting harmonic add to " + String(_commandItem->argument[0].toInt()), Command});
            }
        }
    } else
    if (_commandItem->command == "bowcontrolharmonicbase") {
        if (request) {
//                commandResponses->push_back({ "h:" + String(bowControlArray[currentBowSerial].getHarmonic()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonic(_commandItem->argument[0].toInt() - bowControlArray[currentBowSerial].baseNote);
            commandResponses->push_back({"Setting harmonic to " + String(_commandItem->argument[0].toInt()  - bowControlArray[currentBowSerial].baseNote), Command});
        }
    } else
    if (_commandItem->command == "solenoid") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        currentSolenoidSerial = _commandItem->argument[0].toInt();
        commandResponses->push_back({"Setting current solenoid to " + String(currentSolenoidSerial), Command});
    } else
    if (_commandItem->command == "solenoidengage") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        solenoidArray[currentSolenoidSerial].solenoidEngage(_commandItem->argument[0].toInt());
        commandResponses->push_back({"Setting solenoid engagement to " + String(_commandItem->argument[0].toInt()), Command});
    } else
    if (_commandItem->command == "solenoiddisengage") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        solenoidArray[currentSolenoidSerial].solenoidDisengage(_commandItem->argument[0].toInt());
        commandResponses->push_back({"Setting solenoid disengagement to " + String(_commandItem->argument[0].toInt()), Command});
    } else
    if (_commandItem->command == "solenoidengageduration") {
        if (request) {
            commandResponses->push_back({ "sed:" + String(solenoidArray[currentSolenoidSerial].solenoidEngageDuration), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            solenoidArray[currentSolenoidSerial].solenoidEngageDuration = _commandItem->argument[0].toInt();
            commandResponses->push_back({"Setting solenoid engage duration to " + String(_commandItem->argument[0].toInt()), Command});
        }
    } else
    if (_commandItem->command == "setmanualpressure") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        bowControlArray[currentBowSerial].setManualTilt(_commandItem->argument[0].toInt());
        commandResponses->push_back({"Setting pressure to " + String(_commandItem->argument[0].toInt()), Command});
    } else
    if (_commandItem->command == "bowpressurebaseline") {
        if (request) {
            commandResponses->push_back({ "bpb:" + String(bowControlArray[currentBowSerial].getPressureBaseline()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setPressureBaseline(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting pressure baseline to " + String(_commandItem->argument[0].toInt()), Command});
        }
    } else
    if (_commandItem->command == "bowpressuremodifier") {
        if (request) {
            commandResponses->push_back({ "bpm:" + String(bowControlArray[currentBowSerial].getPressureModifier()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setPressureModifier(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting pressure modifier to " + String(_commandItem->argument[0].toInt()), Command});
        }
    } else
    if (_commandItem->command == "bowpressurerest") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        bowControlArray[currentBowSerial].bowRest(_commandItem->argument[0].toInt());
        commandResponses->push_back({"Setting bow rest position to " + String(_commandItem->argument[0].toInt()), Command});
    } else
    if (_commandItem->command == "bowpressureengage") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        bowControlArray[currentBowSerial].bowEngage(_commandItem->argument[0].toInt());
        commandResponses->push_back({"Setting bow engage to " + String(_commandItem->argument[0].toInt()), Command});
    } else
    if (_commandItem->command == "bowpressurehold") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0].toInt() > 0) { bowControlArray[currentBowSerial].setHold(true); } else { bowControlArray[currentBowSerial].setHold(false);}
        commandResponses->push_back({"Setting bow hold to " + String(_commandItem->argument[0].toInt()), Command});
    } else
    /*    if (_commandItem->command == "mute") {
      bowControlArray[currentBowSerial].bowMute(_commandItem->argument[0].toInt());
      debugPrintln("Setting bow mute to " + String(_commandItem->argument[0].toInt()), Command);
    } else
    if (_commandItem->command == "muteforce") {
      bowControlArray[currentBowSerial].muteForce = _commandItem->argument[0].toInt();
      debugPrintln("Setting mute force to " + String(bowControlArray[currentBowSerial].muteForce), Command);
    } else
    if (_commandItem->command == "muteperiod") {
      bowControlArray[currentBowSerial].mutePeriod = _commandItem->argument[0].toInt();
      debugPrintln("Setting mute perid to " + String(bowControlArray[currentBowSerial].muteForce) + " ms", Command);
    } else */
    if (_commandItem->command == "bowmotorpower") {
        if (request) {
            commandResponses->push_back({ "bmp:" + String(bowControlArray[currentBowSerial].getBowPower()), InfoRequest });
        } else {
//                bowControlArray[currentBowSerial].manualSpeedPWM =_commandItem->argument[0].toInt();
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setBowPower(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting bow power to " + String(bowControlArray[currentBowSerial].manualSpeedPWM), Command});
        }
    } else
    if (_commandItem->command == "bowcontrolspeedmode") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if ((_commandItem->argument[0].toInt() >= 0) && (_commandItem->argument[0].toInt() <= 1)) {
            bowControlArray[currentBowSerial].speedMode = (_speedMode) (_commandItem->argument[0].toInt());
        }
        commandResponses->push_back({"Setting speed mode to " + String(int(bowControlArray[currentBowSerial].speedMode)), Command});
    } else
    if (_commandItem->command == "bowcontrolharmonicshift") {
        if (request) {
            commandResponses->push_back({ "bchsh:" + String(bowControlArray[currentBowSerial].getHarmonicShift()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShift(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting harmonic shift to " + String(bowControlArray[currentBowSerial].getHarmonicShift()), Command});
        }
    } else
    if (_commandItem->command == "bowcontrolharmonicshift5") {
        if (request) {
            commandResponses->push_back({ "bchs5:" + String(bowControlArray[currentBowSerial].getHarmonicShift5()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShift5(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting harmonic shift 5 to " + String(bowControlArray[currentBowSerial].getHarmonicShift5()), Command});
        }
    } else {
        return false;
    }
    return true;
}

bool stringModule::processSerialCommand_CalibrationsSettings(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
    if (_commandItem->command == "bowpidki") {
        if (request) {
            commandResponses->push_back({ "bpki:" + String(bowControlArray[currentBowSerial].Ki), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].Ki = _commandItem->argument[0].toFloat();
            commandResponses->push_back({"Setting PID Ki value to " + String(bowControlArray[currentBowSerial].Ki), Command});
        }
    } else
    if (_commandItem->command == "bowpidkp") {
        if (request) {
            commandResponses->push_back({ "bpkp:" + String(bowControlArray[currentBowSerial].Kp), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].Kp = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting PID Kp value to " + String(bowControlArray[currentBowSerial].Kp), Command});
        }
    } else
    if (_commandItem->command == "bowpidkd") {
        if (request) {
            commandResponses->push_back({ "bpkd:" + String(bowControlArray[currentBowSerial].Kd), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].Kd = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting PID Kd value to " + String(bowControlArray[currentBowSerial].Kd), Command});
        }
    } else
    if (_commandItem->command == "bowpidr") {
        commandResponses->push_back({"Resetting PID", Command});
        bowControlArray[currentBowSerial].previousError = 0;
        bowControlArray[currentBowSerial].pidReset();
    }  else
    if (_commandItem->command == "bowpidintegratorerror") {
        if (request) {
            commandResponses->push_back({ "bpie:" + String(bowControlArray[currentBowSerial].integratorIgnoreBelow), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].integratorIgnoreBelow = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting error ignore to " + String(bowControlArray[currentBowSerial].integratorIgnoreBelow), Command});
        }
    } else
    if (_commandItem->command == "bowcontrolfundamental") {
        if (request) {
            commandResponses->push_back({ "bcu:" + String(calibrationDataArray[currentBowSerial].fundamentalFrequency), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].fundamentalFrequency = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting fundamental frequency to " + String(calibrationDataArray[currentBowSerial].fundamentalFrequency), Command});
        }
    } else
    if (_commandItem->command == "bowcalibrateall") {
        calibrateArray[currentBowSerial].calibrateAll();
    } else
    if (_commandItem->command == "bowcalibratepressure") {
        //debugPrintln("Finding min/max pressure", Command);
        if (!calibrateArray[currentBowSerial].findMinMaxPressure()) {
            //debugPrintln("Find min/max pressure FAILED", Error);
            commandResponses->push_back({"bcp:error", debugPrintType::Error});
        } else {
            commandResponses->push_back({"bcp:ok", debugPrintType::InfoRequest});
        }
    } else
    if (_commandItem->command == "bowcalibratespeed") {
        //debugPrintln("Finding min/max speed", Command);
        if (!calibrateArray[currentBowSerial].findMinMaxSpeedPWM()) {
            commandResponses->push_back({"Find min/max speed FAILED", Error});
        }
    } else
    if (_commandItem->command == "bowcalibratespeedpid") {
        commandResponses->push_back({"Finding min/max speed with the PID", Command});
        if (!calibrateArray[currentBowSerial].findMinMaxSpeedPID()) {
            commandResponses->push_back({"Find min/max speed with the PID FAILED", Error});
        }
    } else
/*        if (_commandItem->command == "calibratesave") {
        debugPrintln("Saving calibration data", Command);
        calibrateArray[currentBowSerial].saveParams();
    } else*/
/*        if (_commandItem->command == "calibratelist") {
        debugPrintln("Calibration data:", Command);
        calibrateArray[currentBowSerial].printCalibrationData();
    }  else*/
    if (_commandItem->command == "bowcalibratestablethreshold") {
        bowControlArray[currentBowSerial].elapsedTimeThreshold = _commandItem->argument[0].toFloat(); // String(serialCommand.substring(2,serialCommand.length())).toFloat();
        commandResponses->push_back({"Setting elapsed stable threshold to " + String(bowControlArray[currentBowSerial].elapsedTimeThreshold) + "ms", Command});
    } else
/*    if (_commandItem->command == "recoverrate") {
        bowControlArray[currentBowSerial].recoverRate = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
        debugPrintln("Setting recover rate to " + String(bowControlArray[currentBowSerial].recoverRate), Command);
    } else*/
    if (_commandItem->command == "bowpidmaxerror") {
        if (request) {
            commandResponses->push_back({ "bpme:" + String(bowControlArray[currentBowSerial].pidMaxError), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].pidMaxError = _commandItem->argument[0].toInt();
            commandResponses->push_back({"Setting pid max error to " + String(bowControlArray[currentBowSerial].pidMaxError), Command});
        }
    } else
    if (_commandItem->command == "bowmotortimeout") {
        if (request) {
            commandResponses->push_back({ "bmt:" + String(bowControlArray[currentBowSerial].bowShutoffTimeout), InfoRequest });
        } else {
            bowControlArray[currentBowSerial].bowShutoffTimeout = _commandItem->argument[0].toInt();
            commandResponses->push_back({"Setting motor timeout to " + String( bowControlArray[currentBowSerial].bowShutoffTimeout), Command});
        }
    } else
    if (_commandItem->command == "bowmotorspeedmax") {
        if (request) {
            commandResponses->push_back({ "bmsx:" + String(calibrationDataArray[currentBowSerial].maxHz), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].maxHz = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting max bow frequency to " + String(calibrationDataArray[currentBowSerial].maxHz), Command});
        }
    } else
    if (_commandItem->command == "bowmotorspeedmin") {
        if (request) {
            commandResponses->push_back({ "bmsi:" + String(calibrationDataArray[currentBowSerial].minHz), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].minHz = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting min bow frequency to " + String(calibrationDataArray[currentBowSerial].minHz), Command});
        }
    } else
    if (_commandItem->command == "bowmotormininertialpwm") {
        if (request) {
            commandResponses->push_back({ "bmmip:" + String(calibrationDataArray[currentBowSerial].minInertialPWM), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].minInertialPWM = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting min bow inertial PWM to " + String(calibrationDataArray[currentBowSerial].minInertialPWM), Command});
        }
    } else
    if (_commandItem->command == "bowpressurepositionmax") {
        if (request) {
            commandResponses->push_back({ "bppx:" + String(calibrationDataArray[currentBowSerial].stallPressure), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].stallPressure = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting max bow pressure to " + String(calibrationDataArray[currentBowSerial].stallPressure), Command});
        }
    } else
    if (_commandItem->command == "bowpressurepositionengage") {
        if (request) {
            commandResponses->push_back({ "bppe:" + String(calibrationDataArray[currentBowSerial].firstTouchPressure), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].firstTouchPressure = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting min bow pressure to " + String(calibrationDataArray[currentBowSerial].firstTouchPressure), Command});
        }
    } else
    if (_commandItem->command == "bowpressurepositionrest") {
        if (request) {
            commandResponses->push_back({ "bppr:" + String(calibrationDataArray[currentBowSerial].restPosition), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].restPosition = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            commandResponses->push_back({"Setting bow rest position " + String(calibrationDataArray[currentBowSerial].restPosition), Command});
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
            }/*
            bowControlArray[currentBowSerial].currentHarmonicSeries = hs;
            bowControlArray[currentBowSerial].currentHarmonicSeriesData = bowControlArray[currentBowSerial].harmonicSeriesList.series[hs];*/
            bowControlArray[currentBowSerial].loadHarmonicSeries(hs);
            commandResponses->push_back({"Setting harmonic series to " + String(bowControlArray[currentBowSerial].currentHarmonicSeries), Command});
        }
    } else
    if (_commandItem->command == "bowharmonicserieslist") {
        if (request) {
/*            for (int i = 0; i < int(bowControlArray[currentBowSerial].harmonicSeriesList.series.size()); i++) {
                String response = "bhsl:" + String(i);
                int j=0;
                while (j < int(bowControlArray[currentBowSerial].harmonicSeriesList.series[i].ratio.size())) {
                    response += ":" + String(bowControlArray[currentBowSerial].harmonicSeriesList.series[i].ratio[j], 4);
                    j++;
                }
                commandResponses->push_back({response, InfoRequest});
            }*/
            String response = "bhsl";
            int j=0;
            while (j < int(bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio.size())) {
                response += ":" + String(bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio[j], 4);
                j++;
            }
            commandResponses->push_back({response, InfoRequest});
        } else {
            if (!checkArgumentsMin(_commandItem, commandResponses, 2)) { return false; }
            String response = "Setting harmonic list data:" + String(_commandItem->argument[0].toInt());

//            debugPrintln("wanted " + String(_commandItem->argument[0].toInt()) + " list size is " + String(bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1), debugPrintType::Debug);
            while (_commandItem->argument[0].toInt() > bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1) {
//                debugPrintln("Adding new list", debugPrintType::Debug);
                bowControlArray[currentBowSerial].harmonicSeriesList.addHarmonicSeries();
            }

//            debugPrintln("Clearing ratios", debugPrintType::Debug);
            bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].ratio.clear();
//            debugPrintln("Setting ID", debugPrintType::Debug);
            bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].Id = String(_commandItem->argument[1]);

//            debugPrintln("Setting data", debugPrintType::Debug);
            for (int i = 1; i < (int(_commandItem->argument.size()) - 1); i++) {
                response += ":" + _commandItem->argument[i + 1];
                bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].setHarmonic(i - 1, _commandItem->argument[i + 1].toFloat());
            }
//            debugPrintln("About to update harmonic data", debugPrintType::Debug);

            if (bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio.size() == 0) {
                debugPrintln("Current harmonic series is zero, reloading", debugPrintType::Debug);
                bowControlArray[currentBowSerial].loadHarmonicSeries(bowControlArray[currentBowSerial].currentHarmonicSeries);
            }

            bowControlArray[currentBowSerial].updateHarmonicData();
            commandResponses->push_back({response, Command});
        }
    } else
    if (_commandItem->command == "bowharmonicseriesratio") {
        if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
        /*if (!validateNumber(_commandItem->argument[0].toInt(), 0,
            bowControlArray[currentBowSerial].harmonicSeriesList.series[bowControlArray[currentBowSerial].currentHarmonicSeries].ratio.size())) { return false; }

        bowControlArray[currentBowSerial].harmonicSeriesList.series[bowControlArray[currentBowSerial].currentHarmonicSeries].setHarmonic(
            _commandItem->argument[0].toInt(), _commandItem->argument[1].toFloat());*/

        bowControlArray[currentBowSerial].currentHarmonicSeriesData.setHarmonic(
            _commandItem->argument[0].toInt(), _commandItem->argument[1].toFloat());


        bowControlArray[currentBowSerial].updateHarmonicData();

        commandResponses->push_back({"Set harmonic " + String(_commandItem->argument[0].toInt()) + " to " + String(_commandItem->argument[1].toFloat()), Command});
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

        commandResponses->push_back({"Removing ratio " + String(i), Command});
    } else
    if (_commandItem->command == "bowharmonicseriescount") {
        //commandResponses->push_back({ "bhsc:" + String(bowControlArray[currentBowSerial].harmonicSeriesList.series.size()), InfoRequest });
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
            if (!validateNumber(_commandItem->argument[1].toInt(), 0, bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1, true)) {
                bowControlArray[currentBowSerial].harmonicSeriesList.addHarmonicSeries(_commandItem->argument[0], bowControlArray[currentBowSerial].currentHarmonicSeriesData.ratio);
                bowControlArray[currentBowSerial].currentHarmonicSeries = bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1;
                commandResponses->push_back({ "Saved as new harmonic list " + String(bowControlArray[currentBowSerial].currentHarmonicSeries), Command });
            } else {
                bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[1].toInt()] = bowControlArray[currentBowSerial].currentHarmonicSeriesData;
                bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[1].toInt()].Id = _commandItem->argument[0];
                bowControlArray[currentBowSerial].currentHarmonicSeries = _commandItem->argument[1].toInt();
                commandResponses->push_back({ "Saved over harmonic list " + String(bowControlArray[currentBowSerial].currentHarmonicSeries), Command });
            }
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

            commandResponses->push_back({ "Removed list " + String(bowControlArray[currentBowSerial].currentHarmonicSeries), Command });
        }
    } else
    if (_commandItem->command == "bowmotorvoltage") {
        if (request) {
            commandResponses->push_back({ "bmv:" + String(bowIOArray[currentBowSerial].getBowMotorVoltage()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].setBowMotorVoltage(_commandItem->argument[0].toFloat());
            commandResponses->push_back({"Setting bow voltage to " + String(bowIOArray[currentBowSerial].getBowMotorVoltage()), Command});
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
            commandResponses->push_back({"Setting base note to " + String(bowControlArray[currentBowSerial].baseNote), Command});
        }
    } else
    if (_commandItem->command == "bowmotorcurrentlimit") {
        if (request) {
            commandResponses->push_back({ "bmcl:" + String(bowIOArray[currentBowSerial].bowMotorCurrentLimit), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].bowMotorCurrentLimit = _commandItem->argument[0].toFloat();
            commandResponses->push_back({"Setting bow current limit to " + String(bowIOArray[currentBowSerial].bowMotorCurrentLimit), Command});
        }
    } else
/*
    { "bowmotorpowerlimit", "bmpl", "0-255", "Bow motor power limit (W) - !WARNING! Can ruin your instrument if changed" },
    { "bowmotorfaultcommands", "bmfc", "command list", "Commands to execute when a motor fault is tripped - !WARNING! Can ruin your instrument if changed" },
    { "bowmotoroverpowercommands", "bmopc", "command list", "Commands to execute when motor is over the power limit - !WARNING! Can ruin your instrument if changed" },
*/
    if (_commandItem->command == "bowmotorpowerlimit") {
        if (request) {
            commandResponses->push_back({ "bmpl:" + String(bowIOArray[currentBowSerial].bowMotorWattage), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].bowMotorWattage = _commandItem->argument[0].toFloat();
            commandResponses->push_back({"Setting bow power limit to " + String(bowIOArray[currentBowSerial].bowMotorCurrentLimit), Command});
        }
    } else
    if (_commandItem->command == "bowmotorfaultcommands") {
        if (request) {
            commandResponses->push_back({ "bmfc:" + String(bowControlArray[currentBowSerial].commandsMotorFault), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].commandsMotorFault = _commandItem->argument[0];
            commandResponses->push_back({"Setting bow fault commands to " + String(bowControlArray[currentBowSerial].commandsMotorFault), Command});
        }
    } else
    if (_commandItem->command == "bowmotoroverpowercommands") {
        if (request) {
            commandResponses->push_back({ "bmopc:" + String(bowControlArray[currentBowSerial].commandsOverPowerCurrent), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].commandsOverPowerCurrent = _commandItem->argument[0];
            commandResponses->push_back({"Setting bow over power commands to " + String(bowControlArray[currentBowSerial].commandsOverPowerCurrent), Command});
        }
    } else

    if (_commandItem->command == "bowcontrolharmonicshiftrange") {
        if (request) {
            commandResponses->push_back({ "bchsr:" + String(bowControlArray[currentBowSerial].getHarmonicShiftRange()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShiftRange(_commandItem->argument[0].toFloat());
            commandResponses->push_back({"Setting harmonic shift range to " + String(bowControlArray[currentBowSerial].getHarmonicShiftRange()), Command});
        }
    } else
    if (_commandItem->command == "solenoidmaxforce") {
        if (request) {
            commandResponses->push_back({ "sxf:" + String(solenoidArray[currentSolenoidSerial].forceMax), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            solenoidArray[currentSolenoidSerial].forceMax = _commandItem->argument[0].toInt();
            commandResponses->push_back({"Setting solenoid max force to " + String(solenoidArray[currentSolenoidSerial].forceMax), Command});
        }
    } else
    if (_commandItem->command == "solenoidminforce") {
        if (request) {
            commandResponses->push_back({ "sif:" + String(solenoidArray[currentSolenoidSerial].forceMin), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            solenoidArray[currentSolenoidSerial].forceMin = _commandItem->argument[0].toInt();
            commandResponses->push_back({"Setting solenoid min force to " + String(solenoidArray[currentSolenoidSerial].forceMin), Command});
        }
    } else
    if (_commandItem->command == "solenoidforcemultiplier") {
        if (request) {
            commandResponses->push_back({ "sfm:" + String(solenoidArray[currentSolenoidSerial].forceMultiplier), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            solenoidArray[currentSolenoidSerial].forceMultiplier = _commandItem->argument[0].toFloat();
            commandResponses->push_back({"Setting solenoid force multiplier to " + String(solenoidArray[currentSolenoidSerial].forceMultiplier), Command});
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
            commandResponses->push_back({"Setting bow modulation speed to " + String(bowControlArray[currentBowSerial].bowSpeedWhileEngaged), Command});
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
            commandResponses->push_back({"Setting bow engage speed to " + String(bowControlArray[currentBowSerial].bowSpeedToEngage), Command});
        }
    } else
    if (_commandItem->command == "bowhome") {
        bowControlArray[currentBowSerial].bowIOConnect->homeBow();
    } else {
        return false;
    }
    return true;
}

bool stringModule::processSerialCommand_MuteControl(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
    if (currentBowSerial >= int(muteArray.size())) {
        return false;
    }
    if (_commandItem->command == "mutesetposition") {
        if (request) {
            commandResponses->push_back({ "msp:" + String(muteArray[currentBowSerial].getTilt()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            muteArray[currentBowSerial].setTilt(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting mute to " + String(muteArray[currentBowSerial].getTilt()), Command});
        }
    } else
    if (_commandItem->command == "mutefullmute") {
        if (request) {
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (_commandItem->argument[0].toInt() == 1) {
                muteArray[currentBowSerial].fullMute();
                commandResponses->push_back({"Engaging full mute", Command});
            }
        }
    } else
    if (_commandItem->command == "mutehalfmute") {
        if (request) {
        } else {
            muteArray[currentBowSerial].halfMute();
            commandResponses->push_back({"Engaging half mute", Command});
        }
    } else
    if (_commandItem->command == "muterest") {
        if (request) {
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (_commandItem->argument[0].toInt() == 1) {
                muteArray[currentBowSerial].rest();
                commandResponses->push_back({"Engaging mute rest", Command});
            }
        }
    } else
    if (_commandItem->command == "mutesavefull") {
        muteArray[currentBowSerial].saveFullMute();
        commandResponses->push_back({"Saving full mute position", Command});
    } else
    if (_commandItem->command == "mutesavehalf") {
        muteArray[currentBowSerial].saveHalfMute();
        commandResponses->push_back({"Saving half mute position", Command});
    } else
    if (_commandItem->command == "mutesaverest") {
        muteArray[currentBowSerial].saveRest();
        commandResponses->push_back({"Saving rest position", Command});
    } else
    if (_commandItem->command == "mutefullmuteposition") {
        if (request) {
            commandResponses->push_back({ "mfmp:" + String(muteArray[currentBowSerial].getFullMutePosition()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            muteArray[currentBowSerial].setFullMutePosition(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting full mute position to " + String(muteArray[currentBowSerial].getFullMutePosition()), Command});
        }
    } else
    if (_commandItem->command == "mutehalfmuteposition") {
        if (request) {
            commandResponses->push_back({ "mhmp:" + String(muteArray[currentBowSerial].getHalfMutePosition()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            muteArray[currentBowSerial].setHalfMutePosition(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting half mute position to " + String(muteArray[currentBowSerial].getHalfMutePosition()), Command});
        }
    } else
    if (_commandItem->command == "muterestposition") {
        if (request) {
            commandResponses->push_back({ "mrp:" + String(muteArray[currentBowSerial].getRestPosition()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            muteArray[currentBowSerial].setRestPosition(_commandItem->argument[0].toInt());
            commandResponses->push_back({"Setting rest position to " + String(muteArray[currentBowSerial].getRestPosition()), Command});
        }
    } else
    if (_commandItem->command == "mutesustain") {
        if (request) {
            commandResponses->push_back({ "ms:" + String(muteArray[currentBowSerial].getSustain()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (_commandItem->argument[0].toInt() == 1) {
                muteArray[currentBowSerial].setSustain(true);
            } else {
                muteArray[currentBowSerial].setSustain(false);
            }
            commandResponses->push_back({"Setting sustain to " + String(muteArray[currentBowSerial].getSustain()), Command});
        }
    } else
    if (_commandItem->command == "mutebackoff") {
        if (request) {
            commandResponses->push_back({ "mbo:" + String(muteArray[currentBowSerial].backOffTime), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }

            muteArray[currentBowSerial].backOffTime = _commandItem->argument[0].toInt();
            commandResponses->push_back({"Mute backoff set to " + String(muteArray[currentBowSerial].backOffTime), Command});
        }
    } else
    if (_commandItem->command == "mutehome") {
        muteArray[currentBowSerial].homeMute();
    } else
    if (_commandItem->command == "mutecalibrate") {
        if (!calibrateMuteArray[currentBowSerial].calibrateAll()) {
            return false;
        }
        commandResponses->push_back({"Mute calibration finished", Command});
    } else {
        return false;
    }
    return true;
}

bool stringModule::processSerialCommand_StatusTesting(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
    if (_commandItem->command == "bowstatus") {
        commandResponses->push_back({"Set frequency " + String(bowControlArray[currentBowSerial].getPIDTarget()) + " Hz, ", TextInfo});
        commandResponses->push_back({"Read frequency " + String(bowIOArray[currentBowSerial].averageFreq()) + " Hz", TextInfo});
        commandResponses->push_back({" P gain " + String(bowControlArray[currentBowSerial].Kp), TextInfo});
        commandResponses->push_back({" I gain " + String(bowControlArray[currentBowSerial].Ki), TextInfo});
        commandResponses->push_back({" D gain " + String(bowControlArray[currentBowSerial].Kd), TextInfo});
        commandResponses->push_back({" ignore below " + String(bowControlArray[currentBowSerial].integratorIgnoreBelow), TextInfo});
        commandResponses->push_back({" max error " + String(bowControlArray[currentBowSerial].pidMaxError), TextInfo});
    } else
    if (_commandItem->command == "bowdebugmeasuretimetotarget") {
        bowControlArray[currentBowSerial].measureTimeToTarget(_commandItem->argument[0].toInt());
    }  else

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
    if (_commandItem->command == "bowmotorfrequency") {
        if (request) {
            commandResponses->push_back({ "bmf:" + String(bowIOArray[currentBowSerial].averageFreq()), InfoRequest });
        }
    }  else
    if (_commandItem->command == "bowpidpeakerror") {
        if (request) {
            commandResponses->push_back({ "bpperr:" + String(bowControlArray[currentBowSerial].getPIDPeakError()), InfoRequest });
        }
    }  else
    if (_commandItem->command == "nooperation") {
        commandResponses->push_back({ "Precisely did nothing at all", Command });
    } else {
        return false;
    }
    return true;
}

/*    { "bowactuatorset", "bas", "0-127", "Set current bow actuator"},
    { "bowactuatorload", "bal", "-", "Load parameters from current bow actuator"},
    { "bowactuatorsave", "bav", "-", "Save current bow parameters into currently selected bow actuator" }
*/
bool stringModule::processSerialCommand_BowActuator(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
    commandList *delegatedCommands = nullptr) {

    if (_commandItem->command == "bowactuatorset") {
        if (request) {
            commandResponses->push_back({ "bas:" + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (bowControlArray[currentBowSerial].bowActuators->setBowActuator(_commandItem->argument[0].toInt()) != _commandItem->argument[0].toInt()) {
                return false;
            }
            commandResponses->push_back({"Setting bow actuator to " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), Command});
        }
    }  else
    if (_commandItem->command == "bowactuatorload") {
        bowControlArray[currentBowSerial].bowActuators->loadBowActuator();
        commandResponses->push_back({"Loaded data for actuator " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), Command});
    }  else
    if (_commandItem->command == "bowactuatorsave") {
        bowControlArray[currentBowSerial].bowActuators->saveBowActuator();
        commandResponses->push_back({"Saving data for actuator " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), Command});
    }  else
    if (_commandItem->command == "bowactuatordata") {
        if (request) {
            int8_t bowIndex = bowControlArray[currentBowSerial].bowActuators->getBowActuator();
            if (checkArguments(_commandItem, commandResponses, 1, true)) {
                bowIndex = _commandItem->argument[0].toInt();
                if (bowIndex > (bowControlArray[currentBowSerial].bowActuators->getBowActuatorCount() - 1)) {
                    return false;
                }
            }
            commandResponses->push_back({ "bad:" + String(bowIndex) + ":" +
                String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorFirstTouchPressure(bowIndex)) + ":"  +
                String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorStallPressure(bowIndex)) + ":" +
                String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorRestPosition(bowIndex)) + ":" +
                bowControlArray[currentBowSerial].bowActuators->getBowActuatorID(bowIndex), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 5)) { return false; }
            bowControlArray[currentBowSerial].bowActuators->setBowActuatorData(_commandItem->argument[0].toInt(), _commandItem->argument[1].toInt(),
                _commandItem->argument[2].toInt(), _commandItem->argument[3].toInt(), _commandItem->argument[4]);
            commandResponses->push_back({"Setting bow actuator data for bow " + _commandItem->argument[0] + "  ID: " + _commandItem->argument[4] + ", first touch: " +
                _commandItem->argument[1] + ", stall pressure: " + _commandItem->argument[2] + ", rest position: " + _commandItem->argument[3] , Command});
        }
    } else
    if (_commandItem->command == "bowactuatorid") {
        if (request) {
            commandResponses->push_back({ "bai:" + String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorID()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].bowActuators->setBowActuatorID(_commandItem->argument[0]);
            commandResponses->push_back({"Setting bow actuator ID to " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorID()), Command});
        }
    }  else
    if (_commandItem->command == "bowactuatorcount") {
        commandResponses->push_back({ "bac:" + String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorCount()), InfoRequest });
    } else
    if (_commandItem->command == "bowactuatoradd") {
        if (request) {
            return false;
        } else {
            uint8_t bowIndex = bowControlArray[currentBowSerial].bowActuators->addBowActuator();
            commandResponses->push_back({"Adding bow actuator " + String(bowIndex), Command});
        }
    } else
    if (_commandItem->command == "bowactuatorremove") {
        if (request) {
            return false;
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!bowControlArray[currentBowSerial].bowActuators->removeBowActuator(_commandItem->argument[0].toInt())) { return false; }
            commandResponses->push_back({"Removing bow actuator " + String(_commandItem->argument[0].toInt()), Command});
        }
    } else {
        return false;
    }
    return true;
}


bool stringModule::processRequestCommand(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool delegated) {
    if (!checkArgumentsMin(_commandItem, commandResponses, 1)) { return false; }
    String command = _commandItem->argument[0];
    for (int i = 0; i < int(sizeof(serialCommandsStringModule) / sizeof(serialCommandItem)); i++) {
        if (serialCommandsStringModule[i].shortCommand == command) {
            command = serialCommandsStringModule[i].longCommand;
            break;
        }
    }
    commandItem requestItem(command);
    for (uint8_t i = 1; i<_commandItem->argument.size();i++) {
        requestItem.argument.push_back(_commandItem->argument[i]);
    }

    if (processSerialCommand_GeneralControl(&requestItem, commandResponses, true, delegated)) {
    } else
    if (processSerialCommand_CalibrationsSettings(&requestItem, commandResponses, true, delegated)) {
    } else
    if (processSerialCommand_MuteControl(&requestItem, commandResponses, true, delegated)) {
    } else
    if (processSerialCommand_StatusTesting(&requestItem, commandResponses, true, delegated)) {
    } else
    if (processSerialCommand_BowActuator(&requestItem, commandResponses, true, delegated)) {
    } else
    {
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

#if EFARMASTER
    String slaveString = _commandItem->command;
    for (int i=0; i<_commandItem->argument.size(); i++) {
        slaveString += ":" + _commandItem->argument[i];
    }
    debugPrintln("Writing to slave " + slaveString, Debug);
    writeToSlave(slaveString);
#endif
    processCommandItems(_commandItem, serialCommandsStringModule, sizeof(serialCommandsStringModule) / sizeof(serialCommandItem));
//        debugPrintln("String module checking for " + _commandItem->command, Debug);

    if (processSerialCommand_GeneralControl(_commandItem, commandResponses, false, delegated)) {
    } else
    if (processSerialCommand_CalibrationsSettings(_commandItem, commandResponses, false, delegated)) {
    } else
    if (processSerialCommand_MuteControl(_commandItem, commandResponses, false, delegated)) {
    } else
    if (processSerialCommand_StatusTesting(_commandItem, commandResponses, false, delegated)) {
    } else
    if (processSerialCommand_BowActuator(_commandItem, commandResponses, false, delegated)) {
    } else
    if (_commandItem->command == "help") {
        //String help = "These commands are specific for the currently selected string module\n";
        //help += "---\n";
        //help += printCommandHelp(serialCommandsStringModule, sizeof(serialCommandsStringModule) / sizeof(serialCommandItem));
        commandResponses->push_back({":\"These commands are specific for the currently selected string module\"", debugPrintType::Help});
        addCommandHelp(serialCommandsStringModule, sizeof(serialCommandsStringModule) / sizeof(serialCommandItem), commandResponses, "[str]:");
    } else
    if (_commandItem->command == "requestinfo") {
        if (!processRequestCommand(_commandItem, commandResponses, delegated)) { return false; }
    } else {
        return false;
    }
//        debugPrintln("Finished module command successfully", Debug);
    return true;
};

stringModule::stringModule() {
    bowIOArray.reserve(1);
    bowControlArray.reserve(1);
    calibrationDataArray.reserve(1);
};

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
            muteArray[i].updateMute();
        }
    }
}


#endif
