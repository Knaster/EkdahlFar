#ifndef MODULESYSTEMGLOBALFUNCTIONS_HPP
#define MODULESYSTEMGLOBALFUNCTIONS_HPP

static eProcessResult s_dir(ModuleCommand *self, ModuleCommandDeclarationArguments) {
    //hidden/h:modules/m:commands/c:instances/i:instancecounts/ic:recursive/r

//    debugPrintln("Called from " + ((Module*)(self))->moduleID->getLongName(), debugPrintType::Debug);

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
        debugPrintln("Using default parameters", debugPrintType::Debug);
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
        ((ModuleHandler*)self)->dir(inCommandResponses, ((Module*)(self))->moduleID->getLongName(), ((Module*)(self))->moduleID->getShortAlias(), hidden, modules, commands, instances, instanceCount, recursive);
    } else {
        ((Module*)self)->dir(inCommandResponses, ((Module*)(self))->moduleID->getLongName(), ((Module*)(self))->moduleID->getShortAlias(), hidden, modules, commands, instances, instanceCount, recursive);
    }

    return eProcessResult::Ok;
}

static eProcessResult s_dump(ModuleCommand *self, ModuleCommandDeclarationArguments) {
    if (((Module*) self)->isGroupHandler) {
        //*inCommandResponses = ((ModuleHandler*)self)->dumpData();
        ((ModuleHandler*)self)->dumpData(inCommandResponses);
    } else {
        //*inCommandResponses = ((Module*)self)->dumpData();
        ((Module*)self)->dumpData(inCommandResponses);
    }

    return eProcessResult::Ok;
}

#endif // MODULESYSTEMGLOBALFUNCTIONS_HPP
