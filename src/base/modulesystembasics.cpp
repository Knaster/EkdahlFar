#ifndef MODULESYSTEMBASICS_CPP
#define MODULESYSTEMBASICS_CPP

#include "base/modulesystembasics.hpp"

String commandTypeDescription[23] = {
    "string",
    "bool",
    "float",
    "uint8",
    "int16",
    "uint16",
    "commands",
    "data",
    "conditional",
    "returnonly",
    "immediate",
    "expression",
    "add",
    "remove",
    "count",
    "name",
    "index",
    "milliseconds",
    "microseconds",
    "hertz",
    "requestonly",
    "returnrequest",
    "outputassignment"
};

const String moduleTypeDescription[3] = { "Software", "Hardware", "Remote" };

String getModuleType(uint8_t mType) {
    if ((mType > sizeof(moduleTypeDescription)) || (mType < 0)) { return ""; }
    return moduleTypeDescription[mType];
}

#endif // MODULESYSTEMBASICS_CPP
