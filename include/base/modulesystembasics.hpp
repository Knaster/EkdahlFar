#ifndef MODULESYSTEMBASICS_HPP
#define MODULESYSTEMBASICS_HPP

#include "base/arduinorequired.hpp"
#include "base/commandparser.hpp"

enum eProcessResult { NotFound =  0, Ok = 1, CommandFailed = 2, WrongArgumentCount = 3, WrongArgumentMinimum = 4, WrongArgumentValue = 5, PassThrough = 6 };

#define ModuleCommandDeclarationArguments CommandItem *inCommandItem, std::vector<commandResponse> *inCommandResponses, bool request, ModuleCommandDeclaration thisItem

struct ModuleCommandDeclaration;

// This class is never used alone but part of the inheritance for Module and thus ModuleHandler
class ModuleCommand {
public:
    virtual const ModuleCommandDeclaration getModuleCommand(uint8_t i) const = 0;
};

// Using 4 bits 0bxxxxxxxxxxxx0000
enum eCommandType_data {
    ectNone = 0,
    ectSimpleString = 1,
    ectSimpleBool = 2,
    ectSimpleFloat = 3,
    ectSimpleUInt8 = 4,
    ectSimpleInt16 = 5,
    ectSimpleUInt16 = 6,
    ectCommands = 7,
    ectData = 8,
    ectMilliseconds = 9,
    ectMicroseconds = 10,
    ectHertz = 11,
    ectOutputAssignment = 12,
    ectSimpleInt8 = 13,
    ectConditional = 14         // Makes something happen give that the first parameter is '1'
};

// Using 1 bit (+ 0bxxxxxxxxxxx0xxxx)
#define eCT_do_sh 4
enum eCommandType_dataOptions {
    ectStatic = 0 << eCT_do_sh,         // Indicates that the argument(s) are going to be interpreted literally
    ectExpression = 1 << eCT_do_sh,     // Indicates that the argument(s) are of expression type and accepts equations and variables
};

// Using 2 bits (+ 0bxxxxxxxx000xxxxx)
// If ANY of these bits are set, the implication is that you CANNOT set this parameter
#define eCT_ac (eCT_do_sh + 1)
enum eCommandType_access {
    ectSetRequest = 0 << eCT_ac,       // Indicates that the command is of get/set type
    ectRequest = 1 << eCT_ac,          // Indicates that the command only returns data and cannot be set
    ectSelfInvoked = 2 << eCT_ac,      // Indicates that the command is self-invoked and will be issued internally during certain conditions
    ectInvokeOnly = 3 << eCT_ac        // Indicates that the command can only be set, a data request cannot be made on the function
};


// Actions are stateless, momentary events that are generally not readable. Things like triggering the hammer, adding a new module etc.
// Parameters, settings & calibrations when set will keep that state until changed, things like "motor run", "full mute" etc.
// Using 4 bits (+ 0bxxxx0000xxxxxxxx
#define eCT_fu (eCT_ac + 2)
enum eCommandType_function {
    ectUndefined = (0 << eCT_fu),
    //ectParameter = (1 << eCT_fu),           // Indicates a parameter setting
    ectCalibration = (1 << eCT_fu),         // Indicates a setting that is a calibration setting meant for setting up once
    ectAddModule = (2 << eCT_fu),           // Indicates that the function adds a module
    ectRemoveModule = (3 << eCT_fu),        // Indicates that the function removes a module
    ectCountModules = (4 << eCT_fu),        // Indicates that the function returns the number of child modules
    ectName = (5 << eCT_fu),                // Sets the name of a module
    ectIndex = (6 << eCT_fu),               // Retrieves or sets (?) the index of the currently selected child module
    ectAssignment = (7 << eCT_fu),          // Indicates that the command is assigned other commands that will at some point be executed
    ectAdminAction = (8 << eCT_fu),         // Indicates a command that invokes and immediate, momentary, action meant for administrative use
    ectLiveParameter = (9 << eCT_fu),       // A parameter setting intended for live on-the-fly modification that may or may not be saved
    ectLiveAction = (10 << eCT_fu),         // Invokes a momentary action that is meant as a live control which doesn't keep a state, generally not saved
    ectAdminSetting = (11 << eCT_fu),       // Indicates an administrative setting
    ectLiveStatistics = (12 << eCT_fu),     // A command that is used for statistics, generally a return-only, no-save
    ectInternalStatistics = (13 << eCT_fu)  // Statistical function only used for internal use
};

// Using 4 bits (+ 0b00000xxxxxxxxxxx
#define eCT_fl (eCT_fu + 4)
enum eCommandType_flags {
    ectSystem = (1 << eCT_fl),             // Used for system-related commands not meant for user interaction
    ectLogic = (2 << eCT_fl),              // Used to perform logical functions in conjunction with other commands, does nothing on its own
    ectVolatileSetting = (4 << eCT_fl),    // Indicates a setting that is potentially harmful to change
};

/*
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
*/
struct ModuleCommandDeclaration {
    const String longCommand;
    const String shortCommand;
    const String arguments;
    const String Help;
    const bool hidden;
    const bool save;
    eProcessResult (*commandFunction) (ModuleCommand*, ModuleCommandDeclarationArguments);
    //const eCommandType cType;
    const uint16_t cType;
    const String variables;
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
    const eModuleType moduleType = eModuleType::software;
    bool isGroupHandler = false;
    std::string shortAlias;
};

extern const String moduleTypeDescription[];

String getModuleType(uint8_t mType);

#include "module.hpp"

#define staticCopy(name, className) \
    static eProcessResult s_##name(ModuleCommand* self, ModuleCommandDeclarationArguments) { return static_cast<className*>(self)->name(inCommandItem, inCommandResponses, request, thisItem); }

#define CREATE_MODULE_COMMAND_FUNCTION_FWD(funcname, className) eProcessResult funcname(ModuleCommandDeclarationArguments); staticCopy(funcname, className)

#define CREATE_MODULE_COMMAND_FUNCTION(funcname, className) eProcessResult className::funcname(ModuleCommandDeclarationArguments)

//#define getModuleCount(name) int name::getModuleCommandCount() { return sizeof(moduleCommands) / sizeof(moduleCommands[0]); }
#define getModuleCount(name) int name::getModuleCommandCount() const { return sizeof(moduleCommands) / sizeof(moduleCommands[0]); }

#define MODULECOMMANDHANDLER \
    static const ModuleCommandDeclaration moduleCommands[]; \
    int getModuleCommandCount() const; \
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
