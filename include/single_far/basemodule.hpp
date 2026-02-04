#ifndef BASEMODULE_HPP
#define BASEMODULE_HPP

#include "base/modulehandler.hpp"
#include "plugins/pluginhandler.hpp"

class BaseModule : public Module
{
public:
    //SETMODULEID("", "", "", eModuleType::software, false)
    tModuleID *tmoduleID = nullptr;
    tModuleID& getModuleID() override { return *tmoduleID; }
    const tModuleID& getModuleID() const override { return *tmoduleID; }
    tModuleID defaultModuleID = { "", "", "", eModuleType::software, false };

    MODULECOMMANDHANDLER

    BaseModule(Module *inModule, ExpressionParser *inExpressionParser);

    CREATE_MODULE_COMMAND_FUNCTION_FWD(requestInfo, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(debugPrint, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(version, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(saveAllParameters, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(loadAllParameters, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(resetAllParameters, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(userVariables, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(expressionParserEvaluate, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ifEqual, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ifGreater, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ifLess, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(reset, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(nick, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(noOperation, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(dump, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(freeRAM, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(test, BaseModule)

    eProcessResult processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false) override;

    void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool inModules, bool commands, bool instances, bool instanceCount, bool recursive) override;
    void dumpData(std::vector<commandResponse> *dataDump) override;
    void help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix) override;

    void reset();

    void update();

    void loadAllParameters();

    String currentFirmwareVersion;
    bool firmwareChanged = false;
    void checkVersion();
protected:
    ModuleHandler *mainModule = nullptr;
    PluginHandler *pluginHandler = nullptr;
    ExpressionParser *expressionParser = nullptr;
    String pNickName = "";
private:
};

#endif // BASEMODULE_HPP
