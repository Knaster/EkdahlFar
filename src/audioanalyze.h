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
AudioConnection           *audioPatchCord1;//patchCord1(adc1, notefreq2);
AudioConnection           *audioPatchCord2;
AudioConnection           *audioPatchCord3;
AudioOutputUSB            *audioOutUSB;

AudioFilterBiquad         *audioFilterBiquad;
AudioConnection           *audioPatchCord4;

bool audioAnalyzeStarted = false;

void startAudioAnalyze() {
    audioADC = new AudioInputAnalog(22);
    audioFilterBiquad = new AudioFilterBiquad();
    audioFilterBiquad->setLowpass(0, 800, 0.707);
#ifndef USB_MIDI_AUDIO_SERIAL
    audioNoteFreq = new AudioAnalyzeNoteFrequency();
    audioPeak = new AudioAnalyzePeak();
#endif

#ifdef USB_MIDI_AUDIO_SERIAL
    audioOutUSB = new AudioOutputUSB();
#endif
#ifndef USB_MIDI_AUDIO_SERIAL
    audioPatchCord3 = new AudioConnection(*audioADC, *audioFilterBiquad);
    audioPatchCord1 = new AudioConnection(*audioFilterBiquad, *audioNoteFreq);
    audioPatchCord2 = new AudioConnection(*audioFilterBiquad, *audioPeak);
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

void stopAudioAnalyze() {
    delete audioPatchCord1;
    delete audioNoteFreq;
    delete audioPatchCord2;
    delete audioPeak;
    delete audioADC;
    audioAnalyzeStarted = false;
}

bool audioFrequencyAvaliable() {
    if (!audioAnalyzeStarted) { return false; }
    return audioNoteFreq->available();
}

float audioFrequency() {
    if (!audioAnalyzeStarted || (audioNoteFreq->probability() < 0.9)) { return -1; }
    return audioNoteFreq->read();
}

float audioProbability() {
    if (!audioAnalyzeStarted) { return -1; }
    //  if (!audioNoteFreq->available()) { return -1; }
    return audioNoteFreq->probability();
}

float audioProcessorUsage() {
    if (!audioAnalyzeStarted) { return -1; }
    return audioNoteFreq->processorUsage();
}

#endif
