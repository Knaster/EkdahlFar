#ifndef MODULE_H
#define MODULE_H

#include <base/arduinorequired.hpp>
#include <base/modulesystembasics.hpp>

class Module : public ModuleCommand
{
public:
    Module();

    virtual const tModuleID& getModuleID() const = 0;
    virtual tModuleID& getModuleID() = 0;

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
//    const ModuleID *moduleID;

    virtual eProcessResult processBuiltInCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false);

    virtual eProcessResult processModuleCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false);

    virtual eProcessResult processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false);

    virtual void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive);

    virtual void update();

    // MUST be implemented
    virtual int getModuleCommandCount();

    virtual void dumpData(std::vector<commandResponse> *dataDump);  // Returns dump data directly as string
    virtual void help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix);
    bool isGroupHandler = false;
private:
    bool pDataChange = false;
public:
    bool hasDataChange();
protected:
    Module *owner = nullptr;

    void setDataChanged() { pDataChange = true; }
};

#endif // MODULE_H
