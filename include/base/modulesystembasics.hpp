#ifndef MODULESYSTEMBASICS_HPP
#define MODULESYSTEMBASICS_HPP

#include "base/arduinorequired.hpp"
#include "base/commandparser.hpp"

enum eProcessResult { NotFound =  0, Ok = 1, CommandFailed = 2, WrongArgumentCount = 3, WrongArgumentMinimum = 4, WrongArgumentValue = 5, PassThrough = 6 };

#define ModuleCommandDeclarationArguments commandItem *inCommandItem, std::vector<commandResponse> *inCommandResponses, bool request, ModuleCommandDeclaration thisItem

struct ModuleCommandDeclaration;

// This class is never used alone but part of the inheritance for Module and thus ModuleHandler
class ModuleCommand {
public:
    virtual const ModuleCommandDeclaration getModuleCommand(uint8_t i) const = 0;
};

enum eCommandType {
    SimpleString = 0,
    SimpleBool = 1,
    SimpleFloat = 2,
    SimpleUInt8 = 3,
    SimpleInt16 = 4,
    SimpleUInt16 = 5,
    Commands = 6,
    Data = 7,
    Conditional = 8,
    ReturnOnly = 9,
    Immediate = 10,
    Expression = 11,
    Add = 12,
    Remove = 13,
    Count = 14,
    Name = 15,
    Index = 16,
    Milliseconds = 17,
    Microseconds = 18,
    Hertz = 19,
    RequestOnly = 20,
    ReturnRequest = 21,
    OutputAssignment = 22
};

extern String commandTypeDescription[];

struct ModuleCommandDeclaration {
    const String longCommand;
    const String shortCommand;
    const String arguments;
    const String Help;
    const bool hidden;
    const bool save;
    eProcessResult (*commandFunction) (ModuleCommand*, ModuleCommandDeclarationArguments);
    const eCommandType cType;
};

enum eModuleType {
    software = 0,
    hardware = 1,
    remote = 2
};

#include <string>

struct tModuleID {
    const char* longName;
    const char* shortName;
    const char* description;
    const eModuleType moduleType = 0;
    bool isGroupHandler = false;
    std::string shortAlias;
};

extern const String moduleTypeDescription[];

String getModuleType(uint8_t mType);

#include "module.hpp"
/*
static eProcessResult s_dir(ModuleCommand* self, ModuleCommandDeclarationArguments);
static eProcessResult s_dump(ModuleCommand* self, ModuleCommandDeclarationArguments);
static eProcessResult s_help(ModuleCommand* self, ModuleCommandDeclarationArguments);
*/
#define staticCopy(name, className) \
    static eProcessResult s_##name(ModuleCommand* self, ModuleCommandDeclarationArguments) { return static_cast<className*>(self)->name(inCommandItem, inCommandResponses, request, thisItem); }

#define CREATE_MODULE_COMMAND_FUNCTION_FWD(funcname, className) eProcessResult funcname(ModuleCommandDeclarationArguments); staticCopy(funcname, className)

#define CREATE_MODULE_COMMAND_FUNCTION(funcname, className) eProcessResult className::funcname(ModuleCommandDeclarationArguments)

#define getModuleCount(name) int name::getModuleCommandCount() { return sizeof(moduleCommands) / sizeof(moduleCommands[0]); }

#define MODULECOMMANDHANDLER \
    static const ModuleCommandDeclaration moduleCommands[]; \
    int getModuleCommandCount(); \
    const ModuleCommandDeclaration getModuleCommand(uint8_t i) const override { \
        if ((i < 0) || (i > getModuleCommandCount())) { \
            debugPrintln("Command index out of bounds", debugPrintType::Error); \
        } \
        return moduleCommands[i]; \
    }

#define CREATE_DATACHANGED_CALLBACK_FWD(name, className) \
    bool name(Module *module); \
    static bool s_##name(Module *owner, Module *module) { return static_cast<className*>(owner)->name(module); }

#define CREATE_DATACHANGED_CALLBACK(name, className) \
    bool className::name(Module *module)

#define SETMODULEID(longName, shortName, description, moduleType, groupHandler) \
    static constexpr const char* kLongName = longName; \
    static constexpr const char* kShortName = shortName; \
    static constexpr const char* kDescription = description; \
    static constexpr eModuleType kModuleType = moduleType; \
    tModuleID tmoduleID = { kLongName, kShortName, kDescription, kModuleType, groupHandler, kShortName }; \
    const tModuleID& getModuleID() const override { return tmoduleID; } \
    tModuleID& getModuleID() override { return tmoduleID; } \
    static tModuleID getModuleIDStatic() { return { kLongName, kShortName, kDescription, kModuleType, groupHandler, kShortName }; }

#endif
