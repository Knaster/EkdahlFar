#ifndef PLUGIN_MULT_HPP
#define PLUGIN_MULT_HPP

#include "plugins/pluginhandler.hpp"

class Plugin_Mult_Connector {
public:
    String id;
    double value;
};

class Plugin_Mult : public Plugin
{
public:
    SETMODULEID("multiple", "mlt", "Multiple input connector v1.0", eModuleType::software, false)

    CREATEPLUGINFUNCTIONS(Plugin_Mult)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(name, Plugin_Mult)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(target, Plugin_Mult)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(data, Plugin_Mult)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(remove, Plugin_Mult)

    Plugin_Mult();

    void updateManual();
    void update() override {};
private:
    enum eMultiType {
        pAdder = 0,
        pRatio = 1
    };

    String sMultiTypeName[2] = { "adder", "ratio" };

    String pName;
    String pTarget;
    //String lastOutput;
    int32_t pLastOutput;
    std::vector<Plugin_Mult_Connector> addConnectors;
    std::vector<Plugin_Mult_Connector> ratioConnectors;

    Plugin_Mult_Connector* getConnector(String inId, std::vector<Plugin_Mult_Connector> *connectors);
    Plugin_Mult_Connector* connect(String id, float value, bool request, std::vector<Plugin_Mult_Connector> *connectors);

    eProcessResult commandConnect(commandItem *inCommandItem, std::vector<commandResponse> *inCommandResponses, ModuleCommandDeclaration *thisItem, bool request,
                                  std::vector<Plugin_Mult_Connector> *connectors);

    eProcessResult commandDisconnect(commandItem *inCommandItem, std::vector<commandResponse> *inCommandResponses, ModuleCommandDeclaration *thisItem, bool request,
                                           std::vector<Plugin_Mult_Connector> *connectors);
};

REGISTERPLUGIN(Plugin_Mult)

#endif // PLUGIN_MULT_HPP
