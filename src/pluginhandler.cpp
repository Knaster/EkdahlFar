#ifndef PLUGINHANDLER_CPP
#define PLUGINHANDLER_CPP

#include "pluginhandler.hpp"

const ModuleCommandDeclaration PluginHandler::moduleCommands[] = {
    { "add", "a", "plugin", "Add plugin with the given name", false, false, &s_add, eCommandType::Add },
    { "remove", "rm", "plugin:index", "Remove plugin with the given name and index", false, false, &s_remove, eCommandType::Remove },
    { "count", "c", "(plugin)", "Return the number of plugins of the given type, if none given returns all", false, false, nullptr, eCommandType::Count },
};

getModuleCount(PluginHandler)

PluginHandler::PluginHandler()
{
//    moduleID = new ModuleID("pluginhandler", "ph", "Plugin handler v1.0", eModuleType::software);
    //addGroup(ModuleID("numbermap", "nm", "Number mapper v1.0", eModuleType::software, false));
    #define plugs PluginFactory::instance().plugins[i].tmoduleID
    for (int i = 0; i < PluginFactory::instance().plugins.size(); i++) {
//        addGroup(ModuleID(plugs.longName, plugs.shortName, plugs.description, plugs.moduleType, plugs.isGroupHandler));
        addGroup(plugs);
    }
}

CREATE_MODULE_COMMAND_FUNCTION(add, PluginHandler) {
    if (!addPlugin(inCommandItem->argument[0])) {
        debugPrintln("Plugin not found", debugPrintType::Debug);
        return eProcessResult::CommandFailed;
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::Debug });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(remove, PluginHandler) {
    if (request) {
        return eProcessResult::Ok;
    }
    if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
    ModuleGroup *group = getGroup(inCommandItem->argument[0]);
    if (group == nullptr) {
        debugPrintln("Group not found: " + inCommandItem->argument[0], debugPrintType::Error);
        return eProcessResult::WrongArgumentValue;
    }
    if (!group->removeModule(inCommandItem->argument[1].toInt())) {
        debugPrintln("Module not found: "  + inCommandItem->argument[1], debugPrintType::Error);
        return eProcessResult::WrongArgumentValue;
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::Debug });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(count, PluginHandler) {
    return eProcessResult::Ok;
}

void PluginHandler::update() {
    if (mainTimer > (lastUpdate + updateRate)) {
        lastUpdate = mainTimer;
        for (int i = 0; i < plugins.size(); i++) {
            plugins[i]->update();
        }
    }
}

bool PluginHandler::addPlugin(String name) {
    for (int i = 0; i< moduleGroups.size(); i++) {
//        if ((moduleGroups[i].moduleID->getLongName() == name) || (moduleGroups[i].moduleID->getShortName() == name)) {
        if ((String(moduleGroups[i].tmoduleID.longName) == name) || (String(moduleGroups[i].tmoduleID.shortName) == name)) {
            int index = moduleGroups[i].modules.size();
            std::unique_ptr<Plugin> plugin = PluginFactory::instance().create(moduleGroups[i].tmoduleID.longName);

            if (!plugin) {
                debugPrintln("Failed at creating plugin", debugPrintType::Error);
                return false;
            } else {
                Module *mod = plugin.release();

                ((Plugin*) mod)->index = index;
                ((Plugin*) mod)->registerExpression();
                ((Plugin*) mod)->setGroup(&moduleGroups[i]);
                ((Plugin*) mod)->parent = this;
                moduleGroups[i].addModule(mod);

                plugins.push_back((Plugin*) mod);
                plugins[plugins.size()-1]->update();

                debugPrintln("Plugin " + String( ((Plugin*) mod)->getModuleID().longName) + " with index " + String(index) + " created succeeded", debugPrintType::Debug);
                return true;
            }
        }
    }
    return false;
}

void PluginHandler::addInstances(String name, uint8_t count) {
    debugPrintln("Add " + String(count) + " of " + name, debugPrintType::Debug);
    ModuleGroup *group = getGroup(name);
    if (group == nullptr) {
        debugPrintln("Group not found", debugPrintType::Error);
        return;
    }
    while (group->modules.size() < count) {
        addPlugin(name);
    }
};

#endif
