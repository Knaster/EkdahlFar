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
#ifndef AUDIOANALYZE_H
#define AUDIOANALYZE_H

#include <Audio.h>
//#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <usb_audio.h>
#include <AudioStream.h>

AudioInputAnalog          *audioADC;//adc1(AMUX_DATA);           //xy=317,326
AudioAnalyzeNoteFrequency *audioNoteFreq;//notefreq2;      //xy=549,350
AudioAnalyzePeak          *audioPeak;
AudioAnalyzeRMS           *audioRMS;

AudioConnection           *audioPatchCord1;//patchCord1(adc1, notefreq2);
AudioConnection           *audioPatchCord2;
AudioConnection           *audioPatchCord3;
AudioConnection           *audioPatchCord4;

AudioOutputUSB            *audioOutUSB;

AudioFilterBiquad         *audioFilterBiquad;

#define noteFreqCutoff 800

bool audioAnalyzeStarted = false;

void startAudioAnalyze() {
    audioADC = new AudioInputAnalog(22);
    audioFilterBiquad = new AudioFilterBiquad();
    audioFilterBiquad->setLowpass(0, noteFreqCutoff, 0.707);
#ifndef USB_MIDI_AUDIO_SERIAL
    audioNoteFreq = new AudioAnalyzeNoteFrequency();
    audioPeak = new AudioAnalyzePeak();
    audioRMS = new AudioAnalyzeRMS();
#endif

#ifdef USB_MIDI_AUDIO_SERIAL
    audioOutUSB = new AudioOutputUSB();
#endif
#ifndef USB_MIDI_AUDIO_SERIAL
    audioPatchCord3 = new AudioConnection(*audioADC, *audioFilterBiquad);
    audioPatchCord1 = new AudioConnection(*audioFilterBiquad, *audioNoteFreq);
    audioPatchCord2 = new AudioConnection(*audioFilterBiquad, *audioPeak);
    audioPatchCord4 = new AudioConnection(*audioADC, *audioRMS);
//    audioPatchCord1 = new AudioConnection(*audioADC, *audioNoteFreq);
//    audioPatchCord2 = new AudioConnection(*audioADC, *audioPeak);
#endif
#ifdef USB_MIDI_AUDIO_SERIAL
//    audioPatchCord3 = new AudioConnection(*audioADC, *audioOutUSB);
    audioPatchCord3 = new AudioConnection(*audioADC, *audioFilterBiquad);
    audioPatchCord4 = new AudioConnection(*audioFilterBiquad, *audioOutUSB);
#endif
    AudioMemory(30);

#ifndef USB_MIDI_AUDIO_SERIAL
    audioAnalyzeStarted = true;
    audioNoteFreq->begin(0.45);
#endif
}
/*
void stopAudioAnalyze() {
    delete audioPatchCord1;
    delete audioNoteFreq;
    delete audioPatchCord2;
    delete audioPeak;
    delete audioADC;
    audioAnalyzeStarted = false;
}
*/
bool audioFrequencyAvaliable() {
    if (!audioAnalyzeStarted) { return false; }
    return audioNoteFreq->available();
}

float audioFrequency() {
    if (!audioAnalyzeStarted || (audioNoteFreq->probability() < 0.9)) { return -1; }
    return audioNoteFreq->read();
}

float audioPeakAmplitude() {
    if (audioPeak->available()) {
        return audioPeak->read();
    } else {
        return -1;
    }
}

float audioRMSAmplitude() {
    if (audioRMS->available()) {
        return audioRMS->read();
    } else {
        return -1;
    }
}

/*
float audioProbability() {
    if (!audioAnalyzeStarted) { return -1; }
    //  if (!audioNoteFreq->available()) { return -1; }
    return audioNoteFreq->probability();
}
*/
float audioProcessorUsage() {
    if (!audioAnalyzeStarted) { return -1; }
    return audioNoteFreq->processorUsage();
}

#endif
