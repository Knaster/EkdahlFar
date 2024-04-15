#ifndef STRINGMODULE_C
#define STRINGMODULE_C

#include "avr_functions.h"
#include <vector>
#include "bowio.h"

#include "stringmodule.hpp"

bool stringModule::addBow(char motorRevPin, char motorVoltagePin, char motorDCDCEn, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort,
    char stepHomeSensorPin, char stepCorrectionSensorPin) {

    _calibrationData* calibrationData = new _calibrationData();
    calibrationDataArray.push_back(*calibrationData);

    bowIO* _bowIO = new bowIO(motorRevPin, motorVoltagePin, motorDCDCEn, tachoPin, currentSensePin, motorFaultPin, stepEnPin, stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin, stepCorrectionSensorPin);
    _bowIO->stepServoStepper->stepperID = 1;
    bowIOArray.push_back(*_bowIO);

    int arrayIndex = bowIOArray.size() - 1;
    ///\todo fix pointers here, major irritation

    bowControl* _bowControl = new bowControl(bowIOArray[arrayIndex], calibrationDataArray[arrayIndex]);
    bowControlArray.push_back(*_bowControl);

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
    if (_commandItem->command == "frequency") {
        if (request) {
            commandResponses->push_back({ "f:" + String(bowControlArray[currentBowSerial].getPIDTarget()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setPIDTarget(_commandItem->argument[0].toFloat()); //String(serialCommand.substring(1,serialCommand.length())).toFloat());
            commandResponses->push_back({"Setting frequency " + String(bowControlArray[currentBowSerial].getPIDTarget()) + "Hz", Command});
        }
    }else
    if (_commandItem->command == "run") {
        if (request) {
            commandResponses->push_back({ "run:" + String(bowControlArray[currentBowSerial].run), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (_commandItem->argument[0].toInt() > 0) {
                bowControlArray[currentBowSerial].run = 1;
                bowIOArray[currentBowSerial].enableBowPower();
            } else {
                bowControlArray[currentBowSerial].run = 0;
                bowIOArray[currentBowSerial].disableBowPower();
            }
            debugPrintln("Setting run to " + String(bowControlArray[currentBowSerial].run), Command);
        }
    } else
    if (_commandItem->command == "pid") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0].toInt() > 0) { bowControlArray[currentBowSerial].PIDon = true; } else { bowControlArray[currentBowSerial].PIDon = false;}
        debugPrintln("Setting PID On to " + String(bowControlArray[currentBowSerial].PIDon), Command);
    } else
    if (_commandItem->command == "harmonic") {
        if (request) {
            commandResponses->push_back({ "h:" + String(bowControlArray[currentBowSerial].getHarmonic()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!bowControlArray[currentBowSerial].setHarmonic(_commandItem->argument[0].toInt())) {
                debugPrintln("ERROR setting harmonic to " + String(_commandItem->argument[0].toInt()), Error);
            } else {
                debugPrintln("Setting harmonic to " + String(_commandItem->argument[0].toInt()), Command);
            }
        }
    } else
    if (_commandItem->command == "harmonicbase") {
        if (request) {
//                commandResponses->push_back({ "h:" + String(bowControlArray[currentBowSerial].getHarmonic()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonic(_commandItem->argument[0].toInt() - bowControlArray[currentBowSerial].baseNote);
            debugPrintln("Setting harmonic to " + String(_commandItem->argument[0].toInt()  - bowControlArray[currentBowSerial].baseNote), Command);
        }
    } else
    if (_commandItem->command == "solenoid") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        currentSolenoidSerial = _commandItem->argument[0].toInt();
        debugPrintln("Setting current solenoid to " + String(currentSolenoidSerial), Command);
    } else
    if (_commandItem->command == "solenoidengage") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        solenoidArray[currentSolenoidSerial].solenoidEngage(_commandItem->argument[0].toInt());
        debugPrintln("Setting solenoid engagement to " + String(_commandItem->argument[0].toInt()), Command);
    } else
    if (_commandItem->command == "solenoiddisengage") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        solenoidArray[currentSolenoidSerial].solenoidDisengage(_commandItem->argument[0].toInt());
        debugPrintln("Setting solenoid disengagement to " + String(_commandItem->argument[0].toInt()), Command);
    } else
    if (_commandItem->command == "setmanualpressure") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        bowControlArray[currentBowSerial].setManualTilt(_commandItem->argument[0].toInt());
        debugPrintln("Setting pressure to " + String(_commandItem->argument[0].toInt()), Command);
    } else
    if (_commandItem->command == "setpressurebaseline") {
        if (request) {
            commandResponses->push_back({ "spb:" + String(bowControlArray[currentBowSerial].getPressureBaseline()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setPressureBaseline(_commandItem->argument[0].toInt());
            debugPrintln("Setting pressure baseline to " + String(_commandItem->argument[0].toInt()), Command);
        }
    } else
    if (_commandItem->command == "setpressuremodifier") {
        if (request) {
            commandResponses->push_back({ "spm:" + String(bowControlArray[currentBowSerial].getPressureModifier()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setPressureModifier(_commandItem->argument[0].toInt());
            debugPrintln("Setting pressure modifier to " + String(_commandItem->argument[0].toInt()), Command);
        }
    } else
    if (_commandItem->command == "rest") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        bowControlArray[currentBowSerial].bowRest(_commandItem->argument[0].toInt());
        debugPrintln("Setting bow rest position to " + String(_commandItem->argument[0].toInt()), Command);
    } else
    if (_commandItem->command == "engage") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        bowControlArray[currentBowSerial].bowEngage(_commandItem->argument[0].toInt());
        debugPrintln("Setting bow engage to " + String(_commandItem->argument[0].toInt()), Command);
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
    if (_commandItem->command == "setbowpower") {
        if (request) {
            commandResponses->push_back({ "sbp:" + String(bowControlArray[currentBowSerial].getBowPower()), InfoRequest });
        } else {
//                bowControlArray[currentBowSerial].manualSpeedPWM =_commandItem->argument[0].toInt();
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setBowPower(_commandItem->argument[0].toInt());
            debugPrintln("Setting bow power to " + String(bowControlArray[currentBowSerial].manualSpeedPWM), Command);
        }
    } else
    if (_commandItem->command == "setbowhold") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if (_commandItem->argument[0].toInt() > 0) { bowControlArray[currentBowSerial].setHold(true); } else { bowControlArray[currentBowSerial].setHold(false);}
        debugPrintln("Setting bow hold to " + String(_commandItem->argument[0].toInt()), Command);
    } else
    if (_commandItem->command == "setspeedmode") {
        if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
        if ((_commandItem->argument[0].toInt() >= 0) && (_commandItem->argument[0].toInt() <= 1)) {
            bowControlArray[currentBowSerial].speedMode = (_speedMode) (_commandItem->argument[0].toInt());
        }
        debugPrintln("Setting speed mode to " + String(int(bowControlArray[currentBowSerial].speedMode)), Command);
    } else
    if (_commandItem->command == "setharmonicshift") {
        if (request) {
            commandResponses->push_back({ "shs:" + String(bowControlArray[currentBowSerial].getHarmonicShift()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShift(_commandItem->argument[0].toInt());
            debugPrintln("Setting harmonic shift to " + String(bowControlArray[currentBowSerial].getHarmonicShift()), Command);
        }
    } else
    if (_commandItem->command == "setharmonicshift5") {
        if (request) {
            commandResponses->push_back({ "shs5:" + String(bowControlArray[currentBowSerial].getHarmonicShift5()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShift5(_commandItem->argument[0].toInt());
            debugPrintln("Setting harmonic shift 5 to " + String(bowControlArray[currentBowSerial].getHarmonicShift5()), Command);
        }
    } else {
        return false;
    }
    return true;
}

bool stringModule::processSerialCommand_CalibrationsSettings(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
    if (_commandItem->command == "pidki") {
        if (request) {
            commandResponses->push_back({ "ki:" + String(bowControlArray[currentBowSerial].Ki), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].Ki = _commandItem->argument[0].toFloat();
            debugPrintln("Setting PID Ki value to " + String(bowControlArray[currentBowSerial].Ki), Command);
        }
    } else
    if (_commandItem->command == "pidkp") {
        if (request) {
            commandResponses->push_back({ "kp:" + String(bowControlArray[currentBowSerial].Kp), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].Kp = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            debugPrintln("Setting PID Kp value to " + String(bowControlArray[currentBowSerial].Kp), Command);
        }
    } else
    if (_commandItem->command == "pidkd") {
        if (request) {
            commandResponses->push_back({ "kd:" + String(bowControlArray[currentBowSerial].Kd), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].Kd = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            debugPrintln("Setting PID Kd value to " + String(bowControlArray[currentBowSerial].Kd), Command);
        }
    } else
    if (_commandItem->command == "pidr") {
        debugPrintln("Resetting PID", Command);
        bowControlArray[currentBowSerial].previousError = 0;
        bowControlArray[currentBowSerial].pidReset();
    }  else
    if (_commandItem->command == "integratorerror") {
        if (request) {
            commandResponses->push_back({ "ie:" + String(bowControlArray[currentBowSerial].integratorIgnoreBelow), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].integratorIgnoreBelow = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(2,serialCommand.length())).toFloat();
            debugPrintln("Setting error ignore to " + String(bowControlArray[currentBowSerial].integratorIgnoreBelow), Command);
        }
    } else
    if (_commandItem->command == "fundamental") {
        if (request) {
            commandResponses->push_back({ "u:" + String(calibrationDataArray[currentBowSerial].fundamentalFrequency), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].fundamentalFrequency = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            debugPrintln("Setting fundamental frequency to " + String(calibrationDataArray[currentBowSerial].fundamentalFrequency), Command);
        }
    } else
    if (_commandItem->command == "calibrateall") {
        calibrateArray[currentBowSerial].calibrateAll();
    } else
    if (_commandItem->command == "calibratepressure") {
        debugPrintln("Finding min/max pressure", Command);
        if (!calibrateArray[currentBowSerial].findMinMaxPressure()) {
            debugPrintln("Find min/max pressure FAILED", Error);
        }
    } else
    if (_commandItem->command == "calibratespeed") {
        debugPrintln("Finding min/max speed", Command);
        if (!calibrateArray[currentBowSerial].findMinMaxSpeedPWM()) {
            debugPrintln("Find min/max speed FAILED", Error);
        }
    } else
    if (_commandItem->command == "calibratespeedpid") {
        debugPrintln("Finding min/max speed with the PID", Command);
        if (!calibrateArray[currentBowSerial].findMinMaxSpeedPID()) {
            debugPrintln("Find min/max speed with the PID FAILED", Error);
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
    if (_commandItem->command == "stablethreshold") {
        bowControlArray[currentBowSerial].elapsedTimeThreshold = _commandItem->argument[0].toFloat(); // String(serialCommand.substring(2,serialCommand.length())).toFloat();
        debugPrintln("Setting elapsed stable threshold to " + String(bowControlArray[currentBowSerial].elapsedTimeThreshold) + "ms", Command);
    } else
    if (_commandItem->command == "recoverrate") {
        bowControlArray[currentBowSerial].recoverRate = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
        debugPrintln("Setting recover rate to " + String(bowControlArray[currentBowSerial].recoverRate), Command);
    } else
    if (_commandItem->command == "pidmaxerror") {
        bowControlArray[currentBowSerial].pidMaxError = _commandItem->argument[0].toInt();
        debugPrintln("Setting pid max error to " + String(bowControlArray[currentBowSerial].pidMaxError), Command);
    } else
    if (_commandItem->command == "setbowtimeout") {
        if (request) {
            commandResponses->push_back({ "sbt:" + String(bowControlArray[currentBowSerial].bowShutoffTimeout), InfoRequest });
        } else {
            bowControlArray[currentBowSerial].bowShutoffTimeout = _commandItem->argument[0].toInt();
            debugPrintln("Setting motor timeout to " + String( bowControlArray[currentBowSerial].bowShutoffTimeout), Command);
        }
    } else
    if (_commandItem->command == "bowmaxspeed") {
        if (request) {
            commandResponses->push_back({ "bxs:" + String(calibrationDataArray[currentBowSerial].maxHz), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].maxHz = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            debugPrintln("Setting max bow frequency to " + String(calibrationDataArray[currentBowSerial].maxHz), Command);
        }
    } else
    if (_commandItem->command == "bowminspeed") {
        if (request) {
            commandResponses->push_back({ "bis:" + String(calibrationDataArray[currentBowSerial].minHz), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].minHz = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            debugPrintln("Setting min bow frequency to " + String(calibrationDataArray[currentBowSerial].minHz), Command);
        }
    } else
    if (_commandItem->command == "bowmininertialpwm") {
        if (request) {
            commandResponses->push_back({ "biiw:" + String(calibrationDataArray[currentBowSerial].minInertialPWM), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].minInertialPWM = _commandItem->argument[0].toFloat(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            debugPrintln("Setting min bow inertial PWM to " + String(calibrationDataArray[currentBowSerial].minInertialPWM), Command);
        }
    } else
    if (_commandItem->command == "bowmaxpressure") {
        if (request) {
            commandResponses->push_back({ "bxp:" + String(calibrationDataArray[currentBowSerial].stallPressure), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].stallPressure = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            debugPrintln("Setting max bow pressure to " + String(calibrationDataArray[currentBowSerial].stallPressure), Command);
        }
    } else
    if (_commandItem->command == "bowminpressure") {
        if (request) {
            commandResponses->push_back({ "bip:" + String(calibrationDataArray[currentBowSerial].firstTouchPressure), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].firstTouchPressure = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            debugPrintln("Setting min bow pressure to " + String(calibrationDataArray[currentBowSerial].firstTouchPressure), Command);
        }
    } else
    if (_commandItem->command == "bowrestposition") {
        if (request) {
            commandResponses->push_back({ "brp:" + String(calibrationDataArray[currentBowSerial].restPosition), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            calibrationDataArray[currentBowSerial].restPosition = _commandItem->argument[0].toInt(); //String(serialCommand.substring(1,serialCommand.length())).toFloat();
            debugPrintln("Setting bow rest position " + String(calibrationDataArray[currentBowSerial].restPosition), Command);
        }
    } else
    if (_commandItem->command == "harmonicseries") {
        if (request) {
            commandResponses->push_back({ "hs:" + String(bowControlArray[currentBowSerial].currentHarmonicSeries), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            if (!validateNumber(_commandItem->argument[0].toInt(), 0, bowControlArray[currentBowSerial].harmonicSeriesList.series.size())) { return false; }
            uint8_t hs = _commandItem->argument[0].toInt();
            if (hs > (bowControlArray[currentBowSerial].harmonicSeriesList.series.size() - 1)) {
                commandResponses->push_back({"Harmonic list doesn't exist " + String(hs), Error});
                return true;
            }
            bowControlArray[currentBowSerial].currentHarmonicSeries = hs;
            commandResponses->push_back({"Setting harmonic series to " + String(bowControlArray[currentBowSerial].currentHarmonicSeries), Command});
        }
    } else
    if (_commandItem->command == "harmoniclist") {
        if (request) {
            for (int i = 0; i < int(bowControlArray[currentBowSerial].harmonicSeriesList.series.size()); i++) {
                String response = "hl:" + String(i);
                int j=0;
                while (j < int(bowControlArray[currentBowSerial].harmonicSeriesList.series[i].frequency.size())) {
                    response += ":" + String(bowControlArray[currentBowSerial].harmonicSeriesList.series[i].frequency[j], 4);
                    j++;
                }
                commandResponses->push_back({response, InfoRequest});
            }
        } else {
            if (!checkArgumentsMin(_commandItem, commandResponses, 2)) { return false; }
            String response = "Setting harmonic list data:" + String(_commandItem->argument[0].toInt());

            bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].frequency.clear();
            for (int i = 1; i < int(_commandItem->argument.size()); i++) {
                response += ":" + _commandItem->argument[i];
                bowControlArray[currentBowSerial].harmonicSeriesList.series[_commandItem->argument[0].toInt()].setHarmonic(i - 1,
                    _commandItem->argument[i].toFloat());
            }
            bowControlArray[currentBowSerial].updateHarmonicData();
            commandResponses->push_back({response, Command});
        }
    } else
    if (_commandItem->command == "setharmonic") {
        if (!checkArguments(_commandItem, commandResponses, 2)) { return false; }
        if (!validateNumber(_commandItem->argument[0].toInt(), 0,
            bowControlArray[currentBowSerial].harmonicSeriesList.series[bowControlArray[currentBowSerial].currentHarmonicSeries].frequency.size())) { return false; }

        bowControlArray[currentBowSerial].harmonicSeriesList.series[bowControlArray[currentBowSerial].currentHarmonicSeries].setHarmonic(
            _commandItem->argument[0].toInt(), _commandItem->argument[1].toFloat());

        bowControlArray[currentBowSerial].updateHarmonicData();

        commandResponses->push_back({"Set harmonic " + String(_commandItem->argument[0].toInt()) + " to " + String(_commandItem->argument[1].toFloat()) +
            " Hertz" , Command});
    } else
    if (_commandItem->command == "harmoniccount") {
        if (request) {
            commandResponses->push_back({ "hc:" + String(bowControlArray[currentBowSerial].harmonicSeriesList.series.size()), InfoRequest });
        } else {
        }
    } else
    if (_commandItem->command == "bowvoltage") {
        if (request) {
            commandResponses->push_back({ "bv:" + String(bowIOArray[currentBowSerial].getBowMotorVoltage()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].setBowMotorVoltage(_commandItem->argument[0].toFloat());
            debugPrintln("Setting bow voltage to " + String(bowIOArray[currentBowSerial].getBowMotorVoltage()), Command);
        }
    } else
    if (_commandItem->command == "bowcurrent") {
        if (request) {
            commandResponses->push_back({ "bc:" + String(bowIOArray[currentBowSerial].getBowCurrent()), InfoRequest });
        } else {
        }
    } else
    if (_commandItem->command == "setbasenote") {
        if (request) {
            commandResponses->push_back({ "sbn:" + String(bowControlArray[currentBowSerial].baseNote), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setBaseNote(_commandItem->argument[0].toInt());
            debugPrintln("Setting base note to " + String(bowControlArray[currentBowSerial].baseNote), Command);
        }
    } else
    if (_commandItem->command == "bowcurrentlimit") {
        if (request) {
            commandResponses->push_back({ "bcl:" + String(bowIOArray[currentBowSerial].bowMotorCurrentLimit), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowIOArray[currentBowSerial].bowMotorCurrentLimit = _commandItem->argument[0].toFloat();
            debugPrintln("Setting bow current limit to " + String(bowIOArray[currentBowSerial].bowMotorCurrentLimit), Command);
        }
    } else
    if (_commandItem->command == "harmonicshiftrange") {
        if (request) {
            commandResponses->push_back({ "hsr:" + String(bowControlArray[currentBowSerial].getHarmonicShiftRange()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].setHarmonicShiftRange(_commandItem->argument[0].toFloat());
            debugPrintln("Setting harmonic shift range to " + String(bowControlArray[currentBowSerial].getHarmonicShiftRange()), Command);
        }
    } else
    if (_commandItem->command == "setsolenoidmaxforce") {
        if (request) {
            commandResponses->push_back({ "ssxf:" + String(solenoidArray[currentSolenoidSerial].forceMax), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            solenoidArray[currentSolenoidSerial].forceMax = _commandItem->argument[0].toInt();
            debugPrintln("Setting solenoid max force to " + String(solenoidArray[currentSolenoidSerial].forceMax), Command);
        }
    } else
    if (_commandItem->command == "setsolenoidminforce") {
        if (request) {
            commandResponses->push_back({ "ssif:" + String(solenoidArray[currentSolenoidSerial].forceMin), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            solenoidArray[currentSolenoidSerial].forceMin = _commandItem->argument[0].toInt();
            debugPrintln("Setting solenoid min force to " + String(solenoidArray[currentSolenoidSerial].forceMin), Command);
        }
    } else
    if (_commandItem->command == "setsolenoidforcemultiplier") {
        if (request) {
            commandResponses->push_back({ "ssfm:" + String(solenoidArray[currentSolenoidSerial].forceMultiplier), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            solenoidArray[currentSolenoidSerial].forceMultiplier = _commandItem->argument[0].toFloat();
            debugPrintln("Setting solenoid force multiplier to " + String(solenoidArray[currentSolenoidSerial].forceMultiplier), Command);
        }
    } else
    if (_commandItem->command == "bowmodulationspeed") {
        if (request) {
            commandResponses->push_back({ "bms:" + String(bowControlArray[currentBowSerial].bowSpeedWhileEngaged), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            uint16_t bes = _commandItem->argument[0].toInt();
            if (bes > bowControlArray[currentBowSerial].bowSpeedToEngage) { bes = bowControlArray[currentBowSerial].bowSpeedToEngage; }
            bowControlArray[currentBowSerial].bowSpeedWhileEngaged = bes;
            debugPrintln("Setting bow modulation speed to " + String(bowControlArray[currentBowSerial].bowSpeedWhileEngaged), Command);
        }
    } else
    if (_commandItem->command == "bowengagespeed") {
        if (request) {
            commandResponses->push_back({ "bes:" + String(bowControlArray[currentBowSerial].bowSpeedToEngage), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            uint16_t bes = _commandItem->argument[0].toInt();
            if (bes > 100) { bes = 100; }
            bowControlArray[currentBowSerial].bowSpeedToEngage = bes;
            debugPrintln("Setting bow engage speed to " + String(bowControlArray[currentBowSerial].bowSpeedToEngage), Command);
        }
    } else {
        return false;
    }
    return true;
}

bool stringModule::processSerialCommand_MuteControl(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
//        debugPrintln("Mute size " + String(muteArray.size()) + " current " + String(currentBowSerial), Debug);
    if (currentBowSerial >= int(muteArray.size())) {
        //debugPrintln("Mute out of bounds", Error);
        return false;
    }
    if (_commandItem->command == "mutesetposition") {
        if (request) {
            commandResponses->push_back({ "msp:" + String(muteArray[currentBowSerial].getTilt()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            muteArray[currentBowSerial].setTilt(_commandItem->argument[0].toInt());
            debugPrintln("Setting mute to " + String(muteArray[currentBowSerial].getTilt()), Command);
        }
    } else
    if (_commandItem->command == "mutefullmute") {
        if (request) {
        } else {
            muteArray[currentBowSerial].fullMute();
            debugPrintln("Engaging full mute", Command);
        }
    } else
    if (_commandItem->command == "mutehalfmute") {
        muteArray[currentBowSerial].halfMute();
        debugPrintln("Engaging half mute", Command);
    } else
    if (_commandItem->command == "muterest") {
        muteArray[currentBowSerial].rest();
        debugPrintln("Engaging mute rest", Command);
    } else
    if (_commandItem->command == "mutesavefull") {
        muteArray[currentBowSerial].saveFullMute();
        debugPrintln("Saving full mute position", Command);
    } else
    if (_commandItem->command == "mutesavehalf") {
        muteArray[currentBowSerial].saveHalfMute();
        debugPrintln("Saving half mute position", Command);
    } else
    if (_commandItem->command == "mutesaverest") {
        muteArray[currentBowSerial].saveRest();
        debugPrintln("Saving rest position", Command);
    } else
/*        if (_commandItem->command == "mutesaveparams") {
        muteArray[currentBowSerial].saveParams();
        debugPrintln("Saving mute parameters in EEPROM", Command);
    } else*/
    if (_commandItem->command == "mutefullmuteposition") {
        if (request) {
            commandResponses->push_back({ "mfmp:" + String(muteArray[currentBowSerial].getFullMutePosition()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            muteArray[currentBowSerial].setFullMutePosition(_commandItem->argument[0].toInt());
            debugPrintln("Setting full mute position to " + String(muteArray[currentBowSerial].getFullMutePosition()), Command);
        }
    } else
    if (_commandItem->command == "mutehalfmuteposition") {
        if (request) {
            commandResponses->push_back({ "mhmp:" + String(muteArray[currentBowSerial].getHalfMutePosition()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            muteArray[currentBowSerial].setHalfMutePosition(_commandItem->argument[0].toInt());
            debugPrintln("Setting half mute position to " + String(muteArray[currentBowSerial].getHalfMutePosition()), Command);
        }
    } else
    if (_commandItem->command == "muterestposition") {
        if (request) {
            commandResponses->push_back({ "mrp:" + String(muteArray[currentBowSerial].getRestPosition()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            muteArray[currentBowSerial].setRestPosition(_commandItem->argument[0].toInt());
            debugPrintln("Setting rest position to " + String(muteArray[currentBowSerial].getRestPosition()), Command);
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
            debugPrintln("Setting sustain to " + String(muteArray[currentBowSerial].getSustain()), Command);
        }
    } else {
        return false;
    }
    return true;
}

bool stringModule::processSerialCommand_StatusTesting(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false, commandList *delegatedCommands = nullptr) {
    if (_commandItem->command == "status") {
        debugPrint("Set frequency " + String(bowControlArray[currentBowSerial].getPIDTarget()) + " Hz, ", TextInfo);
        debugPrint("Read frequency " + String(bowIOArray[currentBowSerial].averageFreq()) + " Hz", TextInfo);
        debugPrint(" P gain " + String(bowControlArray[currentBowSerial].Kp), TextInfo);
        debugPrint(" I gain " + String(bowControlArray[currentBowSerial].Ki), TextInfo);
        debugPrint(" D gain " + String(bowControlArray[currentBowSerial].Kd), TextInfo);
        debugPrint(" ignore below " + String(bowControlArray[currentBowSerial].integratorIgnoreBelow), TextInfo);
        debugPrintln(" max error " + String(bowControlArray[currentBowSerial].pidMaxError), TextInfo);
    } else
    if (_commandItem->command == "measuretimetotarget") {
        bowControlArray[currentBowSerial].measureTimeToTarget(_commandItem->argument[0].toInt());
    }  else
    if (_commandItem->command == "stringfrequency") {
        if (request) {
            if (audioFrequencyAvaliable()) {
                commandResponses->push_back({ "sf:" + String(audioFrequency(),1), InfoRequest });
            } else {
                commandResponses->push_back({ "sf: 0", InfoRequest });
            }
        }
    }  else
    if (_commandItem->command == "bowfrequency") {
        if (request) {
            commandResponses->push_back({ "bf:" + String(bowIOArray[currentBowSerial].averageFreq()), InfoRequest });
        }
    }  else
    if (_commandItem->command == "pidpeakerror") {
        if (request) {
            commandResponses->push_back({ "pidperr:" + String(bowControlArray[currentBowSerial].getPIDPeakError()), InfoRequest });
        }
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
            bowControlArray[currentBowSerial].bowActuators->setBowActuator(_commandItem->argument[0].toInt());
            debugPrintln("Setting bow actuator to " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), Command);
        }
    }  else
    if (_commandItem->command == "bowactuatorload") {
        bowControlArray[currentBowSerial].bowActuators->loadBowActuator();
        debugPrintln("Loaded data for actuator " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), Command);
    }  else
    if (_commandItem->command == "bowactuatorsave") {
        bowControlArray[currentBowSerial].bowActuators->saveBowActuator();
        debugPrintln("Saving data for actuator " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()), Command);
    }  else
    if (_commandItem->command == "bowactuatordata") {
        if (request) {
            commandResponses->push_back({ "bad:" +  String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorFirstTouchPressure()) + ":"  +
                String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorStallPressure()) + ":" +
                String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorRestPosition()) + ":" +
                String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorID()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 4)) { return false; }
            bowControlArray[currentBowSerial].bowActuators->setBowActuatorData(_commandItem->argument[0].toInt(), _commandItem->argument[1].toInt(),
                _commandItem->argument[2].toInt(), _commandItem->argument[3]);
            debugPrintln("Setting bow actuator data for bow " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuator()) + "  ID: " +
                _commandItem->argument[3] + ", first touch: " + String(_commandItem->argument[0]) + ", stall pressure: " + _commandItem->argument[1] +
                ", rest position: " + _commandItem->argument[2] , Command);
        }
    } else
    if (_commandItem->command == "bowactuatorid") {
        if (request) {
            commandResponses->push_back({ "bai:" + String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorID()), InfoRequest });
        } else {
            if (!checkArguments(_commandItem, commandResponses, 1)) { return false; }
            bowControlArray[currentBowSerial].bowActuators->setBowActuatorID(_commandItem->argument[0]);
            debugPrintln("Setting bow actuator ID to " + String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorID()), Command);
        }
    }  else
    if (_commandItem->command == "bowactuatorcount") {
        commandResponses->push_back({ "bac:" + String(bowControlArray[currentBowSerial].bowActuators->getBowActuatorCount()), InfoRequest });
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
