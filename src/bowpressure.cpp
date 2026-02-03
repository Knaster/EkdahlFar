#ifndef BOWPRESSURE_C
#define BOWPRESSURE_C

#include "bowpressure.hpp"

const ModuleCommandDeclaration BowPressure::moduleCommands[] = {
    { "baseline", "ba", "0-65535", "Bow pressure baseline, modulation is added to this point upward", false, false, &s_baseline, eCommandType::SimpleUInt16 },
    { "modifier", "mo", "0-65535", "Bow pressure modulation, added to the baseline", false, false, &s_modifier, eCommandType::SimpleUInt16 },
    { "rest", "rs", "0|1", "Puts the bow pressure in the resting position (conditional)", false, false, &s_rest, eCommandType::Conditional },
    { "engage", "en", "0|1", "Puts the bow pressure in the engage position (conditional)", false, false, &s_engage, eCommandType::Conditional },
    { "stallpressure", "sp", "0-65535", "Bow pressure stall/maximum position", false, false, &s_stallPressure, eCommandType::SimpleUInt16 },
    { "engagepressure", "ep", "0-65535", "Bow pressure touch/minimum position", false, false, &s_engagePressure, eCommandType::SimpleUInt16 },
    { "restpressure", "rp", "0-65535", "Bow pressure rest position", false, false, &s_restPressure, eCommandType::SimpleUInt16},
    { "engagespeed", "es", "1 - 100?", "Bow pressure movement speed when engaging or disengaging", false, false, &s_engageSpeed, eCommandType::SimpleUInt8 },
    { "modulationspeed", "ms", "0.1 - 10", "Bow pressure movement speed while engaged", false, false, &s_modulationSpeed, eCommandType::SimpleUInt8 },
    { "hold", "hd", "0|1", "Sets bow hold on/off", false, false, &s_hold, eCommandType::SimpleBool },
    { "home", "hm", "-", "Homing bow, used at startup and in case of the bow loosing position", false, false, &s_home, eCommandType::Immediate },
    { "tmcinfo", "tmi", "-", "Request statistical information from the TMC2209", true, false, &s_tmcinfo, eCommandType::RequestOnly }
};

getModuleCount(BowPressure)

BowPressure::BowPressure(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
//    moduleID = new ModuleID("bowpressure", "bp", "Bowing pressure controller v1.0", eModuleType::hardware);
    tmc2209ServoStepper = new Tmc2209ServoStepper(stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);

    bowActuators = new BowActuators();
    addModule(bowActuators);
}

CREATE_MODULE_COMMAND_FUNCTION(baseline, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(baselinePressure), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setPressureBaseline(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(baselinePressure), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(modifier, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(modifierPressure), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setPressureModifier(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(rest, BowPressure) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    rest(inCommandItem->argument[0].toInt());
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(engage, BowPressure) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    engage(inCommandItem->argument[0].toInt());
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(inCommandItem->argument[0].toInt()), InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(stallPressure, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getStallPressure()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setStallPressure(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(getStallPressure()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(engagePressure, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getEngagePressure()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setEngagePressure(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(getEngagePressure()), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(restPressure, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getRestPressure()), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        setRestPressure(inCommandItem->argument[0].toInt());
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(getRestPressure()), InfoRequest });
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(engageSpeed, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(speedToEngage), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        uint16_t bes = inCommandItem->argument[0].toInt();
        if (bes > 100) { bes = 100; }
        speedToEngage = bes;
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(speedToEngage), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(modulationSpeed, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(speedWhileEngaged), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        uint16_t bes = inCommandItem->argument[0].toInt();
        if (bes > speedWhileEngaged) { bes = speedToEngage; }
        speedWhileEngaged = bes;
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(speedWhileEngaged), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(hold, BowPressure) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (inCommandItem->argument[0].toInt() > 0) { setHold(true); } else { setHold(false);}
    }
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(getHold()), InfoRequest});
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(home, BowPressure) {
    if (!request) {
        if (home()) {
            inCommandResponses->push_back({thisItem.shortCommand + ":1", InfoRequest});
        } else {
            inCommandResponses->push_back({thisItem.shortCommand + ":0", InfoRequest});
        }
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(tmcinfo, BowPressure) {
    getTMC2209Info();
    return eProcessResult::Ok;
}

BowActuator* BowPressure::currentActuator() {
    ModuleGroup *ahGroup = getGroup("actuatorhandler");
    if (ahGroup == nullptr) {
        debugPrintln("Actuator handler group not found", debugPrintType::Error);
        return nullptr;
    }
    ModuleGroup *acGroup = ((ModuleHandler*) (ahGroup->modules[0]))->getGroup("actuator");
    if (acGroup == nullptr) {
        debugPrintln("Actuator group not found", debugPrintType::Error);
        return nullptr;
    }
    BowActuator *actuator = acGroup->getSingleSelection();
    if (actuator == nullptr) {
        debugPrintln("Actuator not found", debugPrintType::Error);
        return nullptr;
    }
    return actuator;
}

/***** Module specific commands mirroring serially attainable commands *****/
bool BowPressure::setRestPressure(uint16_t inRestPressure) {
    BowActuator *cAC = currentActuator();
    if (cAC == nullptr) { return false; }
    cAC->restPosition = inRestPressure;
    return true;
}

uint16_t BowPressure::getRestPressure() {
    BowActuator *cAC = currentActuator();
    if (cAC == nullptr) { return false; }
    return cAC->restPosition;
}

bool BowPressure::setEngagePressure(uint16_t inEngagePressure) {
    BowActuator *cAC = currentActuator();
    if (cAC == nullptr) { return false; }
    cAC->firstTouchPressure = inEngagePressure;
    return true;
}

uint16_t BowPressure::getEngagePressure() {
    BowActuator *cAC = currentActuator();
    if (cAC == nullptr) { return false; }
    return cAC->firstTouchPressure;
}

bool BowPressure::setStallPressure(uint16_t inStallPressure) {
    BowActuator *cAC = currentActuator();
    if (cAC == nullptr) { return false; }
    cAC->stallPressure = inStallPressure;
    return true;
}

uint16_t BowPressure::getStallPressure() {
    BowActuator *cAC = currentActuator();
    if (cAC == nullptr) { return false; }
    return cAC->stallPressure;
}

void BowPressure::setHardwarePressure(uint16_t pressure) {
    if ((tmc2209ServoStepper != nullptr) && (tmc2209ServoStepper->stepServoStepper != nullptr)) {
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

bool BowPressure::rest(bool enact) {
    if (enact == 0) { return false; }
    pressureMode = ePressureMode::Rest;
    setRestSignal();
    if (pHold) { return true; }

    reachedEngage = false;
    setPressureSafe(getRestPressure());
    return true;
}

void BowPressure::setHold(bool inHold) {
    pHold = !inHold;
    if ((pHold == false) && (pressureMode == Rest)) { rest(1); }
}

bool BowPressure::engage(bool enact) {
    if (enact == 0) { return false; }
    reachedEngage = false;
    tmc2209ServoStepper->stepServoStepper->reachedTarget = false;
    pressureMode = ePressureMode::Engage;
    return calculateBaselineModifierPressure();
}

void BowPressure::getTMC2209Info() { return tmc2209ServoStepper->getTMC2209Info(); }

bool BowPressure::getRestSignal() {
    if (restSignal) {
        restSignal = false;
        return true;
    }
    return false;
}

/***** Hidden commands for modular use *****/
/***** Internal commands for stand-alone and semi-modular use *****/

void BowPressure::update() {
    if (tmc2209ServoStepper->stepServoStepper->intDrivenMsgFlag) {
        debugRaw(tmc2209ServoStepper->stepServoStepper->intDrivenMsg);
        tmc2209ServoStepper->stepServoStepper->intDrivenMsgFlag = false;
    }
}

/***** Internal commands for debugging use, most likely to be removed *****/

//void BowPressure::setStepperID(uint16_t stepperID) { tmc2209ServoStepper->stepServoStepper->stepperID = stepperID; }

/***** Private commands *****/

void BowPressure::setPressureSafe(uint16_t pressure) {
    if (pressure > getStallPressure()) { pressure = getStallPressure(); }

    if ((pressureMode == ePressureMode::Engage) && (tmc2209ServoStepper->stepServoStepper->reachedTarget))  {
        reachedEngage = true;
    }

    if ((pressureMode == ePressureMode::Engage) && (reachedEngage)) {
        tmc2209ServoStepper->stepServoStepper->setSpeed(speedWhileEngaged);
    } else {
        tmc2209ServoStepper->stepServoStepper->setSpeed(speedToEngage);
    }

    setHardwarePressure(pressure);
    return;
}

/// Calculate the tilt PWM value using baselineTiltPWM and modfierTiltPWM and send it to BowIO
bool BowPressure::calculateBaselineModifierPressure() {
    uint16_t pEngagePressure = getEngagePressure();
    unsigned int pressure = pEngagePressure + ((double)(getStallPressure() - pEngagePressure) / 65535 * ((double)(baselinePressure + modifierPressure)));
    setPressureSafe(pressure);
    return true;
}
#endif // BOWPRESSURE_C
