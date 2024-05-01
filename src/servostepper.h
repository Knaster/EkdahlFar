#ifndef SERVOSTEPPER_H
#define SERVOSTEPPER_H

/** \brief The purpose of this class is to control stepper motors in terms of positional indexing much like RC servos
 *
 *  All functions are non-blocking and updatePosition() has to be called continously in order for software and hardware to be updated.
 *
 *  The function completeTask() can be called in order to block execution until the last command has completed or timed out
 *
 *  The whole basis of indexing is in reference to a starting point; the home position. Therefore the first thing to do is to call the
 *  home() function
 *
 */

class servoStepper {
private:
    uint8_t pinStep;
    uint8_t pinDir;
    uint8_t pinHomeSense;
    uint8_t pinVref = -1;
    uint8_t pinEn = -1;
public:
    enum eHomingStage { UNHOMED, HOMED, FIRSTHOMINGRISING, SECONDHOMINGFALLING, SECONDHOMINGRISING, FIRSTHOMINGFALLING, MOVEPASTHOMESWITCH, MOVETOHOMESWITCH, GOTOOFFSET };
    eHomingStage homing = UNHOMED;

    enum eStepDirection {  REVERSE = 1, FORWARD = 0 };
    eStepDirection moveDirection = eStepDirection::REVERSE;

    enum eEdgeType { EDGERISING = 1, EDGEFALLING = 0 };

    uint16_t homingPoint[2][2]; // index by eEdgeType, eStepDirection

    #define stepsPerRevolution 200
    #define microSteps 32
    #define valuePerRotation (stepsPerRevolution * microSteps) / 65535

    #define dirChangeMultiplier 1 //5
    #define homeSenseActive 0
    #define homeSenseInactive 1

//    #define homingSpeedRough 10
//    #define homingSpeedFine 5
    uint8_t homingSpeedRough = 10;
    uint8_t homingSpeedFine = 5;
    eStepDirection homingDirection = eStepDirection::REVERSE;

    eStepDirection homingOffsetDirection = eStepDirection::FORWARD;
    int16_t homingOffset = 0;

    int16_t homingStepsThreshold = (stepsPerRevolution / 2) * microSteps;   // (stepsPerRevolution / 3) * microSteps;

    uint16_t maxSpeed = 20;
    float speedRPM = 10;
    uint16_t speedUPeriod;      // Period in uS for the given RPM, does NOT account for acceleration/decelration

    uint16_t dirChange = 0;     // Indicate whether direction has changed

    elapsedMicros updatePeriod;

    uint16_t currentStep = 0;
    bool homingSensed;
    bool homingSensedChanged;

    bool isMoving = false;
    uint16_t movingTo = 0;      // Current target position

//    int16_t homingSteps = 0;    // Debug variable for measuring accuracy of position

    // params for serial control DRV8825: speed 25, multiplier 0.2, steps 30
    // noiseless params for TMC2209 30 / 0.1
    float accelerationMultiplier = 0.1;     // latest 0.1 -- 0.2 min for serial control @ speed 20
    uint8_t accelerationSteps = 35;            //  30 min for serial control @ speed 20
    int16_t currentAccelStep = 0;
    uint16_t uAccelPeriod = 0;

    bool hasStepped = true;     // Flag indicating that a new step has been issued, used to indicate invocation of calculateAcceleration

    bool turnAround = false;
    uint16_t turnAroundPosition = 0;
    eStepDirection turnAroundDirection = eStepDirection::REVERSE;

    bool invertDirection = false;

    IntervalTimer stepInterval;
    void *stepIntervalCallback;
    //eStepDirection indexingDirection = eStepDirection

    bool autoCorrectPosition = false;

    int stepperID = -1;

    bool reachedTarget = false;

public:
    void calculateAcceleration() {
        int stepsFromEnd = 0;
//        int stepsFromStart = 0;

        if (moveDirection == eStepDirection::FORWARD) {
            stepsFromEnd = movingTo - currentStep;
        } else {
            stepsFromEnd = currentStep - movingTo;
        }

        if (stepsFromEnd <= accelerationSteps) {
            currentAccelStep--;
        } else {
            currentAccelStep++;
        }

        if (currentAccelStep < 0) { currentAccelStep = 0; }
        if (currentAccelStep > accelerationSteps) { currentAccelStep = accelerationSteps; }

        uAccelPeriod = speedUPeriod + speedUPeriod * (accelerationSteps - currentAccelStep) * accelerationMultiplier;
    }

//    servoStepper(byte _pinStep, byte _pinDir, byte _pinHomeSense, byte _pinFault = -1, void (*faultCallback)(void) = NULL, byte _pinEn = -1, byte _pinVref = -1, byte current = 0.2) {
    servoStepper(byte inPinStep, byte inPinDir, byte inPinHomeSense, void *inStepIntervalCallback = nullptr) { //, byte inPinEn = -1) {
        pinStep = inPinStep;
        pinDir = inPinDir;
        pinHomeSense = inPinHomeSense;
//        pinEn = inPinEn;

        pinMode(pinStep, OUTPUT);
        pinMode(pinDir, OUTPUT);
        pinMode(pinHomeSense, INPUT_PULLDOWN);

        digitalWrite(pinStep, 0);
/*
        if (pinEn != -1) {
            pinMode(pinEn, OUTPUT);
            digitalWrite(pinEn, 0);
        }
*/
        stepIntervalCallback = inStepIntervalCallback;

        getHomingChange();
    }

    bool setMaxSpeed(uint16_t inSpeedRPM) {
        maxSpeed = inSpeedRPM;
        return true;
    }

    bool setSpeed(float inSpeedRPM) {
        speedRPM = inSpeedRPM;
        speedUPeriod = 1000000 / (speedRPM * 60) / microSteps;
        return true;
    }

    void setDirection(eStepDirection inDirection) {
        int tDirection = inDirection;
        if (invertDirection) {
            if (tDirection == eStepDirection::FORWARD) {
                tDirection = eStepDirection::REVERSE;
            } else {
                tDirection = eStepDirection::FORWARD;
            }
        }
        digitalWrite(pinDir, tDirection);
        moveDirection = inDirection;
    }

    eStepDirection notDirection(eStepDirection inDirection) {
        if (inDirection == eStepDirection::FORWARD) {
            return eStepDirection::REVERSE;
        } else {
            return eStepDirection::FORWARD;
        }
    }

    bool getHomingChange() {
        byte newValue = digitalRead(pinHomeSense);
        if (homingSensed != newValue) {
            homingSensed = newValue;
            homingSensedChanged = true;
            return true;
        }
        homingSensedChanged = false;
        return false;
    }

    void stepNext(uint16_t period) {
        if (updatePeriod < (period)) { return; }

        digitalWrite(pinStep, 255);
        delayMicroseconds(2);
        digitalWrite(pinStep, 0);

        if (moveDirection == eStepDirection::FORWARD) {
            currentStep++;
        } else {
            currentStep--;
        }

        updatePeriod = 0;
        hasStepped = true;
    }

    void stepNext() {
        stepNext(speedUPeriod);
    }

    eStepDirection getDirectionTo(uint16_t position) {
        if (currentStep < position) {
            return eStepDirection::FORWARD;
        } else
        if (currentStep > position) {
            return eStepDirection::REVERSE;
        } else {
            return eStepDirection::FORWARD;
        }
    }

    void updatePosition() {
        getHomingChange();

        switch(homing) {
            case eHomingStage::HOMED:
                updatePositionMove();
                break;
            case eHomingStage::UNHOMED:
                break;
            case eHomingStage::FIRSTHOMINGRISING:
                updatePositionFirstHomingRising();
                break;
            case eHomingStage::SECONDHOMINGFALLING:
                updatePositionSecondHomingFalling();
                break;
            case eHomingStage::SECONDHOMINGRISING:
                updatePositionSecondHomingRising();
                break;
            case eHomingStage::FIRSTHOMINGFALLING:
                updatePositionFirstHomingFalling();
                break;
            case eHomingStage::MOVEPASTHOMESWITCH:
                updatePositionMovePastHomeSwitch();
                break;
            case eHomingStage::MOVETOHOMESWITCH:
                updatePositionMoveToHomeSwitch();
                break;
            case eHomingStage::GOTOOFFSET:
                updatePositionGotoOffset();
                break;
        }
    }

    void updatePositionMove() {
        if (isMoving) {
            if (autoCorrectPosition) { correctPosition(); }

            if (currentStep == movingTo) {
                if (turnAround) {
                    movingTo = turnAroundPosition;
                    setDirection(turnAroundDirection);
                    turnAround = false;
                } else {
                    reachedTarget = true;
//                    debugPrintln("Setting reached target to true", debugPrintType::Debug);
                    isMoving = false;
                }
            } else {
                if (hasStepped) {
                    calculateAcceleration();
                    hasStepped = false;
                }

                stepNext(uAccelPeriod);
                if (stepIntervalCallback != nullptr) {
                    stepInterval.begin(stepIntervalCallback, ((double) uAccelPeriod));
                }
            }
        }
    }

    void correctPosition() {
        if (homingSensedChanged) {
            //eStepDirection homePointDirection;
            eEdgeType edgeDirection;
            if (homingSensed) {
                //homePointDirection = notDirection(moveDirection);
                edgeDirection = eEdgeType::EDGEFALLING;
            } else {
                //homePointDirection = moveDirection;
                edgeDirection = eEdgeType::EDGERISING;
            }

//            if (currentStep != homingPoint[homePointDirection]) {
            if (currentStep != homingPoint[edgeDirection][moveDirection]) {
                debugPrintln("Got position " + String(currentStep) + " should have position " + String(homingPoint[edgeDirection][moveDirection]) +
                    " edge " + String(edgeDirection) + " direction " + String(moveDirection) + " movingTo " + String(movingTo) + " - Correcting : stepID " + String(stepperID),
                    debugPrintType::Debug);

                // say we're moving to 1800, the step THINKS it is at 1795 but corrects to 1805 and corrects it to there
                // then movingTo will miss the target

                if ((moveDirection == eStepDirection::FORWARD) && (homingPoint[edgeDirection][moveDirection] > movingTo)) {
                    debugPrintln("This should become a problem : stepID "  + String(stepperID), Error);
                    movingTo = homingPoint[edgeDirection][moveDirection];
                }

                if ((moveDirection == eStepDirection::REVERSE) && (homingPoint[edgeDirection][moveDirection] < movingTo)) {
                    debugPrintln("This should become a problem : stepID "  + String(stepperID), Error);
                    movingTo = homingPoint[edgeDirection][moveDirection];
                }

                currentStep = homingPoint[edgeDirection][moveDirection];
            }
        }
    }

    bool setPosition(uint16_t position) {
        uint16_t wantToMoveTo = ((float) valuePerRotation * position);

        if ((wantToMoveTo == movingTo) && (isMoving)) { return true; }

        if (isMoving) {
            if (moveDirection == getDirectionTo(wantToMoveTo)) {
                if (turnAround) {
                }
                turnAround = false;
            } else {
                if (turnAround) {
                }
                turnAroundPosition = wantToMoveTo;
                turnAroundDirection = getDirectionTo(wantToMoveTo);

                if ((!turnAround) && ((currentStep + accelerationSteps) > movingTo)) {
                    if (moveDirection == eStepDirection::FORWARD) {
                        movingTo = currentStep + accelerationSteps;
                    } else {
                        movingTo = currentStep - accelerationSteps;
                    }
                }
                turnAround = true;
                return true;
            }
        } else {
            currentAccelStep = 0;
        }
        movingTo = wantToMoveTo;

        setDirection(getDirectionTo(movingTo));
        isMoving = true;
//        debugPrintln("Setting reached target to false", debugPrintType::Debug);
        reachedTarget = false;
        updatePosition();
        return true;
    }

    void setHomingOffset(uint16_t offset) {
        homingOffset = ((uint32_t) (microSteps * 200) * offset / 65536);
    }

    void eStop() {
        isMoving = false;
        homing = eHomingStage::UNHOMED;
    }

    /*
        Homing sequence:

        1) if homingswitch is not active, go to 2.1
        1.1) if homingswitch is active go in the opposite direction oh homingDirection until the homingswitch is inactive and then continue to 1
        2) move in homingDirection until homingswitch is homeSenseActive
            2.1) if the amount of steps surpass that of homingStepsThreshold, go in the opposite direction of homingDirection until the homingswitch is active and then
                 continue to 1.1
        3) record position as homingPoint:RISING,homingDirection = homingOffset (currentStep = homingOffset)
        4) move forward in homingDirection until homingswitch is homeSenseInactive
        5) record position as homingPoint:FALLING,homingDirection = currentStep
        6) move forward past this point seconHomeOverStep number of steps
        7) move backwards in !homingDirection until homingSwitch is homeSenseActive
        8) record position as homingPoint:RISING,!homingDirection = currentStep
        9) move backwards in !homingDirection until homingSwitch is homeSenseInActive
        10) record position as homingPoint:FALLING,!homingDirection = currentStep
        11) move to offset position
    */

    void updatePositionFirstHomingRising() {
        if (abs(currentStep) > homingStepsThreshold) {
            debugPrintln("Homing past steps threshold, reverting back to switch : stepID " + String(stepperID), debugPrintType::Debug);
            setDirection(notDirection(homingDirection));
            homing = eHomingStage::MOVETOHOMESWITCH;
        }

        if ((homingSensedChanged) && (homingSensed == homeSenseActive)) {
            if (homingOffsetDirection == eStepDirection::FORWARD) {
                currentStep = -homingOffset;
            } else {
                currentStep = homingOffset;
            }
            homingPoint[eEdgeType::EDGERISING][homingDirection] = currentStep;
            debugPrintln("Setting first homing rising point " + String(homingDirection) + " to " + String(homingPoint[eEdgeType::EDGERISING][homingDirection]) +
                " stepID " + String(stepperID), debugPrintType::Debug);

            //setDirection(homingOffsetDirection);
            //homing = eHomingStage::GOTOOFFSET;

            debugPrintln("[firsthomerising] Next direction is " + String(moveDirection) + ", direction inversion is " + String(invertDirection), debugPrintType::Debug);

            homing = eHomingStage::SECONDHOMINGFALLING;
        } else {
            stepNext();
            if (hasStepped) {
//                homingSteps--;
                hasStepped = false;
            }
        }
    }

    bool foundSecondHomeFalling = false;
    uint16_t secondHomeFallingSteps = 0;
    #define secondHomeOverStep 20*32

    void updatePositionSecondHomingFalling() {
        // ?????????????? WTF IS THIS ??????? if statement makes no sense
//        if ((foundSecondHomeFalling) && (abs(currentStep - foundSecondHomeFalling) > secondHomeOverStep)) {   // removed 2024-04-18
        if ((foundSecondHomeFalling) && (currentStep > secondHomeOverStep)) {
            debugPrintln("Second home overstep " + String(secondHomeOverStep), debugPrintType::Debug);
            debugPrintln("Done over stepping, going to next step : stepID " + String(stepperID), debugPrintType::Debug);
            setDirection(notDirection(homingDirection));
            homing = eHomingStage::SECONDHOMINGRISING;
            foundSecondHomeFalling = false; // Added 2024-04-18

            debugPrintln("[secondhomefalling] Next direction is " + String(moveDirection) + ", direction inversion is " + String(invertDirection), debugPrintType::Debug);
        } else
        if ((homingSensedChanged) && (homingSensed == homeSenseInactive)) {
            homingPoint[eEdgeType::EDGEFALLING][homingDirection] = currentStep;
            debugPrintln("Setting second homing falling point " + String(homingDirection) + " to " +
                String(homingPoint[eEdgeType::EDGEFALLING][homingDirection]) + " : stepID " + String(stepperID), debugPrintType::Debug);

            foundSecondHomeFalling = true;
            secondHomeFallingSteps = currentStep;
        } else {
            stepNext();
            if (hasStepped) {
//                homingSteps--;
                hasStepped = false;
            }
        }
    }

    void updatePositionSecondHomingRising() {
        if ((homingSensedChanged) && (homingSensed == homeSenseActive)) {
            homingPoint[eEdgeType::EDGERISING][notDirection(homingDirection)] = currentStep - 1;
            debugPrintln("Setting second homing rising point " + String(notDirection(homingDirection)) + " to " +
                String(homingPoint[eEdgeType::EDGERISING][notDirection(homingDirection)]) + " : stepID " + String(stepperID), debugPrintType::Debug);

            homing = eHomingStage::FIRSTHOMINGFALLING;
        } else {
            stepNext();
            if (hasStepped) {
//                homingSteps--;
                hasStepped = false;
            }
        }
    }

    void updatePositionFirstHomingFalling() {
        if ((homingSensedChanged) && (homingSensed == homeSenseInactive)) {
            homingPoint[eEdgeType::EDGEFALLING][notDirection(homingDirection)] = currentStep - 1;
            debugPrintln("Setting first homing falling point " + String(notDirection(homingDirection)) + " to " +
                String(homingPoint[eEdgeType::EDGEFALLING][notDirection(homingDirection)]) + " : stepID " + String(stepperID), debugPrintType::Debug);

            homing = eHomingStage::GOTOOFFSET;
            movingTo = 0;
            //debugPrintln("Moving to offset " + String(movingTo), debugPrintType::Debug);
        } else {
            stepNext();
            if (hasStepped) {
//                homingSteps--;
                hasStepped = false;
            }
        }
    }

    void updatePositionGotoOffset() {
        if (currentStep == movingTo) {
            isMoving = false;
            homing = eHomingStage::HOMED;
        } else {
            stepNext();
        }
    }

    void updatePositionMovePastHomeSwitch() {
        if ((homingSensedChanged) && (homingSensed == homeSenseInactive)) {
            setDirection(homingDirection);
            homing = eHomingStage::FIRSTHOMINGRISING;
            currentStep = 0;
            debugPrintln("Past homing switch, resuming home : stepID " + String(stepperID), debugPrintType::Debug);
        } else {
            stepNext();
            if (hasStepped) {
//                homingSteps--;
                hasStepped = false;
            }
        }
    }

    void updatePositionMoveToHomeSwitch() {
        if ((homingSensedChanged) && (homingSensed == homeSenseActive)) {
            setDirection(notDirection(homingDirection));
            homing = eHomingStage::MOVEPASTHOMESWITCH;
            debugPrintln("At homing switch, moving past : stepID " + String(stepperID), debugPrintType::Debug);
        } else {
            stepNext();
            if (hasStepped) {
//                homingSteps--;
                hasStepped = false;
            }
        }
    }


    bool home(eStepDirection inHomingDirection = eStepDirection::REVERSE, uint8_t inHomingSpeedRough = 10, uint8_t inHomingSpeedFine = 5,
        eStepDirection inOffsetDirection = eStepDirection::FORWARD) {

        if (isMoving) {
            debugPrintln("Moving!!! Not homing", debugPrintType::Error);
            return false;
        }

        homingDirection = inHomingDirection;
        homingSpeedRough = inHomingSpeedRough;
        homingSpeedFine = inHomingSpeedFine;
        homingOffsetDirection = inOffsetDirection;

        currentStep = 0; // Reset current step

        debugPrintln("Homing direction " + String(homingDirection) + " homing speed r " + String(homingSpeedRough) + " homing speed f " + String(homingSpeedFine) +
            " homing ofs. dir. " + String(homingOffsetDirection), debugPrintType::Debug);

        setSpeed(homingSpeedRough);
        if (homingSensed == homeSenseActive) {
            homing = eHomingStage::MOVEPASTHOMESWITCH;
            setDirection(notDirection(homingDirection));
            debugPrintln("On homing switch, moving : stepID " + String(stepperID), debugPrintType::Debug);
        } else {
            homing = eHomingStage::FIRSTHOMINGRISING;
            setDirection(homingDirection);
        }
        isMoving = true;
//        homingSteps = 0;
        return true;
    }

    bool completeTask(uint16_t timeout = 3000) {
        elapsedMillis timeOutElapse;
        do {
            updatePosition();
        } while ((isMoving) && (timeOutElapse < timeout));
        if (isMoving) {
            eStop();
            return false;
        }
        return true;
    }
};

#endif
