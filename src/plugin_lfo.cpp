#ifndef PLUGIN_LFO_CPP
#define PLUGIN_LFO_CPP

#include "plugin_lfo.hpp"

#include <cmath>

const ModuleCommandDeclaration Plugin_LFO::moduleCommands[] = {
    { "enable", "en", "1|0", "Will enable or disable the output of the target commands", false, true, &s_enable },
    { "target", "tg", "commandlist*", "Sets the command string to execute each iteration, the string 'lfoout' will be replaced with the current value", false, true, &s_target },
    { "updaterate", "ur", "mS", "Sets the frequency with which the target commands are executed. Limited by the global maximum as set in the plugin handler", false, true, &s_updaterate },

    { "waveform", "wf", "sine|triangle|square|saw", "Sets the waveform", false, true, &s_waveform },
    { "frequency", "fq", "Hz", "Sets the frequency of the LFO", false, true, &s_frequency },
    { "amplitude", "amp", "0-65535", "Sets the output amplitude of the LFO", false, true, &s_amplitude },
    { "delay", "dl", "ms", "Sets the amplitude ramp up delay, from 1ms to 65s", false, true, &s_delay },
    { "resetdelay", "rd", "-", "Resets the delay count to zero and reset the waveform", false, false, &s_resetdelay },
    { "resetwave", "rw", "-", "Resets the waveform so it starts over", false, false, &s_resetwave },
    { "bipolar", "bp", "1|0", "Sets whether the waveform is bipolar (default) or positive only", false, true, &s_bipolar }
};

getModuleCount(Plugin_LFO)

Plugin_LFO::Plugin_LFO()
{
    moduleID = new ModuleID("lfo", "lfo", "Basic software LFO v1.0", eModuleType::software, false);
    recalculateIncrease();
}

CREATE_MODULE_COMMAND_FUNCTION(enable, Plugin_LFO) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pEnable = inCommandItem->argument[0].toInt();

        if (pEnable == 0) {
            String newTarget = pTarget;
            newTarget = newTarget.replace("lfoout", "0");
            globalResponseCommands.addCommands(newTarget);
        }
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pEnable), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(target, Plugin_LFO) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pTarget = stripQuotes(inCommandItem->argument[0]);
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(pTarget, true), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(updaterate, Plugin_LFO) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pUpdateRate = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pUpdateRate), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(waveform, Plugin_LFO) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        for (int i = 0; i < WAVEFORMS; i++) {
            if (inCommandItem->argument[0] == pWaveData[i].name) {
                pWaveform = pWaveData[i].waveform;
                inCommandResponses->push_back({ thisItem.shortCommand + ":" + pWaveData[pWaveform].name, debugPrintType::InfoRequest });
                return eProcessResult::Ok;
            }
        }
        return eProcessResult::WrongArgumentValue;
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + pWaveData[pWaveform].name, debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(frequency, Plugin_LFO) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pFrequency = inCommandItem->argument[0].toFloat();
        recalculateIncrease();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pFrequency), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(amplitude, Plugin_LFO) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pAmplitude = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pAmplitude), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(delay, Plugin_LFO) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pDelay = inCommandItem->argument[0].toInt();
        recalculateIncrease();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pDelay), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(resetdelay, Plugin_LFO) {
    pDelayCount = 0;
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(resetwave, Plugin_LFO) {
    pCount = 0;
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(bipolar, Plugin_LFO) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pBipolar = inCommandItem->argument[0].toInt();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pBipolar), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

void Plugin_LFO::update() {
    pCount += calculatedIncrease;
    if (pCount > 65535) { pCount -= 65535; }

    pDelayCount += delayIncrease;
    if (pDelayCount > 65535) { pDelayCount = 65535; }

    float fOut;
    switch(pWaveform) {
        case eWaveform::triangle:
            fOut = shapeTriangle();
            break;
        case eWaveform::sine:
            fOut = shapeSine();
            break;
        case eWaveform::square:
            fOut = shapeSquare();
            break;
        default:
            fOut = pCount - 32767;
    }

    if (!pBipolar) { fOut += 32767; }

    uint16_t amp = pAmplitude / 2;
    int32_t out = 0;
    if (amp >= 32767) {
        out = fOut;
    } else if (amp < 1) {
        out = 0;
    } else {
        out = fOut / (32767 / amp);
    }

    if (pDelayCount > 0) { out = out / (65535 / pDelayCount); }

    if (pBipolar) {
        if (out < -32767) { out = -32767; }
        if (out > 32767) { out = 32767; }
    } else {
        if (out > 65535) { out = 65535; }
        if (out < 0) { out = 0; }
    }

    String newTarget = pTarget;
    newTarget = newTarget.replace("lfoout", String(out));
    if ((pEnable) && (newTarget != lastOutput)) {
        globalResponseCommands.addCommands(newTarget);
        lastOutput = newTarget;
    }
    //if (pEnable) globalResponseCommands.addCommands("Test:" + String(out) + ":" + String(pCount) + ":" + String(fOut) + ":" + String(pDelayCount));
}

float Plugin_LFO::shapeTriangle() {
    float fOut;
    if (pCount < 32767) {
        fOut = ((pCount * 2) - 32767);
    } else {
        fOut = ((32767 - pCount) * 2) + 32767;
    }
    return fOut;
}

float Plugin_LFO::shapeSine() {
    float fOut = 32767 * sin(((3.141592 * 2) / 65535) * pCount);
    return fOut;
}

float Plugin_LFO::shapeSquare() {
    if (pCount < 32767) {
        return -32767;
    } else {
        return 32767;
    }
}

void Plugin_LFO::recalculateIncrease() {
    float parentRate = 0.01;
    if (parent != nullptr) {
        parentRate = 0.001 * ((PluginHandler*) parent)->getUpdateRate();
    }

    calculatedIncrease = (65535 * parentRate) * pFrequency;

    //debugPrintln("pDelay " + String(pDelay), debugPrintType::Debug);
    if (pDelay != 0) {
//        debugPrintln("1 * (pDelay / 1000) " + String(float(1 * (((float) pDelay) / 1000))), debugPrintType::Debug);
        if ((1 * ((float) pDelay) / 1000) > (parentRate)) {
            delayIncrease = (65535 * parentRate) / (((float) pDelay) / 1000);
        } else {
            delayIncrease = 65535;
        }
    } else {
        delayIncrease = 65535;
    }

    //debugPrintln("increase " + String(calculatedIncrease) + " delay increase " + String(delayIncrease), debugPrintType::Debug);
}

#endif
