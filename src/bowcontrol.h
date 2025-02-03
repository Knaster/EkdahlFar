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
#ifndef BOWCONTROL_H
#define BOWCONTROL_H

/// Sets the current bow mode
enum _tiltMode { Rest, Engage, Mute };
enum _speedMode { Automatic, Manual };

class bowControl {
public:
    bowIO *bowIOConnect;                      ///< Pointer to associated bowIO object
    CalibrationData *calibrationDataConnect; ///< Pointer to associated _calibrationData object

    BowActuators *bowActuators;

    float Kp = 500;                           ///< PID P multiplier 100
    float Ki = 7;                             ///< PID I multiplier 2
    float Kd = 200;                            ///< PID D multiplier 40
    float previousError = 0.0;                ///< Contains the last frequency error calculated by the PID, given in Hertz
    float integratorIgnoreBelow = 0.1;        ///< The PID will ignore any integral errors under this threshold 0.4
    uint16_t pidUpdateInterval = 1000;        ///< The interval at which the PID is being called, given in uS
    float pidMaxError = 50;                   ///< Max PID error per loop, essentially sets acceleration

    float KpTerm = 0;                         ///< Calculated P term
    float KiTerm = 0;                         ///< Calculated I term
    float KdTerm = 0;                         ///< Calculated D term
    float integral = 0.0;                     ///< Contains the integral used by the PID
protected:
    float pidTargetSpeed = 0;                 ///< PID target speed

public:
//    elapsedMillis elapsedSinceLastTarget;     ///< Time elapsed since last change in bowing speed or pressure
//    char elapsedTimeThreshold = 40;           ///< Time in ms that elapses in between each bow speed stability check
//    float recoverRate = 0.05;                 ///< Rate at which the Tilt PWM recovers when it has been decreased in order to avoid bow motor stall and oscillation
    bool inRecovery = false;                  ///< Set to true when bow recovery has started
    bool inRelapse = false;                   ///< Set to true when bow recovery has occurred and it has recovered too much, aka triggered bow instability again.
                                        ///< This prevents the bow from recovering any more, preventing future instabilities
private:
    unsigned long previousTime = 0;           ///< This contains the last time stamp that updateString has been processed
    int motorFault = 0;                       ///< Contains the number of motor faults since the last check
    #define stringUpdateInterval = 10000      ///< Interval at which updateString is being processed, given in uS (10ms)

public:
    int run = 0;                          ///< Set to '1' if the bowing wheel is running
    bool PIDon = true;                        ///< Set to true if the PID is turned on

    uint16_t manualTiltPWM = 0;               ///< Manual change to the current Tilt PWM
    uint16_t manualSpeedPWM = 0;              ///< Manual change to the current bowing speed PWM
    uint16_t lastTiltPWM = 0;                 ///< Last Tilt PWM value set, used as a full unsigned 16-bit value

    uint16_t baselineTiltPWM = 0;             ///< The baseline tilt value used when engaging the bow with the string
    uint16_t modifierTiltPWM = 0;             ///< The modifier tilt value used in conjunction with the baselineTiltPWM when engaing the bow with the string
    _tiltMode tiltMode = Rest;

    elapsedMillis elapsedSinceMute;           ///<
    uint16_t mutePeriod = 350;                ///< How long to perform mute action before going back to rest
    uint16_t muteForce = 3000;                ///< Amount of PWM to add to the maximum allowed tilt PWM to accomplish mute

    elapsedMillis bowShutoffTimer;
    uint16_t bowShutoffTimeout = 1000;
    bool bowShutoffTimedout = false;
    bool bowShutoffMotorDisabled = false;
    _speedMode speedMode = Automatic;

//    BufferedOutput *slaveSerialOut = nullptr;
//    SafeStringReader *slaveSerialRead = nullptr;
    harmonicSeries currentHarmonicSeriesData;
    HarmonicSeriesList harmonicSeriesList;
    uint8_t currentHarmonicSeries = 0;

    uint8_t baseNote = 64;

    float bowSpeedToEngage = 10;
    float bowSpeedWhileEngaged = 10;
    bool reachedEngage = false;

    String commandsOverPowerCurrent = "";
    String commandsMotorFault = "";

    bool outputDebugData = true;

private:
    float currentHarmonicFreq = 0;            ///< Base frequency of current harmonic
    float currentHarmonicShiftFreq = 0;       ///< Calculated frequency with harmonic shift
    float currentHarmonicShift5 = 0;
    int harmonicShiftRange = 12;              ///< Harmonic shift range for one 16-bit integer (full range up and down)
    int harmonicShift = 0;                    ///< Harmonic shift given as one 16-bit integer
    int harmonicShift5 = 0;
    int harmonic = 0;                         ///< Current harmonic
    int harmonicAdd = 0;

    bool _hold = false;

    float bowCurrentLimit = 1.5;
    float pidPeakError = 0;

public:
    bowControl(bowIO &_bowIO, CalibrationData &__calibrationData);
    void setBowCurrentLimit(float inBowCurrentLimit);
    float getBowCurrentLimit();
    int isBowStable();

    void pidReset();
    bool setPIDTarget(float _pidTargetSpeed);
    void setPIDTargetUnsafe(float _pidTargetSpeed);
    float getPIDTarget();
    void pidControl();
    void pidInterruptCaller();
    float getPIDPeakError();

    void motorFaultDetected();
    int checkMotorFault();
    void setBowPressureSafe(uint16_t tilt);
    bool calculateBaselineModifierPressure();
    void setPressureBaseline(uint16_t baseline);
    uint16_t getPressureBaseline();
    void setPressureModifier(uint16_t modifier);
    uint16_t getPressureModifier();
    void setBowPower(uint16_t power);
    uint16_t getBowPower();
    bool setManualTilt(uint16_t tilt);
    bool bowRest(int enact);
    bool setHold(bool hold);
    bool bowEngage(int enact);
    bool bowMute(int enact);

    bool calculateHarmonicShift();
    bool setHarmonicShift(int inHarmonicShift);
    bool setHarmonicShift5(int inHarmonicShift5);
    int getHarmonicShift();
    int getHarmonicShift5();
    bool setHarmonicShiftRange(int inHarmonicShiftRange);
    int getHarmonicShiftRange();
    bool setHarmonic(int _harmonic);
    int getHarmonic();

    bool setHarmonicAdd(int _harmonic);
    int getHarmonicAdd();

    bool updateHarmonicData();
    bool setBaseNote(int inBaseNote);

    void measureTimeToTarget(float _pidTargetSpeed);
    String dumpData();
    void updateString();

    bool loadHarmonicSeries(int i);
};

#endif
