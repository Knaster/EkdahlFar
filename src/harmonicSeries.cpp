/*
 * This file is part of The Ekdahl FAR firmware.
 *
 * The Ekdahl FAR firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Ekdahl FAR firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Ekdahl FAR firmware. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2024 Karl Ekdahl
 */
#ifndef HARMONICSERIES_C
#define HARMONICSERIES_C

#include <WProgram.h>
#include <vector>

#include "harmonicSeries.hpp"

const ModuleCommandDeclaration HarmonicSeries::moduleCommands[] = {
    { "", "", "int", "Sets the current Harmonic Series", false, false, nullptr },
    { "name", "na", "name", "Sets the name of the harmonic series", false, false, &s_name },
    { "data", "da", "name:ratios", "Gets / sets all data for the harmonic series in the given slot", false, true, &s_data },
    { "ratio", "r", "harmonic:ratio", "Sets the ratio of the given harmonic in current harmonic series, will increase the list size if needed to adress the harmonic", false, false, &s_ratio },
    { "remove", "rm", "harmonic", "Remove the harmonic ratio given in the current series and shift any ratios accordingly. Cannot remove all ratios", false, false, &s_remove }
};

getModuleCount(HarmonicSeries)

HarmonicSeries::HarmonicSeries() {
    moduleID = new ModuleID("harmonicseries", "hs", "The list of ratios used in the current harmonic series", eModuleType::software);
}
/*
CREATE_DATACHANGED_CALLBACK(fDataChanged, HarmonicSeries) {
    debugPrintln("Data changed hs!", debugPrintType::Debug);
    return true;
}
*/
CREATE_MODULE_COMMAND_FUNCTION(name, HarmonicSeries) {
    if (request) {
        inCommandResponses->push_back({thisItem.shortCommand + ":" + Id, InfoRequest});
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
        Id = stripQuotes(inCommandItem->argument[0]);
        inCommandResponses->push_back({thisItem.shortCommand + ":" + delimitExpression(Id, true), InfoRequest});
    }
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(data, HarmonicSeries) {
    if (!request) {
        if (!checkArgumentsMin(inCommandItem, inCommandResponses, 1, false)) { return eProcessResult::WrongArgumentCount; }
        ratios.clear();
        for (int i = 0; i < inCommandItem->argument.size() - 1; i++) {
            setHarmonic(i, inCommandItem->argument[i + 1].toFloat());
        }
        callDataChanged();
    }

    String response = thisItem.shortCommand + ":" + delimitExpression(Id, true);
    for (int i = 0; i < ratios.size(); i++) {
        response += ":" + String(ratios[i], 3);
    }

    inCommandResponses->push_back({ response, InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(ratio, HarmonicSeries) {
    if (request) {
        String returnString = thisItem.shortCommand + ":";
        int r = 0;
        if (inCommandItem->argument.size() > 0) {
            r = inCommandItem->argument[0].toInt();
            if (!validateNumber(r, 0, ratios.size() - 1)) { return eProcessResult::WrongArgumentValue; }
            returnString += String(ratios[r]);
        } else {
            for (int i = 0; i < ratios.size(); i++) {
                if (i > 0) { returnString += ":"; }
                returnString += String(ratios[r]);
            }
        }
        inCommandResponses->push_back({returnString, InfoRequest});
    } else {
        if (!checkArguments(inCommandItem, inCommandResponses, 2)) { return eProcessResult::WrongArgumentCount; }
        setHarmonic(inCommandItem->argument[0].toInt(), inCommandItem->argument[1].toFloat());
        callDataChanged();

        inCommandResponses->push_back({thisItem.shortCommand + ":" + String(inCommandItem->argument[0].toInt()) + ":" + String(inCommandItem->argument[1].toFloat(), 3), InfoRequest});
    }
    return eProcessResult::Ok;
};

CREATE_MODULE_COMMAND_FUNCTION(remove, HarmonicSeries) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    if (ratios.size() < 2) {
        inCommandResponses->push_back({"Cannot remove all harmonics", Error});
        return eProcessResult::CommandFailed;
    }

    int i = inCommandItem->argument[0].toInt();
    ratios.erase(ratios.begin() + i);
    callDataChanged();

    inCommandResponses->push_back({thisItem.shortCommand + ":" + String(i), InfoRequest});
    return eProcessResult::Ok;
};

void HarmonicSeries::callDataChanged() {
/*    if (dataChangedCallback != nullptr) {
        dataChangedCallback(owner, this);
    }*/
    setDataChanged();
}

void HarmonicSeries::setHarmonic(uint16_t harmonic, float inRatio) {
    while (harmonic >= ratios.size()) {
        ratios.push_back({float(0)});
    }
    ratios[harmonic] = inRatio;
}

float HarmonicSeries::getHarmonic(uint16_t harmonic) {
    if ((harmonic < 0) || (harmonic >= ratios.size())) {
        return -1;
    }
    return ratios[harmonic];
}

#endif
