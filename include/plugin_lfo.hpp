#ifndef PLUGIN_LFO_HPP
#define PLUGIN_LFO_HPP

class Plugin_LFO : public Plugin
{
public:
    SETMODULEID("lfo", "lfo", "Basic software LFO v1.0", eModuleType::software, false)

    CREATEPLUGINFUNCTIONS(Plugin_LFO)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(enable, Plugin_LFO)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(target, Plugin_LFO)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(updaterate, Plugin_LFO)

    CREATE_MODULE_COMMAND_FUNCTION_FWD(waveform, Plugin_LFO)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(frequency, Plugin_LFO)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(amplitude, Plugin_LFO)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(delay, Plugin_LFO)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(resetdelay, Plugin_LFO)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(resetwave, Plugin_LFO)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(bipolar, Plugin_LFO)

    Plugin_LFO();

    void update() override;
private:
    bool pEnable = false;
    String pTarget = "bw.hsh.sh5:lfoout";
    uint16_t pUpdateRate = 10;

    enum eWaveform {
        sawtooth = 0,
        triangle = 1,
        sine = 2,
        square = 3,
        random = 4
    };

    struct tWaveform {
        String name;
        eWaveform waveform;
    };

    #define WAVEFORMS 5

    tWaveform pWaveData[WAVEFORMS] = { { "saw", eWaveform::sawtooth },
                                       { "triangle", eWaveform::triangle },
                                       { "sine", eWaveform::sine },
                                       { "square", eWaveform::square },
                                       { "random", eWaveform::random } };


    eWaveform pWaveform = eWaveform::sine;
    uint16_t pAmplitude = 100;
    float pFrequency = 4;
    uint16_t pDelay = 0;
    bool pBipolar = true;

    float pCount = 0;
    float pDelayCount = 0;

    float calculatedIncrease;
    float delayIncrease;

    void recalculateIncrease();

    float shapeTriangle();
    float shapeSine();
    float shapeSquare();

    String lastOutput = "";
};

REGISTERPLUGIN(Plugin_LFO)

#endif // PLUGIN_LFO_HPP
