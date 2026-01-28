#ifndef PLUGINHANDLER_HPP
#define PLUGINHANDLER_HPP

#include "modulehandler.hpp"

#include <functional>
#include <string>

class Plugin : public Module {
public:
    virtual const tModuleID& getModuleID() const = 0;
    virtual tModuleID& getModuleID() = 0;
    virtual void registerExpression() {};
    uint8_t index = 0;
    virtual ModuleGroup* getGroup() const = 0;
    virtual void setGroup(ModuleGroup *inGroup) const = 0;
    ModuleHandler *parent = nullptr;

    virtual ~Plugin() = default;
//    virtual eProcessResult setData(commandItem inCommandItem);
//    virtual eProcessResult getData();
    virtual void update() {
        debugPrintln("Standard plugin response", debugPrintType::Debug);
    };
};

class PluginFactory {
public:
    using creator_t = std::function<std::unique_ptr<Plugin>()>;

    typedef struct PluginRegistry {
        creator_t creator;
        tModuleID tmoduleID;
    };

    static PluginFactory& instance() {
        static PluginFactory staticPluginFactory;
        return staticPluginFactory;
    }

    std::vector<PluginRegistry> plugins;

    void registerPlugin(creator_t creator, tModuleID tmoduleID) {
        plugins.push_back({ std::move(creator), tmoduleID });
    }

//    std::unique_ptr<Plugin> create(const std::string& longName) const {
    std::unique_ptr<Plugin> create(String longName) const {
//        debugPrintln("Number of plugins registered: " + String(plugins.size()), debugPrintType::Debug);
        for (int i = 0; i < plugins.size(); i++) {
            if (String(plugins[i].tmoduleID.longName) == longName) {
//                debugPrintln("Got the plugin", debugPrintType::Debug);
                return plugins[i].creator();
            }
        }
        debugPrintln("Plugin not found", debugPrintType::Error);
        return nullptr;
    }

private:
};
#include <cstring>

#define CREATEPLUGINFUNCTIONS(class) \
    static std::unique_ptr<Plugin> create() { \
        return std::make_unique<class>(); \
    } \
    \
    static ModuleGroup *group; \
    ModuleGroup* getGroup() const override { return group; }; \
    virtual void setGroup(ModuleGroup *inGroup) const override { group = inGroup; }; \
    \
    static class* getIndex(uint8_t index) { \
        if (index >= group->modules.size()) { \
            debugPrintln("Map not found", debugPrintType::Error); \
            return nullptr; \
        } \
        return ((class*) group->modules[index]); \
    }\
    \
    private:\
    static bool registered;\
    public:

#define REGISTEREXPRESSION(callback) \
    void registerExpression() override { \
        expressionParser.registerFunction(tmoduleID.shortName, (const void*) &callback, TE_FUNCTION2); \
    } \

#define REGISTERPLUGIN(class) \
    bool class::registered = [] { \
        PluginFactory::instance().registerPlugin(&class::create, class::getModuleIDStatic()); \
        return true; \
    } (); \
    \
    ModuleGroup* class::group = nullptr;

class PluginHandler : public ModuleHandler
{
public:
    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(add, PluginHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(remove, PluginHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(count, PluginHandler)

    PluginHandler();

    void update() override;
    bool addPlugin(String name);
    void addInstances(String name, uint8_t count) override;
//    void dumpData(std::vector<commandResponse> inCommandResponse) override;

    std::vector<Plugin*> plugins;
protected:

private:
    uint16_t updateRate = 10;
    uint32_t lastUpdate = 0;
    elapsedMillis mainTimer;
public:
    uint16_t getUpdateRate() { return updateRate; }
};


float convertMsToRate(float parentRate, float inRate) {
    float outRate = 0;

    if (inRate != 0) {
        if ((1 * ((float) inRate) / 1000) > (parentRate)) {
            outRate = (65535 * parentRate) / (((float) inRate) / 1000);
        } else {
            outRate = 65535;
        }
    } else {
        outRate = 65535;
    }
    return outRate;
}

#include "../src/plugin_lfo.cpp"
#include "../src/plugin_map.cpp"
#include "../src/plugin_adsr.cpp"

#endif // PLUGINS_HPP
