#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "tinyexpr.h"

#define mapSize 12    // C  C# D  D# E  F  F# G  G# A  A# B
double dMap[mapSize] = { 1, 1, 0, 1, 2, 1, 0, 0, 1, 0, 1, 2 };
double epZeroThreshold = 200;

class ExpressionParser {
public:
    double dnote;
    double dchannel;
    double dvelocity;
    double dnotecount;
    double dpressure;
    double dvalue;
    double dpitch;
    double dprogram;
    #define userVariableMax 10
    double duv[userVariableMax] = { 0,0,0,0,0,0,0,0,0,0 };

private:
    #define expFunctionCount 23
    te_variable expFunctions[expFunctionCount] = {
        { "channel", &dchannel } ,
        { "note", &dnote },
        { "velocity", &dvelocity },
        { "notecount", &dnotecount},
        { "pressure", &dpressure},
        { "value", &dvalue },
        { "pitch", &dpitch },
        { "program", &dprogram },
        { "bool", nullptr, TE_FUNCTION1 },
        { "ibool", nullptr, TE_FUNCTION1 },
        { "mapkeys", nullptr, TE_FUNCTION1 },
        { "deadband", nullptr, TE_FUNCTION2 },
        { "zerothreshold", nullptr, TE_FUNCTION1 },
        { "uv0", &duv[0]}, { "uv1", &duv[1]}, { "uv2", &duv[2]}, { "uv3", &duv[3]}, { "uv4", &duv[4]},
        { "uv5", &duv[5]}, { "uv6", &duv[6]}, { "uv7", &duv[7]}, { "uv8", &duv[8]}, { "uv9", &duv[9]} };

    void setFunctionPointer(const char* name, const void* address, int type);

    static double mapKeys(double key);
    static double expBool(double input);
    static double expIBool(double input);
    static double deadband(double value, double threshold);
    static double zeroThreshold(double value);
    static double ifequal(double value, double comp, String commandList);
public:
    ExpressionParser();

    String parseCommandExpressions(String expression);

    bool setVariable(const char* name, double value);
};

#endif // EXPRESSIONPARSER_H
