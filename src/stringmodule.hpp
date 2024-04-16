#ifndef STRINGMODULE_HPP
#define STRINGMODULE_HPP

serialCommandItem serialCommandsStringModule[] = {
    { "bow", "b", "0-15", "Sets the currently selected bow"},
    { "pidki", "ki", "float", "Sets the Ki parameter of the PID of the selected bow" },
    { "pidkp", "kp", "float", "Sets the Kp parameter of the PID of the selected bow" },
    { "pidkd", "kd", "float", "Sets the Kd parameter of the PID of the selected bow" },
    { "frequency", "f", "float", "Sets bow speed frequency in hertz of the selected bow, requires that PID is turned off" },
    { "status", "s", "-", "Prints status information for the selected bow" },
    { "pidr", "-", "-", "Resets the PID of the selected bow" },
    { "integratorerror", "ie", "float", "Sets the lower threshold of error values for the PID integrator to ignore of the selected bow" },
    { "fundamental", "u", "float", "Sets the fundamental frequency of the selected bow" },
    { "calibrateall", "ca", "-", "Performs all calibration routines on the selected bow, see below for routines performed" },
    //  { "calibratefundamental", "cf", "", "Attempts to find the fundamental frequency of the selected bow" },
    { "calibratespeed", "cms", "-", "Finds the minimum and maximum bow speed of the selected bow" },
    { "calibratespeedpid", "cmsp", "-", "Finds the minimum and maximum bow speed of the selected bow using the PID" },
    { "calibratepressure", "cmp", "-", "Finds the minimum and maximum bow pressure of the selected bow" },
//    { "calibratesave", "cs", "", "Saves the calibration data for the selected bow in EEPROM" },
//    { "calibratelist", "cl", "", "Lists all the current calibration data" },
    { "run", "-", "1|0", "Sets the bow run on/off" },
    { "pid", "-", "1|0", "Sets the bow PID on/off" },
    { "stablethreshold", "st", "float", "Sets the timeout value for the bow frequency stability check for the selected bow" },
    { "recoverrate", "rr", "float", "Sets the tilt recover rate for the selected bow" },
    { "harmonic", "h", "float", "Sets the current harmonic frequency of the current bow, calculations based on the fundametal frequency" },
    { "harmonicbase", "hb", "0-127", "Sets the current harmonic frequency of the current bow, calculations based on the fundametal frequency and base note" },
    { "solenoid", "solenoid", "int", "Sets the currently selected solenoid" },
    { "solenoidengage", "se", "0-65535", "Sets the solenoid engagement of the current bow using the 1st argument as hardness" },
    { "solenoiddisengage", "sd", "0-1", "Sets the solenoid disengagement of the current bow, an argument of '0' will leave the solenoid at its current state while any other value will disengage the solenoid" },
//    { "setmanualpressure", "smp", "|", "Sets the bow pressure of the current bow" },
    { "setpressurebaseline", "spb", "0-65535", "X" },
    { "setpressuremodifier", "spm", "0-65535", "X" },
    { "rest", "rest", "0|1", "Put the bow in the resting position (conditional)" },
    { "engage", "engage", "0|1", "Put the bow in the engage position (conditional)" },
    //  { "mute", "", "0|1", "Put the bow in the mute position (conditional)" },
    { "measuretimetotarget", "mtt", "float", "Measure time to change to target" },
    { "pidmaxerror", "pme", "float", "Maximum error to correct in each PID loop, essentially sets acceleration" },
    //  { "muteforce", "mf", "float", "Amount of force to add to the tilt maximum when muting the string" },
    { "setbowpower", "sbp", "0-65535", "Sets the raw bow motor power" },
    { "setbowhold", "sbh", "0|1", "Sets bow hold on/off" },
    { "setspeedmode", "ssm", "0|1", "Set bow speed mode where 0 = Automatic and 1 = Manual" },
    { "setbowtimeout", "sbt", "int", "Set bow motor timeout in milliseconds" },
    { "mutesetposition", "msp", "0-65535", "Set mute position" },
    { "mutefullmute", "mfm", "1|0", "Put mute in full mute position (conditional)" },
    { "mutehalfmute", "mhm", "1|0", "Put mute in half mute position (conditional)" },
    { "muterest", "mr", "1|0", "Put mute in rest position (conditional)" },
    { "mutesavefull", "msf", "1|0", "Save current mute position as full mute position (conditional)" },
    { "mutesavehalf", "msh", "1|0", "Save current mute position as half mute position (conditional)" },
    { "mutesaverest", "msr", "1|0", "Save current mute position as mute rest position (conditional)" },
//    { "mutesaveparams", "", "", "Save mute parameters in EEPROM" },
    { "bowmaxspeed", "bxs", "0-65535", "Bow motor maximum speed limit" },
    { "bowminspeed", "bis", "0-65535", "Bow motor minimum speed limit"},
    { "bowmaxpressure", "bxp", "0-65535", "Bow motor stall/maximum pressure limit"},
    { "bowminpressure", "bip", "0-65535", "Bow motor initial touch/minimum pressure limit"},
    { "bowrestposition", "brp", "0-65535", "Bow pressure rest position"},
    { "mutefullmuteposition", "mfmp", "0-65535", "Mute full mute position"},
    { "mutehalfmuteposition", "mhmp", "0-65535", "Mute half mute position"},
    { "muterestposition", "mrp", "0-65535", "Mute rest position"},
    { "harmonicseries", "hs", "int", "Set the current harmonic series" },
    { "harmoniclist", "hl", "-", "List the harmonic ratios in the current harmonic series" },
    { "setharmonic", "sh", "harmonic:scale", "Sets the scale from the fundamental of the given harmonic in current harmonic series" },
    { "harmoniccount", "hc", "-", "Returns the number of harmonic lists"},
    { "bowvoltage", "bv", "1.2-9", "Sets the voltage of the bow motor"},
    { "stringfrequency", "sf", "-", "Retrieves the current string frequency"},
    { "bowfrequency", "bf", "-", "Retrieves the current bow frequncy"},
    { "setbasenote", "sbn", "0-127", "Sets the MIDI base note of the string"},
    { "bowcurrent", "bc", "0-3", "Gets the bow current use" },
    { "bowcurrentlimit", "bcl", "0-3", "Sets the bow current limit" },
    { "mutesustain", "ms", "0|1", "Setting sustain on (1) or off (0)" },
    { "setharmonicshift", "shs", "-32767 - 32767", "Setting shift from the currently playing harmonic where 32767 equals the entire harmonic shift range shifted up" },
    { "harmonicshiftrange", "hsr", "0-36", "Set amount of notes for the harmonic shift" },
    { "setharmonicshift5", "shs5", "-32767 - 32767", "Setting shift from the currently playing harmonic over 5 octaves where 32767 equals 5 octaves shift up" },
    { "setsolenoidmaxforce", "ssxf", "0 - 65535", "Set solenoid maximum usable force" },
    { "setsolenoidminforce", "ssif", "0 - 65535", "Set solenoid minimum usable force" },
    { "setsolenoidforcemultiplier", "ssfm", "0 - 1", "Set solenoid force multiplier" },
    { "pidpeakerror", "pidperr", "float", "Get latest PID peak error"},
    { "bowmininertialpwm", "biiw", "0 - 65535", "Bow minimal PWM to keep inertia"},
    { "bowmodulationspeed", "bms", "0.1 - 10", "Set maximum bow movement speed once engaged"},
    { "bowengagespeed", "bes", "1 - 100?", "Set maximum bow movement when engaging or disengaging"},

    { "bowactuatoradd", "baa", "-", "Add new bow actuator"},
    { "bowactuatorremove", "bar", "actuator", "Remove bow actuator"},
    { "bowactuatorset", "bas", "actuator", "Set current bow actuator"},
    { "bowactuatorload", "bal", "-", "Load parameters from current bow actuator"},
    { "bowactuatorsave", "bav", "-", "Save current bow parameters into currently selected bow actuator" },
    { "bowactuatordata", "bad", "actuator,0-65535:0-65535:0-65535", "Set all data of current bow actuator (firstTouchPressure, stallPressure, restPosition)" },
    { "bowactuatorcount", "bac", "-", "Returns the amount of saved bow actuators"},
    { "bowactuatorid", "bai", "-", "Sets the ID of the current bow actuator"},
};

class stringModule {
    public:
    std::vector<bowIO> bowIOArray;
    std::vector<bowControl> bowControlArray;
    std::vector<_calibrationData> calibrationDataArray;
    std::vector<calibrate> calibrateArray;

    std::vector<solenoid> solenoidArray;

    std::vector<mute> muteArray;

    int EEPROM_offset = 0;

    int currentBowSerial = 0;
    int currentSolenoidSerial = 0;

    BufferedOutput *slaveSerialOut = nullptr;
    SafeStringReader *slaveSerialRead = nullptr;

    stringModule();
    void updateString();
    bool addBow(char motorRevPin, char motorVoltagePin, char motorDCDCEn, char tachoPin, char currentSensePin, char motorFaultPin, char stepEnPin, char stepDirPin,
        char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin, char stepCorrectionSensorPin);
    bool addSolenoid(char _solenoidPin);
    void writeToSlave(String command);
    bool addMute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin);

    bool processSerialCommand_GeneralControl(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
        commandList *delegatedCommands = nullptr);
    bool processSerialCommand_CalibrationsSettings(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
        commandList *delegatedCommands = nullptr);
    bool processSerialCommand_MuteControl(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
        commandList *delegatedCommands = nullptr);
    bool processSerialCommand_StatusTesting(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
        commandList *delegatedCommands = nullptr);
    bool processSerialCommand_BowActuator(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegated = false,
        commandList *delegatedCommands = nullptr);
    bool processRequestCommand(commandItem *_commandItem, std::vector<commandResponse> *commandResponses, bool delegated);
    bool processSerialCommand(commandList *commands, uint16_t *index, std::vector<commandResponse> *commandResponses, bool delegated);
};
#endif
