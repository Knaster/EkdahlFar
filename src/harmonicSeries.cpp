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
                dump += ":" + String(series[i].frequency[j], 4);
            }
            dump += ",";
        }
        return dump;
    }
};

#endif
