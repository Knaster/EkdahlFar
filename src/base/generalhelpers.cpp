#ifndef GENERALHELPERS_CPP
#define GENERALHELPERS_CPP

#include <generalhelpers.hpp>

String delimitExpression(String expression, bool force) {
    char delimit = '-';

    if (expression == "") { return expression; }

    if (((expression[0] == "'") or (expression[0] == "\"")) and (expression[0] == expression[expression.length() - 1])) {
        return expression;
    }

    int singleQ = expression.indexOf("'");
    int doubleQ = expression.indexOf("\"");

    if (singleQ < doubleQ) {
        delimit = '\'';
    } else if (singleQ > doubleQ) {
        delimit = '"';
    }

    if (!((singleQ == 0) || (doubleQ == 0))) {
        if (force) { delimit = '\''; } else { delimit = '-'; }
    }
    if (delimit != '-') { expression = delimit + expression + delimit; }
    return expression;
}

String stripQuotes(String expression) {
    expression.trim();

    if ((String(expression[0]) == "\"") || (String(expression[0]) == "'")) {
        if (expression[0] != expression[expression.length()-1]) {
            return expression;
        }
        expression = expression.substring(1, expression.length() - 1);
    }

    return expression;
}

String removePrefix(String input, String prefix) {
    prefix.replace(".", "");

    if (input.substring(0, prefix.length()) == prefix) {
        input = input.substring(prefix.length());
    }

    return input;
}

bool validateNumber(int16_t number, int16_t min, int16_t max, bool t_supressError) {
    if ((number < min) || (number > max)) {
        if (!t_supressError) { debugPrintln("Number out of bounds", Error); }
        return false;
    }
    return true;
}

#endif
