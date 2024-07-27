#ifndef MUTECALIBRATION_HPP
#define MUTECALIBRATION_HPP

class calibrateMute {
public:
    mute *m_muteConnect;
    bowIO *m_bowIOConnect;
    bowControl *m_bowControlConnect;

    uint16_t maxTestStep = 50000;

    #define maxStepRetract 2000
    #define minAmplitude 0.02

    calibrateMute(mute &t_Mute, bowIO &t_bowIO, bowControl &t_bowControl);

    bool calibrateAll();

private:
    float levelSilence = 0;
    float levelFundamental = 0;
//    float levelFundamentalRMS = 0;
    uint16_t stallPosition = 0;

    float findLevel();

    bool findLevelSilence();
//    bool findlevelFundamentalPeak();
//    bool findLevelFundamentalRMS();
    bool findMuteLevels();

    bool findMuteStall();
    bool findMuteFirstContact();
    bool findMuteSilence();

};

#endif
