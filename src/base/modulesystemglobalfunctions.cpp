#ifndef MODULESYSTEMGLOBALFUNCTIONS_CPP
#define MODULESYSTEMGLOBALFUNCTIONS_CPP

#include "base/modulesystemglobalfunctions.hpp"
#include <base/modulehandler.hpp>

static eProcessResult s_dir(ModuleCommand *self, ModuleCommandDeclarationArguments) {
    bool hidden = false;
    bool modules = false;
    bool commands = false;
    bool instances = false;
    bool instanceCount = false;
    bool recursive = false;

    if (inCommandItem->argument.size() == 0) {
        hidden = false;
        modules = true;
        commands = true;
        instances = false;
        instanceCount = true;
        recursive = true;
    } else {
        for (int i = 0; i < inCommandItem->argument.size(); i++) {
            String arg = inCommandItem->argument[i];
            if ((arg == "hidden") || (arg == "h")) { hidden = true; }
            else if ((arg == "modules") || (arg == "m")) { modules = true; }
            else if ((arg == "commands") || (arg == "c")) { commands = true; }
            else if ((arg == "instances") || (arg == "i")) { instances = true; }
            else if ((arg == "instancecount") || (arg == "ic")) { instanceCount = true; }
            else if ((arg == "recursive") || (arg == "r")) { recursive = true; }
        }
    }

    if (((Module*) self)->isGroupHandler) {
        ModuleHandler *mod = (Module*)(self);
        ((ModuleHandler*)self)->dir(inCommandResponses, String(mod->getModuleID().longName), String(mod->getModuleID().shortAlias.c_str()), hidden, modules, commands, instances, instanceCount, recursive);
    } else {
        Module *mod = (Module*)(self);
        ((Module*)self)->dir(inCommandResponses, String(mod->getModuleID().longName), String(mod->getModuleID().shortAlias.c_str()), hidden, modules, commands, instances, instanceCount, recursive);
    }

    return eProcessResult::Ok;
}

static eProcessResult s_dump(ModuleCommand *self, ModuleCommandDeclarationArguments) {
    if (((Module*) self)->isGroupHandler) {
        ((ModuleHandler*)self)->dumpData(inCommandResponses);
    } else {
        ((Module*)self)->dumpData(inCommandResponses);
    }

    return eProcessResult::Ok;
}

static eProcessResult s_help(ModuleCommand *self, ModuleCommandDeclarationArguments) {
    if (((Module*) self)->isGroupHandler) {
        ModuleHandler *mod = (Module*)(self);
        ((ModuleHandler*)self)->help(inCommandResponses, String(mod->getModuleID().longName), String(mod->getModuleID().shortAlias.c_str()));
    } else {
        Module *mod = (Module*)(self);
        ((Module*)self)->help(inCommandResponses, String(mod->getModuleID().longName), String(mod->getModuleID().shortAlias.c_str()));
    }

    return eProcessResult::Ok;
}

#endif

