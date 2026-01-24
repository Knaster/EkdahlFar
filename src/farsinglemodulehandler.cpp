#ifndef FARSINGLEMODULEHANDLER_CPP
#define FARSINGLEMODULEHANDLER_CPP

#include "farsinglemodulehandler.hpp"



FarSingleModuleHandler::FarSingleModuleHandler()
{
    //ctor
}

bool FarSingleModuleHandler::enumerateModules() {
    farSingle = new FARSingle(&farSingleUpdateServoStepperMute0, &farSingleUpdateServoStepperPressure0, &farSingleUpdateTachometer0, &farSingleUpdatePID0);

    addModule(farSingle);

    return true;
}

#endif
