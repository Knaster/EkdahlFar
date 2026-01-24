#ifndef MODULE_H
#define MODULE_H

class Module : public ModuleCommand
{
public:
    Module();
private:
    static ModuleCommandDeclaration builtinCommands[];

    int getBuiltinCommandCount();

    const ModuleCommandDeclaration getBuiltinCommand(uint8_t i) {
        if ((i < 0) || (i > getBuiltinCommandCount())) {
            debugPrintln("Command index out of bounds", debugPrintType::Error);
        }
        return builtinCommands[i];
    }
public:
//    virtual CREATE_MODULE_COMMAND_FUNCTION_FWD(dir, Module)

    const ModuleID *moduleID;

    virtual eProcessResult processBuiltInCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false);

    virtual eProcessResult processModuleCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false);

    virtual eProcessResult processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false);

    virtual void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive);

    virtual void update();

    virtual int getModuleCommandCount();

    virtual void dumpData(std::vector<commandResponse> *dataDump);  // Returns dump data directly as string

    void setDataChangedCallback(Module *inOwner, void (*inCallback)(Module *owner, Module *module));

    bool isGroupHandler = false;
protected:
    Module *owner = nullptr;
    void (*dataChanged)(Module *owner, Module *module) = nullptr;

private:
};

#endif // MODULE_H
