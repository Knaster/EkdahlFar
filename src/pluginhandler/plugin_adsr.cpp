#ifndef PLUGIN_AHDSR_CPP
#define PLUGIN_AHDSR_CPP

#include "plugins/plugin_adsr.hpp"

#include <cmath>

const ModuleCommandDeclaration Plugin_AHDSR::moduleCommands[] = {
    { "name", "na", "name", "Sets the LFO name", false, true, &s_name, eCommandType::Name },
    { "enable", "en", "1|0", "Will enable or disable the output of the target commands", false, true, &s_enable, eCommandType::SimpleBool },
    { "target", "tg", "commandlist*", "Sets the command string to execute each iteration, the string [ahdsrout] will be replaced with the current value", false, true, &s_target, eCommandType::OutputAssignment },
    { "updaterate", "ur", "mS", "Sets the frequency with which the target commands are executed. Limited by the global maximum as set in the plugin handler", false, true, &s_updaterate, eCommandType::Milliseconds },
    { "gate", "gt", "0|1", "Gate signal, [1] initiates the attack phase and if it reaches its maximum value, continues through the decay down to the sustain point. A [0] initiates the release from whatever point the ADSR is at.", false, true, &s_gate, eCommandType::SimpleBool },
    { "attack", "at", "mS", "Attack time in mS", false, true, &s_attack, eCommandType::Milliseconds },
    { "hold", "hd", "mS", "Hold time in mS", false, true, &s_hold, eCommandType::Milliseconds },
    { "decay", "dc", "mS", "Delay time in mS", false, true, &s_decay, eCommandType::Milliseconds },
    { "sustain", "su", "mS", "Sustain point", false, true, &s_sustain, eCommandType::SimpleUInt16 },
    { "release", "re", "mS", "Release time in mS", false, true, &s_release, eCommandType::Milliseconds },
    { "releasetarget", "rt", "commands", "Commands to execute once the release has hit zero", false, true, &s_releasetarget, eCommandType::OutputAssignment },
    { "amplitude", "amp", "0-65535", "Output amplitude", false, true, &s_amplitude, eCommandType::SimpleUInt16 },
    { "invert", "inv", "1|0", "Inverts the results and gives negative values", false, true, nullptr, eCommandType::SimpleBool }
};

getModuleCount(Plugin_AHDSR)

Plugin_AHDSR::Plugin_AHDSR() {
//    moduleID = new ModuleID("ahdsr", "ahdsr", "Basic software AHDSR v1.0", eModuleType::software, false);
    recalculateRates();
}

CREATE_GETSET_FUNCTION(name, Plugin_AHDSR, pName)

CREATE_MODULE_COMMAND_FUNCTION(enable, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pEnable = inCommandItem->argument[0].toInt();

        if (pEnable == 0) {
            String newTarget = pTarget;
            newTarget = newTarget.replace("ahdsrout", "0");
            globalResponseCommands.addCommands(newTarget);
        }
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pEnable), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(target, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pTarget = stripQuotes(inCommandItem->argument[0]);
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(pTarget, true), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(updaterate, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pUpdateRate = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pUpdateRate), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(gate, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pGate = inCommandItem->argument[0].toInt();
        if (pGate) {
            pReleased = false;
            if (pAttack == 0) {
                pValue = 65535;
                pHoldValue = 0;
                pStage = sHold;
            } else {
                pStage = sAttack;
            }
        } else {
            pStage = sRelease;
        }
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pGate), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(attack, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pAttack = inCommandItem->argument[0].toInt();
        recalculateRates();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pAttack), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(decay, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pDecay = inCommandItem->argument[0].toInt();
        recalculateRates();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pDecay), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(hold, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pHold = inCommandItem->argument[0].toInt();
        recalculateRates();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pHold), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(release, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pRelease = inCommandItem->argument[0].toInt();
        recalculateRates();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pRelease), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(sustain, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pSustain = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pSustain), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(amplitude, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pAmplitude = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pAmplitude), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(releasetarget, Plugin_AHDSR) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pReleaseTarget = stripQuotes(inCommandItem->argument[0]);
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(pReleaseTarget, true), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(invert, Plugin_AHDSR) {
    return eProcessResult::Ok;
}

void Plugin_AHDSR::update() {
//    debugPrintln("Stage: " + String(pStage), debugPrintType::Debug);
    switch(pStage) {
    case eStage::sAttack:
        pValue += pAttackRate;
        if (pValue < 65535) { break; }
        pValue = 65535;
        pHoldValue = 0;
        pStage = sHold;
    case eStage::sHold:
        pHoldValue += pHoldRate;
        if (pHoldValue < 65535) { break; }
        pStage = sDecay;
        pHoldValue = 0;
    case eStage::sDecay:
        if (pValue > pSustain) { pValue -= pDecayRate; break; }
    case eStage::sSustain:
        pValue = pSustain;
        break;
    case eStage::sRelease:
        pValue -= pReleaseRate;
        if (pValue < 0) {
            pValue = 0;
            if (!pReleased) {
                pReleased = true;
                globalResponseCommands.addCommands(pReleaseTarget);
            }
        }
    }

    int32_t out = ((float) (pValue * pAmplitude) / 65535);

    if ((pEnable) && ((out != pLastValue) || (pStage == sHold))) {
        String newTarget = pTarget;
        newTarget = newTarget.replace("ahdsrout", String(out));
        globalResponseCommands.addCommands(newTarget);
        //debugPrintln("Value: " + String(out) + " hold: " + String(pHoldValue), debugPrintType::Debug);
        pLastValue = out;
    }
}

void Plugin_AHDSR::recalculateRates() {
    float parentRate = 0.01;
    if (parent != nullptr) {
        parentRate = 0.001 * ((PluginHandler*) parent)->getUpdateRate();
    }

    pAttackRate = convertMsToRate(parentRate, pAttack);
    pHoldRate = convertMsToRate(parentRate, pHold);
    pDecayRate = convertMsToRate(parentRate, pDecay);
    pReleaseRate = convertMsToRate(parentRate, pRelease);
//    debugPrintln("a:" + String(pAttackRate) + " h:" + String(pHoldRate) + " d:" + String(pDecayRate) +" s:" + String(pSustain) +" r:" + String(pReleaseRate), debugPrintType::Debug);
}

#endif
