#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "tinyexpr.h"
#include <vector>
#include "base/arduinorequired.hpp"

extern double dMap[];
extern double epZeroThreshold;

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
    std::vector<te_variable> expFunctions;
    #define expFunctionCount expFunctions.size()

    static double expBool(double input);
    static double expIBool(double input);
    static double deadband(double value, double threshold);
    static double zeroThreshold(double value);
    static double ifequal(double value, double comp, String commandList);
public:
    ExpressionParser();

    String parseCommandExpressions(String expression);

    void registerFunction(const char *name, const void *address, int type);

    bool setVariable(const char* name, double value);
};

#endif // EXPRESSIONPARSER_H
