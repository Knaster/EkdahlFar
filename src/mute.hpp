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
#ifndef MUTE_H
#define MUTE_H

class Mute {
private:
    uint16_t lastPosition;

    uint16_t restPosition = 0;
    uint16_t fullMutePosition = 65535;
    uint16_t halfMutePosition = 0;

    bool sustain = false;

    uint16_t muteMaxPosition = 59000;

    uint16_t backOffTime = 0;
    elapsedMillis backOffTimer;

public:
    uint16_t hwMutePos = 0;
    uint16_t hwLastMutePos = 0;

    enum emutePosition {mpUndefined, mpRest, mpFull, mpHalf };
    emutePosition mutePosition = mpUndefined;

    Tmc2209ServoStepper *tmc2209ServoStepper = nullptr;

public:
    Mute(char stepEnPin, char stepDirPin, char stepStepPin, HardwareSerial *stepSerialPort, char stepHomeSensorPin);

    bool setupTMC2209();
    void getTMC2209Info();
    bool homeMute(bool invert = false);

    bool setPosition(uint16_t tilt);
    uint16_t getPosition();

    bool rest();
    bool fullMute();
    bool halfMute();

    bool setRestPosition(uint16_t inRestPosition);
    uint16_t getRestPosition();
    bool setFullMutePosition(uint16_t inFullMutePosition);
    uint16_t getFullMutePosition();
    bool setHalfMutePosition(uint16_t inHalfMutePosition);
    uint16_t getHalfMutePosition();
    bool setBackOffTime(uint16_t inBackoffTime);
    uint16_t getBackOffTime();

    bool saveRest();
    bool saveFullMute();
    bool saveHalfMute();

    bool setSustain(bool inSustain);
    bool getSustain();

    void updateMute();
};
#endif
