#ifndef STRINGMODULE_HPP
#define STRINGMODULE_HPP

serialCommandItem serialCommandsStringModule[] = {
    { "bow", "b", "0-15", "Sets the currently selected bow"},

    //pid
    { "bowpid", "bpid", "1|0", "Sets the bow PID on/off" },
    //pidki, ki
    { "bowpidki", "bpki", "float", "Sets the Ki parameter of the PID of the selected bow" },
    //pidkp, kp
    { "bowpidkp", "bpkp", "float", "Sets the Kp parameter of the PID of the selected bow" },
    //pidkd, kd
    { "bowpidkd", "bpkd", "float", "Sets the Kd parameter of the PID of the selected bow" },
    //integratorerror, ie
    { "bowpidintegratorerror", "bpie", "float", "Sets the lower threshold of error values for the PID integrator to ignore of the selected bow" },
    //pidr, pidr
    { "bowpidr", "bpir", "-", "Resets the PID of the selected bow" },
    //pidmaxerror, pme
    { "bowpidmaxerror", "bpme", "float", "Maximum error to correct in each PID loop, essentially sets acceleration" },
    //pidpeakerror, pidperr
    { "bowpidpeakerror", "bpperr", "float", "Get latest PID peak error"},

    //frequency, f
    { "bowcontrolfrequency", "bcf", "float", "Bow motor speed frequency in hertz, requires that PID is turned on" },
    //fundamental, u
    { "bowcontrolfundamental", "bcu", "float", "Bow fundamental frequency, all harmonics are calculated from this number" },
    //harmonic, h
    { "bowcontrolharmonic", "bch", "int", "Bow motor speed in terms of a harmonic number. A ratio is taken from the given harmonic in the current harmonic list, the ratio is then multiplied by the bow fundamental frequency" },
    //harmonicbase, hb
    { "bowcontrolharmonicbase", "bchb", "int", "Same as bowcontrolharmonic but where the harmonic number is based on a MIDI note given by bowcontrolbasenote" },
    //setbasenote, sbn
    { "bowcontrolharmonicbasenote", "bchbn", "0-127", "Sets the MIDI base note of the string, used in conjunction with bowcontrolharmonicbase"},
    //harmonicseries, hs
    { "bowcontrolharmonicseries", "bchs", "int", "Set/Get the current harmonic series" },
    //harmoniclist, hl
    { "bowcontrolharmoniclist", "bchl", "-", "List the harmonic ratios in the current harmonic series" },
    //setharmonic, sh
    { "bowcontrolharmonicratio", "bchr", "harmonic(int):ratio(float)", "Sets the ratio of the given harmonic in current harmonic series" },
    //harmoniccount, hc
    { "bowcontrolharmoniccount", "bchc", "-", "Returns the number of harmonic lists"},
    //setharmonicshift, shs
    { "bowcontrolharmonicshift", "bchsh", "-32767-32767", "Setting shift from the currently playing harmonic where 32767 equals the entire harmonic shift range shifted up" },
    //harmonicshiftrange, hsr
    { "bowcontrolharmonicshiftrange", "bchsr", "0-36", "Set the number of notes for the entire harmonic shift" },
    //setharmonicshift5, shs5
    { "bowcontrolharmonicshift5", "bchs5", "-32767-32767", "Setting shift from the currently playing harmonic over 5 octaves where 32767 equals 5 octaves shift up from the fundamental" },
    //setspeedmode, ssm
    { "bowcontrolspeedmode", "bcsm", "0|1", "Bow motor speed mode, 0 = Automatic and 1 = Manual" },

    //status, s
    { "bowstatus", "bs", "-", "Prints status information for the selected bow" },
    //stringfrequency, sf
    { "pickupstringfrequency", "psf", "-", "Samples the pickup and returns the fundamental tone calculated, if appliccable"},
    //measuretimetotarget, mtt
    { "bowdebugmeasuretimetotarget", "bdmtt", "float", "Measure the time it takes to change from the current frequency to the target frequency" },
    { "bowhome", "bh", "-", "Homing bow, used at startup and in case of the bow loosing position" },

    //calibrateall, ca
    { "bowcalibrateall", "bca", "-", "Performs all calibration routines on the selected bow, see below for routines performed" },
    //calibratespeed, cms
    { "bowcalibratespeed", "bcs", "-", "Finds the minimum and maximum bow speed of the selected bow" },
    //calibratespeedpid, cmsp
    { "bowcalibratespeedpid", "bcsp", "-", "Finds the minimum and maximum bow speed of the selected bow using the PID" },
    //calibratepressure, cmp
    { "bowcalibratepressure", "bcp", "-", "Finds the minimum and maximum bow pressure of the selected bow" },
    //stablethreshold, st
    { "bowcalibratestablethreshold", "bcst", "float", "Sets the timeout value for the bow frequency stability check for the selected bow" },

    //run, run
    { "bowmotorrun", "bmr", "1|0", "Set bow motor run on/off" },
    //setbowpower, sbp
    { "bowmotorpower", "bmp", "0-65535", "Bow motor raw power in 16-bit PWM values, requires that the PID is turned off" },
    //setbowtimeout, sbt
    { "bowmotortimeout", "bmt", "ms(0-65535)", "Bow motor shutdown timeout after bow having been put into the rest position" },
    //bowmaxspeed, bxs
    { "bowmotorspeedmax", "bmsx", "0-65535", "Bow motor maximum speed limit" },
    //bowminspeed, bis
    { "bowmotorspeedmin", "bmsi", "0-65535", "Bow motor minimum speed limit"},
    //bowvoltage, bv
    { "bowmotorvoltage", "bmv", "1.2-9", "Bow motor voltage"},
    //bowfrequency, bf
    { "bowmotorfrequency", "bmf", "-", "Bow motor reported frequency"},
    //bowcurrent, bc
    { "bowmotorcurrent", "bmc", "0-3", "Bow motor reported current use" },
    //bowcurrentlimit, bcl
    { "bowmotorcurrentlimit", "bmcl", "0-3", "Bow motor current limit" },
    //bowmininertiapwm, biiw
    { "bowmotormininertialpwm", "bmmip", "0 - 65535", "Bow minimal PWM to keep inertia"},

/*
    //recoverrate, rr
    { "bowpressurerecoverrate", "bprr", "float", "Bow pressure recover rate for the selected bow" },*/
    //setpressurebaseline, spb
    { "bowpressurebaseline", "bpb", "0-65535", "Bow pressure baseline, modulation is added to this point upward" },
    //setpressuremodifier, spm
    { "bowpressuremodifier", "bpm", "0-65535", "Bow pressure modulation, added to the baseline" },
    //rest, rest
    { "bowpressurerest", "bpr", "0|1", "Puts the bow pressure in the resting position (conditional)" },
    //engage, engage
    { "bowpressureengage", "bpe", "0|1", "Puts the bow pressure in the engage position (conditional)" },
    //bowmaxpressure, bxp
    { "bowpressurepositionmax", "bppx", "0-65535", "Bow pressure stall/maximum position"},
    //bowminpressure, bip
    { "bowpressurepositionengage", "bppe", "0-65535", "Bow pressure touch/minimum position"},
    //bowrestposition, brp
    { "bowpressurepositionrest", "bppr", "0-65535", "Bow pressure rest position"},
    //bowengagespeed, bes
    { "bowpressureengagespeed", "bpes", "1 - 100?", "Bow pressure movement speed when engaging or disengaging"},
    //bowmodulationspeed, bms
    { "bowpressuremodulationspeed", "bpms", "0.1 - 10", "Bow pressure movement speed while engaged"},

//    { "setbowhold", "sbh", "0|1", "Sets bow hold on/off" },

    // solenoid, solenoid
    { "solenoid", "so", "int", "Sets the currently selected solenoid" },
    { "solenoidengage", "se", "0-65535", "Sets the solenoid engagement of the current bow using the 1st argument as hardness" },
    { "solenoiddisengage", "sd", "0-1", "Sets the solenoid disengagement of the current bow, an argument of '0' will leave the solenoid at its current state while any other value will disengage the solenoid" },
    // setsolenoidmaxforce, ssxf
    { "solenoidmaxforce", "sxf", "0 - 65535", "Set solenoid maximum usable force" },
    // setsolenoidminforce, ssif
    { "solenoidminforce", "sif", "0 - 65535", "Set solenoid minimum usable force" },
    // setsolenoidforcemultiplier, ssfm
    { "solenoidforcemultiplier", "sfm", "0 - 1", "Set solenoid force multiplier" },

    { "mutesetposition", "msp", "0-65535", "Set mute position" },
    { "mutefullmute", "mfm", "1|0", "Put mute in full mute position (conditional)" },
    { "mutehalfmute", "mhm", "1|0", "Put mute in half mute position (conditional)" },
    { "muterest", "mr", "1|0", "Put mute in rest position (conditional)" },
    { "mutesavefull", "msf", "1|0", "Save current mute position as full mute position (conditional)" },
    { "mutesavehalf", "msh", "1|0", "Save current mute position as half mute position (conditional)" },
    { "mutesaverest", "msr", "1|0", "Save current mute position as mute rest position (conditional)" },
    { "mutefullmuteposition", "mfmp", "0-65535", "Mute full mute position"},
    { "mutehalfmuteposition", "mhmp", "0-65535", "Mute half mute position"},
    { "muterestposition", "mrp", "0-65535", "Mute rest position"},
    { "mutesustain", "ms", "0|1", "Setting sustain on (1) or off (0)" },
    { "mutebackoff", "mbo", "0-65535", "Setting the time that the mute stays in the mutefullmute position before automatically going into rest, set in mS"},

    { "bowactuatoradd", "baa", "-", "Add new bow actuator"},
    { "bowactuatorremove", "bar", "actuator", "Remove bow actuator"},
    { "bowactuatorset", "bas", "actuator", "Set current bow actuator"},
    { "bowactuatorload", "bal", "-", "Load parameters from current bow actuator"},
    { "bowactuatorsave", "bav", "-", "Save current bow parameters into currently selected bow actuator" },
    { "bowactuatordata", "bad", "actuator:engagepressure:stallpressure:resposition", "Set data of bow actuator [actuator]" },
    { "bowactuatorcount", "bac", "-", "Returns the amount of saved bow actuators"},
    { "bowactuatorid", "bai", "-", "Sets the ID of the current bow actuator"},
    { "bowactuatorsavestall", "bass", "-", "Save current position as bow actuator stall position"}, //NEW - ADD
    { "bowactuatorsavefirsttouch", "basft", "-", "Save current position as bow actuator first touch position"}, //NEW - ADD
    { "bowactuatorsaverest", "basr", "-", "Save current position as bow actuator rest position"}, //NEW - ADD

    { "adcr", "adcr", "-", "ADC value changed report, cannot be invoked manually"}
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
