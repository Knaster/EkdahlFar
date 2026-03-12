#ifndef PLUGINHANDLER_HPP
#define PLUGINHANDLER_HPP

#include "base/modulehandler.hpp"
#include "master_controller/expressionparser.h"

#include <commandlist.hpp>
extern CommandList globalCommands;
extern CommandList globalResponseCommands;

#include <functional>
#include <string>

class Plugin : public Module {
public:
    virtual void registerExpression(ExpressionParser *expressionParser) {};
    uint8_t index = 0;
    virtual ModuleGroup* getGroup() const = 0;
    virtual void setGroup(ModuleGroup *inGroup) const = 0;
    ModuleHandler *parent = nullptr;

    virtual ~Plugin() = default;
    virtual void update();
};

class PluginFactory {
public:
    using creator_t = std::function<std::unique_ptr<Plugin>()>;

    //typedef
    struct PluginRegistry {
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
    void registerExpression(ExpressionParser *expressionParser) override { \
        if (expressionParser == nullptr) { \
            debugPrintln("Expression Parser is NULL", debugPrintType::Error); \
            return; \
        } \
        expressionParser->registerFunction(tmoduleID.shortName, (const void*) &callback, TE_FUNCTION2); \
    } \

#define REGISTERPLUGIN(class) \
    bool class::registered = [] { \
        PluginFactory::instance().registerPlugin(&class::create, class::getModuleIDStatic()); \
        return true; \
    } (); \
    \
    ModuleGroup* class::group = nullptr;

#define CREATEGETSET_HEADER(name, class) \
    CREATE_MODULE_COMMAND_FUNCTION(name, class) { \
        if (!request) { \
            if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; } \

#define CREATEGETSET_FOOTER(variable) \
        } \
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(variable), debugPrintType::InfoRequest }); \
        return eProcessResult::Ok; \
    }

#define CREATEGETSET_FOOTERQ(variable) \
        } \
        inCommandResponses->push_back({ thisItem.shortCommand + ":" + delimitExpression(String(variable), true), debugPrintType::InfoRequest }); \
        return eProcessResult::Ok; \
    }

#define CREATE_GETSET_FUNCTION_CONVERT(name, class, variable, conversion) \
    CREATEGETSET_HEADER(name, class) \
    variable = inCommandItem->argument[0].conversion(); \
    CREATEGETSET_FOOTER(variable)

#define CREATE_GETSET_FUNCTION(name, class, variable) \
    CREATEGETSET_HEADER(name, class) \
    variable = stripQuotes(inCommandItem->argument[0]); \
    CREATEGETSET_FOOTERQ(variable)

#define CREATE_GETSET_FUNCTION_F_CONVERT(name, class, variable, conversion, function) \
    CREATEGETSET_HEADER(name, class) \
    variable = inCommandItem->argument[0].conversion(); \
    function(); \
    CREATEGETSET_FOOTER(variable)

#define CREATE_GETSET_FUNCTION_F(name, class, variable, function) \
    CREATEGETSET_HEADER(name, class) \
    variable = stripQuotes(inCommandItem->argument[0]); \
    function(); \
    CREATEGETSET_FOOTERQ(variable)

class PluginHandler : public ModuleHandler
{
public:
    SETMODULEID("pluginhandler", "ph", "Plugin handler v1.0", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(add, PluginHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(remove, PluginHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(count, PluginHandler)

    PluginHandler(ExpressionParser *inExpressionParser);

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
    ExpressionParser *expressionParser = nullptr;
public:
    uint16_t getUpdateRate() { return updateRate; }
};

float convertMsToRate(float parentRate, float inRate);

#endif // PLUGINS_HPP
