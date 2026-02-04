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
#ifndef HARMONICSERIES_H
#define HARMONICSERIES_H

#include <base/arduinorequired.hpp>
#include <base/module.hpp>

class HarmonicSeries : public Module {
public:
    SETMODULEID("harmonicseries", "hs", "The list of ratios used in the current harmonic series", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(data, HarmonicSeries)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(name, HarmonicSeries)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(ratio, HarmonicSeries)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(remove, HarmonicSeries)

    std::vector<float> ratios;
    String Id;

    void setHarmonic(uint16_t harmonic, float inRatio);
    float getHarmonic(uint16_t harmonic);

    void callDataChanged();

    HarmonicSeries();
private:
    Module *owner;
};

#endif

