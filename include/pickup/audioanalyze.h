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

#include <base/arduinorequired.hpp>
#include <Audio.h>
//#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <usb_audio.h>
#include <AudioStream.h>

#define AUDIO_INTERFACE
//#define AUDIO_BLOCK_SAMPLES 2048

extern AudioAnalyzeNoteFrequency *audioNoteFreq;//notefreq2;      //xy=549,350
extern AudioAnalyzePeak          *audioPeak;
extern AudioAnalyzeRMS           *audioRMS;

void startAudioAnalyze();
bool audioFrequencyAvaliable();
float audioFrequency();
float audioPeakAmplitude();
float audioRMSAmplitude();
float audioProcessorUsage();

#endif
