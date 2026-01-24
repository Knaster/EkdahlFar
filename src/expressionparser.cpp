#ifndef EXPRESSIONPARSER_C
#define EXPRESSIONPARSER_C

#include "expressionparser.h"

ExpressionParser::ExpressionParser() {
    setFunctionPointer("bool", (const void*) &expBool, TE_FUNCTION1);
    setFunctionPointer("ibool", (const void*) &expIBool, TE_FUNCTION1);
    setFunctionPointer("mapkeys", (const void*) &mapKeys, TE_FUNCTION1);
    setFunctionPointer("deadband", (const void*) &deadband, TE_FUNCTION2);
    setFunctionPointer("zerothreshold", (const void*) &zeroThreshold, TE_FUNCTION1);
}

void ExpressionParser::setFunctionPointer(const char* name, const void* address, int type) {
    for (int i=0; i<expFunctionCount; i++) {
        if (expFunctions[i].name == name) {
            expFunctions[i].address = address;
            expFunctions[i].type = type;
            return;
        }
    }
}

bool ExpressionParser::setVariable(const char* name, double value) {
    for (int i=0; i<expFunctionCount; i++) {
        if (expFunctions[i].name == name) {
            double *ptr = const_cast<double*>(static_cast<const double*>(expFunctions[i].address));
            *ptr = value;
            return true;
        }
    }
    return false;
}

double ExpressionParser::mapKeys(double key) {
    int mapIndex = (((int) key) % 12);
    if ((mapIndex < 0) || (mapIndex >= mapSize)) { return 0; }
    return dMap[mapIndex];
}

double ExpressionParser::expBool(double input) {
    if (input <= 0) { return 0; } else { return 1; };
}

double ExpressionParser::expIBool(double input) {
    if (input <= 0) { return 1; } else { return 0; };
}

double ExpressionParser::deadband(double value, double threshold) {
    double target = 0;
    if ((value < (target + threshold)) && (value > (target - threshold))) { return target; } else {
        if (value > target) {
            return value - threshold;
        } else {
            return value + threshold;
        }

    };
}

double ExpressionParser::zeroThreshold(double value) {
    if (value < epZeroThreshold) {
        return 0;
    } else {
        return value;
    }
}

String ExpressionParser::parseCommandExpressions(String inExpression) {
    const char* expression = inExpression.c_str();
    String outExpression;
    int err;
    te_expr *n = te_compile(expression, expFunctions, expFunctionCount, &err);
    double r = -1;
    if (n) {
      r = te_eval(n);
      outExpression = String(r, 5);
    } else {
      debugPrintln("Error in expression '" + inExpression + "'", debugPrintType::Error);
    }
    te_free(n);
    return outExpression;
}

/// Parses out a commandItem objects arguments with a expression parser using a list of variables

#endif // EXPRESSIONPARSER_C
