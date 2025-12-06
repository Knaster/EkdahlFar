#ifndef FARSINGLE_H
#define FARSINGLE_H

#include "debugprint.hpp"

#include "mutecontrol.cpp"
#include "solenoid.cpp"
#include "bowcontrol.cpp"
#include "bowActuators.cpp"
#include "harmonicserieshandler.cpp"
#include "controlReader.cpp"

#include "calibrationhelpers.hpp"

#include "bowcalibration.cpp"
#include "mutecalibration.cpp"

serialCommandItem serialCommandsFarSingle[] = {
    { "bowcalibrateall", "bca", "-", "Performs all calibration routines on the selected bow, see below for routines performed" },
    { "bowcalibratespeed", "bcs", "-", "Finds the minimum and maximum bow speed of the selected bow" },
    { "bowcalibratepressure", "bcp", "-", "Finds the minimum and maximum bow pressure of the selected bow" },
    { "pickupstringfrequency", "psf", "-", "Returns the fundamental tone calculated from the current audio signal if appliccable"},
    { "pickupaudiopeak", "pap", "-", "Returns the peak amplitude of the current audio signal"},
    { "pickupaudiorms", "par", "-", "Returns the RMS amplitude of the current audio signal"},
    { "mutecalibrate", "mca", "-", "Calibrate mute settings"}
};

class FARSingle {
public:
    FARSingle(void *muteStepperCallback, void *pressureStepperCallback, void *tachometerCallback, void *pidCallback);

    eProcessResult processSerialCommand(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                               commandList *delegatedCommands = nullptr);

    eProcessResult processSerialCommandHidden(commandItem *inCommandItem, std::vector<commandResponse> *commandResponses, bool request = false, bool delegate = false,
                                    commandList *delegatedCommands = nullptr);
    String dumpData();

private:
    MuteControl *muteControl;
    Solenoid *hammerControl;
    BowControl *bowControl;

    CalibrateBow *calibrateBow;
    CalibrateMute *calibrateMute;

    BowActuators *bowActuators;
    HarmonicSeriesHandler *harmonicSeriesHandler;

    IntervalTimer pidInterrupt0;
public:
    ControlReader *controlReader;
    void updateControlReader();

    void update();

    void updatePressureServo() { bowControl->updateServo(); }
    void updateMuteServo() { muteControl->updateServo(); }
    void updateTachometer() { bowControl->updateTachometer(); }
    void updatePID() { bowControl->updatePID(); }
};

#endif // FARSINGLE_C
