#ifndef BOWPRESSURE_C
#define BOWPRESSURE_C

#include "bowpressure.hpp"

const ModuleCommandDeclaration BowPressure::moduleCommands[] = {
    { "baseline", "ba", "0-65535", "Bow pressure baseline, modulation is added to this point upward", false, false, &s_baseline },
    { "modifier", "mo", "0-65535", "Bow pressure modulation, added to the baseline", false, false, &s_modifier },
    { "rest", "rs", "0|1", "Puts the bow pressure in the resting position (conditional)", false, false, &s_rest },
    { "engage", "en", "0|1", "Puts the bow pressure in the engage position (conditional)", false, false, &s_engage },
    { "stallpressure", "sp", "0-65535", "Bow pressure stall/maximum position", false, false, &s_stallPressure },
    { "engagepressure", "ep", "0-65535", "Bow pressure touch/minimum position", false, false, &s_engagePressure },
    { "restpressure", "rp", "0-65535", "Bow pressure rest position", false, false, &s_restPressure},
    { "engagespeed", "es", "1 - 100?", "Bow pressure movement speed when engaging or disengaging", false, false, &s_engageSpeed},
    { "modulationspeed", "ms", "0.1 - 10", "Bow pressure movement speed while engaged", false, false, &s_modulationSpeed},
    { "hold", "hd", "0|1", "Sets bow hold on/off", false, false, &s_hold },
    { "home", "hm", "-", "Homing bow, used at startup and in case of the bow loosing position", false, false, &s_home },
    { "tmcinfo", "tmi", "-", "Request statistical information from the TMC2209", true, false, &s_tmcinfo }
};

getModuleCount(BowPressure)

BowPressure::BowPressure(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin) {
    moduleID = new ModuleID("bowpressure", "bp", "Bowing pressure controller v1.0", ModuleID::hardware);
    tmc2209ServoStepper = new Tmc2209ServoStepper(stepDirPin, stepStepPin, stepSerialPort, stepHomeSensorPin);
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
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(maxPressure), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        maxPressure = inCommandItem->argument[0].toInt();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(maxPressure), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(engagePressure, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pEngagePressure), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pEngagePressure = inCommandItem->argument[0].toInt();
        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(pEngagePressure), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(restPressure, BowPressure) {
    if (request) {
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pRestPressure), InfoRequest });
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pRestPressure = inCommandItem->argument[0].toInt();
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pRestPressure), InfoRequest });
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
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    if (inCommandItem->argument[0].toInt() > 0) { setHold(true); } else { setHold(false);}
    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(inCommandItem->argument[0].toInt()), InfoRequest});
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

/***** Module specific commands mirroring serially attainable commands *****/

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
    if (pHold) { return true; }

    reachedEngage = false;
    setPressureSafe(pRestPressure);
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
    if (pressure > maxPressure) { pressure = maxPressure; }

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
    unsigned int pressure = pEngagePressure + ((double)(maxPressure - pEngagePressure) / 65535 * ((double)(baselinePressure + modifierPressure)));
    setPressureSafe(pressure);
    return true;
}
#endif // BOWPRESSURE_C
