#ifndef BASEMODULE_HPP
#define BASEMODULE_HPP


class BaseModule : public Module
{
public:
    MODULECOMMANDHANDLER

    BaseModule(Module *inModule);

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
//    CREATE_MODULE_COMMAND_FUNCTION_FWD(dir, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(dump, BaseModule)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(freeRAM, BaseModule)
    eProcessResult processCommands(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false) override;

    void dir(std::vector<commandResponse> *inCommandResponses, String longPrefix, String shortPrefix, bool hidden, bool inModules, bool commands, bool instances, bool instanceCount, bool recursive) override;
    void dumpData(std::vector<commandResponse> *dataDump) override;

    void reset();

    void update();

    void loadAllParameters();
protected:
    Module *mainModule = nullptr;
private:
};

#endif // BASEMODULE_HPP
