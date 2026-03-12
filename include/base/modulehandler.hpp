#ifndef MODULEHANDLER_HPP
#define MODULEHANDLER_HPP

#include "modulegroup.hpp"

class ModuleHandler : public Module
{
    public:
        ModuleHandler();

        eProcessResult processCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false) override;
        eProcessResult processModuleHandlerCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponse, bool request = false);

        void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive) override;

        void help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix);

        std::vector<ModuleGroup> moduleGroups;

        ModuleGroup* getGroup(String longName);

        void dumpData(std::vector<commandResponse> *dataDump);

        ModuleGroup* addModule(Module *module);
//        ModuleGroup* addGroup(ModuleID inModuleID);
        ModuleGroup* addGroup(tModuleID inModuleID);
        virtual void addInstances(String name, uint8_t count) {};

        uint8_t getOwnIndex(Module inModule);
    protected:
    private:
};

#endif // MODULEHANDLER_HPP
