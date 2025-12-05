#ifndef HARMONICSERIESHANDLER_H
#define HARMONICSERIESHANDLER_H

#include "harmonicSeries.cpp"

serialCommandItem serialCommandsHarmonicSeriesHandler[] = {
    { "bowcontrolfundamental", "bcu", "float", "Bow fundamental frequency, all harmonics are calculated from this number" },
    { "bowcontrolharmonic", "bch", "int", "Bow motor speed in terms of a harmonic number. A ratio is taken from the given harmonic in the current harmonic list, the ratio is then multiplied by the bow fundamental frequency" },
    { "bowcontrolharmonicadd", "bcha", "int", "Additative version of bowcontrolharmonic, the number is given is added to the harmonic given" },
    { "bowcontrolharmonicbase", "bchb", "int", "Same as bowcontrolharmonic but where the harmonic number is based on a MIDI note given by bowcontrolbasenote" },
    { "bowcontrolharmonicbasenote", "bchbn", "0-127", "Sets the MIDI base note of the string, used in conjunction with bowcontrolharmonicbase"},
    { "bowcontrolharmonicshift", "bchsh", "-32767-32767", "Setting shift from the currently playing harmonic where 32767 equals the entire harmonic shift range shifted up" },
    { "bowcontrolharmonicshiftrange", "bchsr", "0-36", "Set the number of harmonic numbers that constitutes the entire harmonic shift" },
    { "bowcontrolharmonicshift5", "bchs5", "-32767-32767", "Setting shift from the currently playing harmonic over 5 octaves where 32767 equals 5 octaves shift up from the fundamental" },

    { "bowharmonicseries", "bhs", "int", "Get/set the current harmonic series" },
    { "bowharmonicseriesdata", "bhsd", "slot:name:ratios", "Sets all data for the harmonic series in the given slot" },
    { "bowharmonicseriesratio", "bhsr", "harmonic:ratio", "Sets the ratio of the given harmonic in current harmonic series, will increase the list size if needed to adress the harmonic" },
    { "bowharmonicseriesratioremove", "bhsrr", "ratio", "Remove the harmmonic ratio given in the current series and shift any ratios accordingly. Cannot remove all ratios"},
    { "bowharmonicseriescount", "bhsc", "-", "Returns the number of harmonic series in the list and their IDs"},
    { "bowharmonicseriessave", "bhss", "series:name", "Saves the current harmonic series in the slot given, or if the slot is out of range, creates a new one at the end of the list"},
    { "bowharmonicseriesremove", "bhsrm", "series", "Remove the series given and shift any series accordingly. Cannot remove all series"}
};

class HarmonicSeriesHandler {
public:
    HarmonicSeriesHandler();

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
    String dumpData();

private:
    //HarmonicSeries currentHarmonicSeriesData;
    HarmonicSeriesList harmonicSeriesList;
    uint8_t currentHarmonicSeries = 0;

    float fundamentalFrequency = 66;
    uint8_t baseNote = 64;

    float currentHarmonicFreq = 0;            ///< Calculated base frequency of current harmonic
    float currentHarmonicShiftFreq = 0;       ///< Calculated frequency with harmonic shift

    int harmonicShiftRange = 12;              ///< Harmonic shift range for one 16-bit integer (full range up and down)
    int harmonicShift = 0;                    ///< Harmonic shift given as one 16-bit integer
    int harmonicShift5 = 0;
    int harmonic = 0;                         ///< Current harmonic
    int harmonicAdd = 0;

    int16_t lowerHarmonic = -48;
    int16_t upperHarmonic = 49;

    bool frequencyChanged = false;
private:
    bool calculateHarmonicShift();

    bool raiseFrequencyChanged();

    eProcessResult processControlCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processHarmonicSeriesCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                      commandList *delegatedCommands = nullptr);

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

    void addHarmonicSeries(String id, float frequencies[]) { harmonicSeriesList.addHarmonicSeries(id, frequencies); }

    float getCalculatedFrequency() { return currentHarmonicShiftFreq; }

    bool checkFrequencyChanged();
};

#endif // HARMONICSERIESHANDLER_H
