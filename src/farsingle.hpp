#ifndef FARSINGLE_H
#define FARSINGLE_H

#include "debugprint.hpp"

#include "mutecontrol.cpp"
#include "solenoid.cpp"
#include "bowcontrol.cpp"
#include "bowActuators.cpp"
#include "harmonicserieshandler.cpp"
#include "controlReader.cpp"

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
