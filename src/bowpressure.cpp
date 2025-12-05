#ifndef BOWPRESSURE_C
#define BOWPRESSURE_C

#include "bowpressure.hpp"

BowPressure::BowPressure(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
    tmc2209ServoStepper = new Tmc2209ServoStepper(stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
//    home();
}

eProcessResult BowPressure::processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                           commandList *delegatedCommands = nullptr) {

    processCommandItems(inCommandItem, serialCommandsBowPressure, sizeof(serialCommandsBowPressure) / sizeof(serialCommandItem));

    if (inCommandItem->command == "help") {
        addCommandHelp(serialCommandsBowPressure, sizeof(serialCommandsBowPressure) / sizeof(serialCommandItem), commandResponses,"");
        return eProcessResult::PassThrough;
    } else
    if (inCommandItem->command == "bowpressurebaseline") {
        if (request) {
            commandResponses->push_back({ "bpb:" + String(baselinePressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setPressureBaseline(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"bpb:" + String(baselinePressure), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressuremodifier") {
        if (request) {
            commandResponses->push_back({ "bpm:" + String(modifierPressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            setPressureModifier(inCommandItem->argument[0].toInt());
            commandResponses->push_back({"bpm:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressurerest") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        rest(inCommandItem->argument[0].toInt());
        commandResponses->push_back({"bpr:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "bowpressureengage") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        engage(inCommandItem->argument[0].toInt());
        commandResponses->push_back({"bpe:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else
    if (inCommandItem->command == "bowpressurepositionmax") {
        if (request) {
            commandResponses->push_back({ "bppx:" + String(maxPressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            maxPressure = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bppx:" + String(maxPressure), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressurepositionengage") {
        if (request) {
            commandResponses->push_back({ "bppe:" + String(engagePressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            engagePressure = inCommandItem->argument[0].toInt();
            commandResponses->push_back({"bppe:" + String(engagePressure), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressurepositionrest") {
        if (request) {
            commandResponses->push_back({ "bppr:" + String(restPressure), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            restPressure = inCommandItem->argument[0].toInt();
            commandResponses->push_back({ "bppr:" + String(restPressure), InfoRequest });
        }
    } else
    if (inCommandItem->command == "bowpressureengagespeed") {
        if (request) {
            commandResponses->push_back({ "bpes:" + String(speedToEngage), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            uint16_t bes = inCommandItem->argument[0].toInt();
            if (bes > 100) { bes = 100; }
            speedToEngage = bes;
            commandResponses->push_back({"bpes:" + String(speedToEngage), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressuremodulationspeed") {
        if (request) {
            commandResponses->push_back({ "bpms:" + String(speedWhileEngaged), InfoRequest });
        } else {
            if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
            uint16_t bes = inCommandItem->argument[0].toInt();
            if (bes > speedWhileEngaged) { bes = speedToEngage; }
            speedWhileEngaged = bes;
            commandResponses->push_back({"bpms:" + String(speedWhileEngaged), InfoRequest});
        }
    } else
    if (inCommandItem->command == "bowpressurehold") {
        if (!checkArguments(inCommandItem, commandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() > 0) { setHold(true); } else { setHold(false);}
        commandResponses->push_back({"bph:" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    } else {
        return eProcessResult::NotFound;
    }

    return eProcessResult::Ok;
}

eProcessResult BowPressure::processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                commandList *delegatedCommands = nullptr) {
    return eProcessResult::NotFound;
}

String BowPressure::dumpData() {
    String dump = "";
    dump += "bpes:" + String(speedToEngage) + ",";
    dump += "bpms:" + String(speedWhileEngaged);
    return dump;
}
/***** Module specific commands mirroring serially attainable commands *****/

void BowPressure::setHardwarePressure(uint16_t pressure) {
    if ((tmc2209ServoStepper != nullptr) && (tmc2209ServoStepper->stepServoStepper != nullptr)) {
        debugPrintln("Setting pressure to " + String(pressure), Hardware);
        tmc2209ServoStepper->stepServoStepper->setPosition(pressure);
        lastPressure = pressure;
    }
}

void BowPressure::setPressureBaseline(uint16_t baseline) {
    baselinePressure = baseline;
    calculateBaselineModifierPressure();
}

uint16_t BowPressure::getPressureBaseline() { return baselinePressure; }

void BowPressure::setPressureModifier(uint16_t modifier) {
    modifierPressure = modifier;
    if (pressureMode == ePressureMode::Engage) {
        calculateBaselineModifierPressure();
    }
}

void BowPressure::setMaxPressure(uint16_t inMaxPressure) { maxPressure = inMaxPressure; }

uint16_t BowPressure::getMaxPressure() { return maxPressure; }

void BowPressure::setEngagePressure(uint16_t inEngagePressure) { engagePressure = inEngagePressure; }

uint16_t BowPressure::getEngagePressure() { return engagePressure; }

void BowPressure::setRestPressure(uint16_t inRestPressure) { restPressure = inRestPressure; }

uint16_t BowPressure::getRestPressure() { return restPressure; }


bool BowPressure::rest(bool enact) {
    if (enact == 0) { return false; }
    pressureMode = ePressureMode::Rest;
    if (hold) { return true; }

    reachedEngage = false;
/*
    bowShutoffTimer = 0;
    bowShutoffTimedout = false;         // added 2024-06-27
    bowShutoffMotorDisabled = false;    // added 2024-06-27
*/
    setPressureSafe(restPressure);
    return true;
}

void BowPressure::setHold(bool inHold) {
    hold = !inHold;
//    if (outputDebugData) { debugPrintln("Hold " + String(hold), Debug); }
    if ((hold == false) && (pressureMode == Rest)) { rest(1); }
}

bool BowPressure::getHold() { return hold; }

ePressureMode BowPressure::getPressureMode() { return pressureMode; }

//void BowPressure::setPressureMode(ePressureMode inPressureMode) { pressureMode = inPressureMode; }


bool BowPressure::engage(bool enact) {
    if (enact == 0) { return false; }
    reachedEngage = false;
    tmc2209ServoStepper->stepServoStepper->reachedTarget = false;
    pressureMode = ePressureMode::Engage;
    return calculateBaselineModifierPressure();
}

bool BowPressure::home(bool invert) { return tmc2209ServoStepper->home(invert); }

//bool BowPressure::setHardwarePosition(uint16_t position) { return tmc2209ServoStepper->stepServoStepper->setPosition(position); }

bool BowPressure::completeTask(uint16_t timeout) { return tmc2209ServoStepper->stepServoStepper->completeTask(timeout); }

void BowPressure::setAutoCorrect(bool autoCorrect) { tmc2209ServoStepper->stepServoStepper->autoCorrectPosition = autoCorrect; }

bool BowPressure::getAutoCorrect() {return tmc2209ServoStepper->stepServoStepper->autoCorrectPosition; };

void BowPressure::setSpeed(float speed) { tmc2209ServoStepper->stepServoStepper->setSpeed(speed); }

float BowPressure::getSpeed() { return tmc2209ServoStepper->stepServoStepper->getSpeed();}


/*
bool BowPressure::getHomingSensed() { return tmc2209ServoStepper->stepServoStepper->getHomingSensed(); };

eStepDirection BowPressure::getMoveDirection() { return tmc2209ServoStepper->stepServoStepper->getMoveDirection(); }

uint16_t BowPressure::getCurrentStep() { return tmc2209ServoStepper->stepServoStepper->getCurrentStep(); };

uint16_t BowPressure::getHomingPoint(uint8_t x, uint8_t y) { return tmc2209ServoStepper->stepServoStepper->getHomingPoint(x,y); }

eHomingStage BowPressure::getHomingStage() { return tmc2209ServoStepper->stepServoStepper->getHomingStage(); };
*/
void BowPressure::getTMC2209Info() { return tmc2209ServoStepper->getTMC2209Info(); }

/***** Hidden commands for modular use *****/
/***** Internal commands for stand-alone and semi-modular use *****/
/*
void BowPressure::update();          // Update function to be periodically called in order for backoff and other things to work
void BowPressure::updateServo();     // Handles time-critical timing of stepper motor update - automatically called through stepInervalCallback - DO NOT MANUALLY CALL THIS
void BowPressure::setStepIntervalCallback(void *stepIntervalCallback);   // This is to be set to a static function that in turn will call the class instance of updateMuteServo
*/
/***** Internal commands for debugging use, most likely to be removed *****/

void BowPressure::setStepperID(uint16_t stepperID) { tmc2209ServoStepper->stepServoStepper->stepperID = stepperID; }

/***** Private commands *****/

void BowPressure::setPressureSafe(uint16_t pressure) {
    if (pressure > maxPressure) { pressure = maxPressure; }

    if ((pressureMode == ePressureMode::Engage) && (tmc2209ServoStepper->stepServoStepper->reachedTarget))  {
        reachedEngage = true;
//        if (outputDebugData) { debugPrintln("Reached engage", debugPrintType::Debug); }
    }

    if ((pressureMode == ePressureMode::Engage) && (reachedEngage)) {
        tmc2209ServoStepper->stepServoStepper->setSpeed(speedWhileEngaged);
        //if (outputDebugData) { debugPrintln("Setting pressure speed to slow", debugPrintType::Debug); }
    } else {
        tmc2209ServoStepper->stepServoStepper->setSpeed(speedToEngage);
//        if (outputDebugData) { debugPrintln("Setting pressure speed to high", debugPrintType::Debug); }
    }

    setHardwarePressure(pressure);
    return;
}

/// Calculate the tilt PWM value using baselineTiltPWM and modfierTiltPWM and send it to BowIO
bool BowPressure::calculateBaselineModifierPressure() {
    unsigned int pressure = engagePressure + ((double)(maxPressure - engagePressure) / 65535 * ((double)(baselinePressure + modifierPressure)));
    setPressureSafe(pressure);
    return true;
}

#endif // BOWPRESSURE_C
