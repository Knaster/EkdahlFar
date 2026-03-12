#ifndef BASEMODULE_HPP
#define BASEMODULE_HPP

//#include "base/modulehandler.hpp"
#include "base/module.hpp"
#include "master_controller/automaticversion.hpp"

class BaseModule : public Module
{
public:
    tModuleID *tmoduleID = nullptr;
    tModuleID& getModuleID() override { return *tmoduleID; }
    const tModuleID& getModuleID() const override { return *tmoduleID; }
    tModuleID defaultModuleID = { "", "", "", eModuleType::software, false };

    MODULECOMMANDHANDLER

    BaseModule(Module *inModule);

    CREATE_MODULE_COMMAND_FUNCTION_FWD(requestInfo, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(debugPrint, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(version, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(saveAllParameters, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(loadAllParameters, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(resetAllParameters, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(reset, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(nick, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(noOperation, BaseModule)
    //CREATE_MODULE_COMMAND_FUNCTION_FWD(dump, BaseModule)

    eProcessResult processCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false) override;
    eProcessResult processSelf(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request, eProcessResult processResult);

    void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool inModules, bool commands, bool instances, bool instanceCount, bool recursive) override;
    void dumpData(std::vector<commandResponse> *dataDump) override;
    void help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix) override;

    void update();

    bool convertRequestInfo(CommandItem *inCommandItem);
    virtual void loadAllParameters() {};
    virtual String saveAllParameters() { return ""; };
    virtual void resetAllParameters() {};
    virtual const char* version();
    virtual void reset();
    virtual void bootLoader();
    virtual String* nick(bool request);

    String currentFirmwareVersion;
    bool firmwareChanged = false;
protected:
    Module *mainModule = nullptr;
    String pNickName = "";
private:
};

#endif // BASEMODULE_HPP
