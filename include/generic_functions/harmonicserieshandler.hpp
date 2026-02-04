#ifndef HARMONICSERIESHANDLER_H
#define HARMONICSERIESHANDLER_H

#include "base/modulehandler.hpp"
#include "generic_functions/harmonicSeries.hpp"

class HarmonicSeriesHandler : public ModuleHandler {
public:
    SETMODULEID("harmonicserieshandler", "hsh", "Harmonic series handler", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_INDEX_CALLBACK_FWD(harmonicSeriesIndexChanged, HarmonicSeriesHandler);

    CREATE_MODULE_COMMAND_FUNCTION_FWD(fundamental, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(harmonic, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(harmonicAdd, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(harmonicBase, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(baseNote, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(shift, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(shiftRange, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(shift5, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(data, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(add, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(remove, HarmonicSeriesHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(count, HarmonicSeriesHandler)

    HarmonicSeriesHandler();

    void dumpData(std::vector<commandResponse> *dataDump) override;

private:
    float fundamentalFrequency = 66;
    uint8_t pBaseNote = 64;

    float currentHarmonicFreq = 0;            ///< Calculated base frequency of current harmonic
    float currentHarmonicShiftFreq = 0;       ///< Calculated frequency with harmonic shift

    int harmonicShiftRange = 12;              ///< Harmonic shift range for one 16-bit integer (full range up and down)
    int harmonicShift = 0;                    ///< Harmonic shift given as one 16-bit integer
    int harmonicShift5 = 0;
    int pHarmonic = 0;                         ///< Current harmonic
    int pHarmonicAdd = 0;

    int16_t lowerHarmonic = -48;
    int16_t upperHarmonic = 49;

    bool frequencyChanged = false;
    bool harmonicSeriesChangeReported = false;
private:
    bool calculateHarmonicShift();

    bool raiseFrequencyChanged();
/*
    eProcessResult processControlCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processHarmonicSeriesCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                      commandList *delegatedCommands = nullptr);
*/
    //std::vector<HarmonicSeries> harmonicSeries;
public:
    bool setHarmonicShift(int inHarmonicShift);

    bool setHarmonicShift5(int inHarmonicShift5);

    int getHarmonicShift();

    int getHarmonicShift5();

    bool setHarmonicShiftRange(int inHarmonicShiftRange);

    int getHarmonicShiftRange();

    bool setHarmonic(int _harmonic);

    int getHarmonic();

    bool setHarmonicAdd(int _harmonic);

    int getHarmonicAdd();

    bool updateHarmonicData();

    bool setBaseNote(int inBaseNote);

    bool loadHarmonicSeries(int i);

    HarmonicSeries* addHarmonicSeries();

    HarmonicSeries* addHarmonicSeries(String id, float frequencies[] = NULL, int size = 0);

    float getCalculatedFrequency() { return currentHarmonicShiftFreq; }

    bool checkFrequencyChanged();
};

#endif // HARMONICSERIESHANDLER_H
