#ifndef PLUGIN_MAP_CPP
#define PLUGIN_MAP_CPP

#include "plugin_map.hpp"

const ModuleCommandDeclaration Plugin_Map::moduleCommands[] = {
    { "name", "na", "name", "Sets the map name", false, true, &s_name },
    { "data", "da", "(in:out)*", "Set the map data in any number of pairs of [in] returns [out]", false, true, &s_data },
    { "count", "c", "count*", "Returns the amount of items in the map", false, false, &s_count },
    { "remove", "da", "in", "Removes the map with [in] if it exists", false, false, &s_remove }
};

getModuleCount(Plugin_Map)

Plugin_Map::Plugin_Map() {
    moduleID = new ModuleID("numbermap", "nm", "Number mapper v1.0", eModuleType::software, false);
}

CREATE_MODULE_COMMAND_FUNCTION(name, Plugin_Map) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        pluginName = inCommandItem->argument[0];
    }
    debugPrintln(thisItem.shortCommand + ":" + pluginName, debugPrintType::Debug);
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(data, Plugin_Map) {
    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 2) || (inCommandItem->argument.size() % 2 != 0)) { return eProcessResult::WrongArgumentCount; }
        int i = 0;
        while ((i + 1) < inCommandItem->argument.size()) {
            mapData[inCommandItem->argument[i].toFloat()] = inCommandItem->argument[i + 1].toFloat();
            i += 2;
        }
    }
    String out = thisItem.shortCommand + ":" + String(index);
    for (const auto& [key, value] : mapData) {
        out += ":" + String(key) + ":" + String(value);
    }
    debugPrintln(out, debugPrintType::InfoRequest);
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(remove, Plugin_Map) {
    if (!request) {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        if (index >= group->modules.size()) {
            debugPrintln("Map not found", debugPrintType::Error);
            return eProcessResult::CommandFailed;
        }
        group->modules.erase(group->modules.begin() + index);
    }
    debugPrintln(thisItem.shortCommand + ":" + String(mapData.size()), debugPrintType::InfoRequest);
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(count, Plugin_Map) {
    debugPrintln(thisItem.shortCommand + ":" + String(mapData.size()), debugPrintType::InfoRequest);
    return eProcessResult::Ok;
}
/*
eProcessResult Plugin_Map::setData(commandItem inCommandItem) {
    return eProcessResult::Ok;
}

eProcessResult Plugin_Map::getData() {
    return eProcessResult::Ok;
}
*/
double Plugin_Map::mapExpression(double index, double input) {
    if (mapData.find(input) != mapData.end()) {
        output = mapData[input];
    } else {
        output = 0;
    }
    return output;
}

double Plugin_Map::s_mapExpression(double index, double input) {
    Plugin_Map *plugin = getIndex(int(index));
    if (plugin == nullptr) { return 0; }
    return plugin->mapExpression(index, input);
}

#endif
