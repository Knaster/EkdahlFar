#ifndef MODULEHANDLER_HPP
#define MODULEHANDLER_HPP

#include "modulegroup.hpp"

class ModuleHandler : public Module
{
    public:
        ModuleHandler();

//        CREATE_MODULE_COMMAND_FUNCTION_FWD(groupdir, ModuleHandler);

        eProcessResult processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false) override;
        eProcessResult processModuleHandlerCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponse, bool request = false);

        void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive) override;

        std::vector<ModuleGroup> moduleGroups;

        ModuleGroup* getGroup(String longName);

        void dumpData(std::vector<commandResponse> *dataDump);

        ModuleGroup* addModule(Module *module);
        ModuleGroup* addGroup(ModuleID inModuleID);
        virtual void addInstances(String name, uint8_t count) {};
    protected:
    private:
};

#endif // MODULEHANDLER_HPP
