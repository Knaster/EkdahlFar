#ifndef MODULEGROUP_HPP
#define MODULEGROUP_HPP

#include <memory>

#define CREATE_INDEX_CALLBACK_FWD(name, className) \
    bool name(ModuleGroup *moduleGroup); \
    static bool s_##name(Module* owner, ModuleGroup *moduleGroup) { return static_cast<className*>(owner)->name(moduleGroup); }

#define CREATE_INDEX_CALLBACK(name, className) \
    bool className::name(ModuleGroup *moduleGroup)

class ModuleGroup
{
    public:
        Module *owner = nullptr;

        ModuleGroup(ModuleID inModuleID);
//        ModuleGroup(Module *inModule);
        virtual ~ModuleGroup();

        Module* addModule(Module *inModule);
        bool removeModule(int index);

        std::vector<Module*> getSelection();
        Module* getSingleSelection();
        String getSelectionLiteral();

        bool setSelection(std::vector<int> inSelection);
        bool setSelection(int inSelection);

        ModuleID *moduleID;

        void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive);

        eProcessResult processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false);

        SELECTION selection;
        bool singleSelection = false;
        bool mustHaveSelection = false;
        // In this bizarre contraption *self and *moduleGroup are both supposed to be 'this' but they are later recast as the Module and ModuleGroup portion of the object

        void setIndexCallback(Module *inOwner, void (*inCallback)(Module *owner, ModuleGroup *moduleGroup));
        //void setDataChangedCallback(Module *inOwner, void (*inCallback)(Module *owner, ModuleGroup *moduleGroup));
        //std::vector<commandResponse> dumpData();
        void dumpData(std::vector<commandResponse> *dataDump);

        std::vector<Module*> modules;

        bool indexing(commandItem *inCommandItem);
    protected:
    private:
        bool (*indexCallback)(Module *owner, ModuleGroup *moduleGroup) = nullptr;
        //void (*dataChanged)(Module *owner, Module *module) = nullptr;
};

#endif // MODULEGROUP_HPP
