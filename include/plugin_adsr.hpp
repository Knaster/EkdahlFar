#ifndef PLUGIN_AHDSR_HPP
#define PLUGIN_AHDSR_HPP


class Plugin_AHDSR : public Plugin
{
public:
    SETMODULEID("ahdsr", "ahdsr", "Basic software AHDSR v1.0", eModuleType::software, false)

    CREATEPLUGINFUNCTIONS(Plugin_AHDSR)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(name, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(enable, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(target, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(updaterate, Plugin_AHDSR)

    CREATE_MODULE_COMMAND_FUNCTION_FWD(gate, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(attack, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(decay, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(hold, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(sustain, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(release, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(amplitude, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(releasetarget, Plugin_AHDSR)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(invert, Plugin_AHDSR)

    Plugin_AHDSR();

    void update() override;

private:
    String pName = "";
    bool pEnable = false;
    String pTarget = "bw.bp.mo:ahdsrout";
    String pReleaseTarget = "bw.bp.rs:1";
    bool pReleased = false;
    uint16_t pUpdateRate = 10;

    bool pGate = false;

    float pValue = 0;
    float pHoldValue = 0;

    float pAmplitude = 5000;

    uint16_t pAttack = 1000;
    uint16_t pHold = 1000;
    uint16_t pDecay = 3000;
    uint16_t pSustain = 16000;
    uint16_t pRelease = 5000;

    float pAttackRate = 0;
    float pHoldRate = 0;
    float pDecayRate = 0;
    float pReleaseRate = 0;

    bool pInvert = false;

    enum eStage {
        sAttack = 0,
        sHold = 1,
        sDecay = 2,
        sSustain = 3,
        sRelease = 4
    };

    eStage pStage = 4;

    int32_t pLastValue = 0;
    void recalculateRates();
};

REGISTERPLUGIN(Plugin_AHDSR)

#endif // PLUGIN_ADSR_HPP
