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

#include "harmonicSeries.h"

class harmonicSeries {
public:
    std::vector<float> frequency;

    void setHarmonic(uint16_t harmonic, float _frequency) {
    //    debugPrintln("Harmonic " + String(harmonic) +", size " + String(frequency.size()), Debug);
        while (harmonic >= frequency.size()) {
            frequency.push_back(0);
        }
        frequency[harmonic] = _frequency;
    }

    harmonicSeries() {

    }
};

class HarmonicSeriesList {
public:
    std::vector<harmonicSeries> series;

    void addHarmonicSeries() {
        harmonicSeries hs;
        series.push_back(hs);
    }

    void addHarmonicSeries(float frequency[]) {
        harmonicSeries hs;

        for (int i=0; i<12; i++) {
            hs.setHarmonic(i, frequency[i]);
        }

        series.push_back(hs);
    }

    String dumpData() {
        String dump = "";
        for (int i = 0; i < int(series.size()); i++) {
            dump += "bchl:" + String(i);
            for (int j = 0; j < int(series[i].frequency.size()); j++) {
                dump += ":" + String(series[i].frequency[j], 5);
            }
            dump += ",";
        }
        return dump;
    }
};

#endif
