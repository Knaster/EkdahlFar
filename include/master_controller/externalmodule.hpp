#ifndef EXTERNALMODULE_HPP
#define EXTERNALMODULE_HPP

#include "arduinorequired.hpp"
#include "module.hpp"

class ExternalModule : public Module
{
public:
    tModuleID *tmoduleID;
    const tModuleID& getModuleID() const override { return *tmoduleID; }
    tModuleID& getModuleID() override { return *tmoduleID; }

    std::vector <ModuleCommandDeclaration> moduleCommands;
    const ModuleCommandDeclaration getModuleCommand(uint8_t i) const override {
        debugPrintln("Getting external module commands", debugPrintType::Debug);
        if ((i < 0) || (i > moduleCommandCount)) {
            debugPrintln("Command index out of bounds", debugPrintType::Error);
        }
        return moduleCommands[i];
    }

    ExternalModule(String deviceString);
    void update() {};

    eProcessResult processCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false);
    void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool modules, bool commands, bool instances, bool instanceCount, bool recursive);
    void dumpData(std::vector<commandResponse> *dataDump);  // Returns dump data directly as string
    void help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix);

    bool isGroupHandler = false;
private:
    uint8_t moduleCommandCount = 0;

    String kLongName;
    String kShortName;
    String kDescription;
    eModuleType kModuleType;
    bool kGroupHandler;
public:
    uint8_t extID;
    void (*transmit) (String commands, uint8_t extID, void *parent);
    void *transmissionObject;
protected:
    Module *owner = nullptr;
};

#endif // EXTERNALMODULE_HPP
