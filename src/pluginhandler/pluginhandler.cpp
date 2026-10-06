#ifndef PLUGINHANDLER_CPP
#define PLUGINHANDLER_CPP

#include "plugins/pluginhandler.hpp"

const ModuleCommandDeclaration PluginHandler::moduleCommands[] = {
    { "add", "a", "plugin", "Add plugin with the given name", false, false, &s_add, eCommandType_data::ectSimpleString | eCommandType_function::ectAddModule },
    { "remove", "rm", "plugin:index", "Remove plugin with the given name and index", false, false, &s_remove, eCommandType_function::ectRemoveModule },
//    { "count", "c", "(plugin)", "Return the number of plugins of the given type, if none given returns all", false, false, nullptr,
//        eCommandType_function::ectCountModules | eCommandType_access::ectRequest },
};

getModuleCount(PluginHandler)

PluginHandler::PluginHandler(ExpressionParser *inExpressionParser)
{
    expressionParser = inExpressionParser;
    #define plugs PluginFactory::instance().plugins[i].tmoduleID
    for (int i = 0; i < PluginFactory::instance().plugins.size(); i++) {
        addGroup(plugs);
    }
}

CREATE_MODULE_COMMAND_FUNCTION(add, PluginHandler) {
    for (int i = 0; i< moduleGroups.size(); i++) {
        if ((String(moduleGroups[i].tmoduleID.longName) == inCommandItem->argument[0]) || (String(moduleGroups[i].tmoduleID.shortName) == inCommandItem->argument[0])) {
            int index = moduleGroups[i].modules.size();
            std::unique_ptr<Plugin> plugin = PluginFactory::instance().create(moduleGroups[i].tmoduleID.longName);

            if (!plugin) {
                //debugPrintln("Failed at creating plugin", debugPrintType::Error);
                inCommandResponses->push_back({ thisItem.shortCommand + " - Plugin addition failed; plugin could not be created" , debugPrintType::Error });
                return eProcessResult::Ok;
            } else {
                Module *mod = plugin.release();

                ((Plugin*) mod)->index = index;
                ((Plugin*) mod)->setGroup(&moduleGroups[i]);
                ((Plugin*) mod)->parent = this;
                ((Plugin*) mod)->registerExpression(expressionParser);
                moduleGroups[i].addModule(mod);

                plugins.push_back((Plugin*) mod);
                plugins[plugins.size()-1]->update();

                //debugPrintln("Plugin " + String( ((Plugin*) mod)->getModuleID().longName) + " with index " + String(index) + " created succeeded", debugPrintType::Debug);
                inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(((Plugin*) mod)->getModuleID().longName) + ":" + String(index), debugPrintType::InfoRequest });
                return eProcessResult::Ok;
            }
        }
    }
    inCommandResponses->push_back({ thisItem.shortCommand + " - Plugin addition failed, plugin '" + inCommandItem->argument[0] + "' not found" , debugPrintType::Error });
    return eProcessResult::Ok;

/*
    if (!addPlugin(inCommandItem->argument[0])) {
        debugPrintln("Plugin not found", debugPrintType::Debug);
        return eProcessResult::CommandFailed;
    }
    //inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest });
    return eProcessResult::Ok;*/
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
    inCommandResponses->push_back({ thisItem.shortCommand + ":" + inCommandItem->argument[0] + ":" + inCommandItem->argument[1], debugPrintType::InfoRequest });
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
        if ((String(moduleGroups[i].tmoduleID.longName) == name) || (String(moduleGroups[i].tmoduleID.shortName) == name)) {
            int index = moduleGroups[i].modules.size();
            std::unique_ptr<Plugin> plugin = PluginFactory::instance().create(moduleGroups[i].tmoduleID.longName);

            if (!plugin) {
                debugPrintln("Failed at creating plugin", debugPrintType::Error);
                return false;
            } else {
                Module *mod = plugin.release();

                ((Plugin*) mod)->index = index;
                ((Plugin*) mod)->setGroup(&moduleGroups[i]);
                ((Plugin*) mod)->parent = this;
                ((Plugin*) mod)->registerExpression(expressionParser);
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
    ModuleGroup *group = getGroup(name);
    if (group == nullptr) {
        debugPrintln("Group not found", debugPrintType::Error);
        return;
    }
    while (group->modules.size() < count) {
        addPlugin(name);
    }
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

void PluginHandler::help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix) {
    Module::help(inCommandResponses, longPrefix, shortPrefix);

    String lp = longPrefix, sp = shortPrefix;
    if (lp != "") { if (lp[lp.length() - 1] != '.') { lp +=  "."; } }
    if (sp != "") { if (sp[sp.length() - 1] != '.') { sp +=  "."; } }

    String response;
/*    if (strcmp(getModuleID().longName, "")) {
        inCommandResponses->push_back({ "help:" + lp.substring(0, lp.length() - 1) + ":" + sp.substring(0, sp.length() - 1) + ":" +
                                      delimitExpression(String(getModuleID().description), true), debugPrintType::InfoRequest });
    }
*/
    #define mcd PluginFactory::instance().plugins[i].moduleCommands[j]
    for (int i = 0; i < PluginFactory::instance().plugins.size(); i++) {
        String pluginName = PluginFactory::instance().plugins[i].tmoduleID.longName;

        inCommandResponses->push_back({ "help:" + lp + pluginName + ":" + sp + PluginFactory::instance().plugins[i].tmoduleID.shortName + ":" +
                                      delimitExpression(PluginFactory::instance().plugins[i].tmoduleID.description, true), debugPrintType::InfoRequest });

        for (int j = 0; j < PluginFactory::instance().plugins[i].moduleCommandCount; j++) {
            response = "help:" + lp + pluginName + "." + mcd.longCommand + ":" + sp + mcd.shortCommand + ":" + String(mcd.cType) + ":" + delimitExpression(mcd.arguments, true) + ":" + mcd.hidden + ":" +
                delimitExpression(mcd.Help, true) + ":" + delimitExpression(mcd.variables, true);
            inCommandResponses->push_back({ response, debugPrintType::InfoRequest });
        }
    }
}
#endif
