#ifndef ARDUINO_REQUIRED
#define ARDUINO_REQUIRED

#ifdef ARDUINO_TEENSY40
#include <WProgram.h>
#else
#include "WString.h"
#endif

#include <algorithm> // for min/max
#define clamp(v,i,x) min(max(v,i),x)

#include <vector>

#include <base/debugprint.hpp>

#endif // ARDUINO_REQUIRED

