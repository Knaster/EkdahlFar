#ifndef PLUGIN_MAP_HPP
#define PLUGIN_MAP_HPP

#include "module.hpp"
#include <map>

class Plugin_Map : public Plugin
{
public:
    SETMODULEID("numbermap", "nm", "Number mapper v1.0", eModuleType::software, false)

    CREATEPLUGINFUNCTIONS(Plugin_Map)
    REGISTEREXPRESSION(s_mapExpression)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(name, Plugin_Map)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(target, Plugin_Map)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(data, Plugin_Map)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(remove, Plugin_Map)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(scale, Plugin_Map)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(trigger, Plugin_Map)

    Plugin_Map();

    double mapExpression(uint8_t input);
    static double s_mapExpression(double index, double input);

    bool updateManual(int16_t index = -1);
    void update() override {};
protected:

private:
    String pName;
    String pTarget;
    String pLastOutput;
    uint16_t pScale = 65535;
    int16_t pLastIndex = -1;

    std::map<uint8_t, float> mapData;
};

REGISTERPLUGIN(Plugin_Map)

#endif // PLUGIN_MAP_HPP
