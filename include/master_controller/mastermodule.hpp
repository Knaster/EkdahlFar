#ifndef MASTERMODULE_HPP
#define MASTERMODULE_HPP

#include "base/modulehandler.hpp"
#include "base/basemodule.hpp"
#include "plugins/pluginhandler.hpp"
#include "master_controller/global_generics.hpp"

#ifndef NO_EXTERNAL_MODULES
#include "master_controller/externalmodulehandler.hpp"
#endif

class MasterModule : public BaseModule
{
public:
    static const ModuleCommandDeclaration masterModuleCommands[];

    int getMasterModuleCommandCount() const;

    const ModuleCommandDeclaration getMasterModuleCommand(uint8_t i) const { \
        if ((i < 0) || (i > getMasterModuleCommandCount())) { \
            debugPrintln("Command index out of bounds", debugPrintType::Error); \
        } \
        return masterModuleCommands[i]; \
    }

    eProcessResult processMasterModuleCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request);

    MasterModule(ModuleHandler *inModule, ExpressionParser *inExpressionParser);

    CREATE_MODULE_COMMAND_FUNCTION_FWD(userVariables, MasterModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(expressionParserEvaluate, MasterModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ifEqual, MasterModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ifGreater, MasterModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ifLess, MasterModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(freeRAM, MasterModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(external, MasterModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(test, MasterModule)

    eProcessResult processCommands(CommandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false) override;

    void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool inModules, bool commands, bool instances, bool instanceCount, bool recursive) override;
    void dumpData(std::vector<commandResponse> *dataDump) override;
    void help(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix) override;

    void update();

    void loadAllParameters() override;
    String saveAllParameters() override;
    void resetAllParameters() override;
    void reset() override;

    void init();
protected:
    PluginHandler *pluginHandler = nullptr;
    ExpressionParser *expressionParser = nullptr;
#ifndef NO_EXTERNAL_MODULES
    ExternalModuleHandler *externalModuleHandler = nullptr;
#endif
    String pNickName = "";
};

#endif // MASTERMODULE_HPP
