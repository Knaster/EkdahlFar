#ifndef PLUGIN_MAP_CPP
#define PLUGIN_MAP_CPP

#include "plugins/plugin_map.hpp"

const ModuleCommandDeclaration Plugin_Map::moduleCommands[] = {
    { "name", "na", "name", "Sets the map name", false, true, &s_name, eCommandType_data::ectSimpleString | eCommandType_function::ectName },
    { "target", "tg", "commandlist*", "Sets the command string to execute whenever a connection value changes", false, true, &s_target,
        eCommandType_data::ectOutputAssignment  | eCommandType_dataOptions::ectExpression | eCommandType_function::ectAssignment, "nmout" },
    { "data", "da", "(in:out)*", "Set the map data in any number of pairs of [in] returns [out]", false, true, &s_data,
        eCommandType_data::ectData | eCommandType_function::ectParameter },
    { "remove", "rm", "in", "Removes the map with [in] if it exists", false, false, &s_remove, eCommandType_data::ectSimpleUInt8 | eCommandType_function::ectRemove },
    { "scale", "sc", "in", "Sets the overall scale of the number map", false, true, &s_scale, eCommandType_data::ectSimpleUInt16 | eCommandType_function::ectParameter },
    { "trigger", "tr", "number", "Triggers the commands set in the target with the given output", false, false, &s_trigger,
        eCommandType_data::ectSimpleInt16 | eCommandType_function::ectParameter }
};

getModuleCount(Plugin_Map)

Plugin_Map::Plugin_Map() {}

CREATE_GETSET_FUNCTION(name, Plugin_Map, pName)

CREATE_GETSET_FUNCTION_F(target, Plugin_Map, pTarget, updateManual)

CREATE_MODULE_COMMAND_FUNCTION(data, Plugin_Map) {
    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 2) || (inCommandItem->argument.size() % 2 != 0)) { return eProcessResult::WrongArgumentCount; }
        int i = 0;
        bool update = false;
        while ((i + 1) < inCommandItem->argument.size()) {
            mapData[inCommandItem->argument[i].toInt()] = inCommandItem->argument[i + 1].toFloat();

            if ((inCommandItem->argument[i].toInt() == pLastIndex)) { update = true; }

            i += 2;
        }
        if (update) {
            debugPrintln("same value, updating", debugPrintType::Debug);
            updateManual();
        }
    }
    String out = thisItem.shortCommand;
    for (const auto& [key, value] : mapData) {
        out += ":" + String(key) + ":" + String(value);
    }
    inCommandResponses->push_back({ out, debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(scale, Plugin_Map) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pScale = inCommandItem->argument[0].toInt();
        updateManual();
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(pScale), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(remove, Plugin_Map) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        uint8_t number = inCommandItem->argument[0].toInt();
        if (mapData.find(number) == mapData.end()) { return eProcessResult::WrongArgumentValue; }
        mapData.erase(number);
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(mapData.size()), debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(trigger, Plugin_Map) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    if (!updateManual(inCommandItem->argument[0].toInt())) { return eProcessResult::WrongArgumentValue; }
    return eProcessResult::Ok;
}

bool Plugin_Map::updateManual(int16_t index) {
    if (index == -1) {
        if (pLastIndex == -1) { return false; }
        index = pLastIndex;
    }
    pLastIndex = index;
    if (mapData.find(index) == mapData.end()) { return false; }

    String newTarget = pTarget;
    newTarget = newTarget.replace("nmout", String(mapData[index]  / ((double) 65535 / ((double) pScale))));
    if ((newTarget != pLastOutput)) {
        debugPrintln(newTarget, debugPrintType::Debug);
        globalResponseCommands.addCommands(newTarget);
        pLastOutput = newTarget;
    }
    return true;
}

double Plugin_Map::mapExpression(uint8_t input) {
    double output;
    if (mapData.find(input) != mapData.end()) {
        output = mapData[input] / ((double) 65535 / ((double) pScale));
        pLastIndex = input;
    } else {
        output = 0;
    }
    return output;
}

double Plugin_Map::s_mapExpression(double index, double input) {
    Plugin_Map *plugin = getIndex(index);
    if (plugin == nullptr) { return 0; }
    return plugin->mapExpression(input);
}

#endif
