#ifndef GENERALHELPERS_HPP
#define GENERALHELPERS_HPP

#include <arduinorequired.hpp>

bool validateNumber(int16_t number, int16_t min, int16_t max, bool t_supressError = false);

String delimitExpression(String expression, bool force = false);
String stripQuotes(String expression);
String removePrefix(String input, String prefix);

#endif // GENERALHELPERS_HPP
