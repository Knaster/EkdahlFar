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
    CREATE_MODULE_COMMAND_FUNCTION_FWD(data, Plugin_Map)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(count, Plugin_Map)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(remove, Plugin_Map)

    Plugin_Map();
/*
    eProcessResult setData(commandItem inCommandItem) override;
    eProcessResult getData() override;
*/
    double mapExpression(double index, double input);

    static double s_mapExpression(double index, double input);
    double output;
protected:

private:
    std::map<float, float> mapData;
    String pluginName;
};

REGISTERPLUGIN(Plugin_Map)

#endif // PLUGIN_MAP_HPP
