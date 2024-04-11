#include <stdint.h>

#define tachoISR_bowControlArrayMax 8
bowControl* tachoISR_bowControlArray[tachoISR_bowControlArrayMax];
uint8_t tachoISR_bowControlCounter = 0;

void tachoISR_ISR0() { tachoISR_bowControlArray[0]->bowIOConnect->tachoISRHandler(); } // debugPrintln("Tacho ISR 0 triggered", Debug); }
void tachoISR_ISR1() { tachoISR_bowControlArray[1]->bowIOConnect->tachoISRHandler(); } // debugPrintln("Tacho ISR 1 triggered", Debug);}
void tachoISR_ISR2() { tachoISR_bowControlArray[2]->bowIOConnect->tachoISRHandler(); } // debugPrintln("Tacho ISR 2 triggered", Debug);}
void tachoISR_ISR3() { tachoISR_bowControlArray[3]->bowIOConnect->tachoISRHandler(); } // debugPrintln("Tacho ISR 3 triggered", Debug);}
void tachoISR_ISR4() { tachoISR_bowControlArray[4]->bowIOConnect->tachoISRHandler(); } // debugPrintln("Tacho ISR 4 triggered", Debug);}
void tachoISR_ISR5() { tachoISR_bowControlArray[5]->bowIOConnect->tachoISRHandler(); } // debugPrintln("Tacho ISR 5 triggered", Debug);}
void tachoISR_ISR6() { tachoISR_bowControlArray[6]->bowIOConnect->tachoISRHandler(); } // debugPrintln("Tacho ISR 6 triggered", Debug);}
void tachoISR_ISR7() { tachoISR_bowControlArray[7]->bowIOConnect->tachoISRHandler(); } // debugPrintln("Tacho ISR 7 triggered", Debug);}

bool tachoISR_assignInterrupt(uint8_t interruptPin, bowControl *bowControlObject) {
    if (tachoISR_bowControlCounter >= tachoISR_bowControlArrayMax) { return false; }
    tachoISR_bowControlArray[tachoISR_bowControlCounter] = bowControlObject;

    switch(tachoISR_bowControlCounter)  {
    case 0:
        attachInterrupt(digitalPinToInterrupt(interruptPin), tachoISR_ISR0, CHANGE);
        debugPrintln("Assigning ISR0", Debug);
        break;
    case 1:
        attachInterrupt(digitalPinToInterrupt(interruptPin), tachoISR_ISR1, CHANGE);
        break;
    case 2:
        attachInterrupt(digitalPinToInterrupt(interruptPin), tachoISR_ISR2, CHANGE);
        break;
    case 3:
        attachInterrupt(digitalPinToInterrupt(interruptPin), tachoISR_ISR3, CHANGE);
        break;
    case 4:
        attachInterrupt(digitalPinToInterrupt(interruptPin), tachoISR_ISR4, CHANGE);
        break;
    case 5:
        attachInterrupt(digitalPinToInterrupt(interruptPin), tachoISR_ISR5, CHANGE);
        break;
    case 6:
        attachInterrupt(digitalPinToInterrupt(interruptPin), tachoISR_ISR6, CHANGE);
        break;
    case 7:
        attachInterrupt(digitalPinToInterrupt(interruptPin), tachoISR_ISR7, CHANGE);
        break;
    default:
        return false;
}
tachoISR_bowControlCounter++;

return true;
}

#define pidISR_bowControlArrayMax 8
bowControl* pidISR_bowControlArray[pidISR_bowControlArrayMax];
uint8_t pidISR_bowControlCounter = 0;

void pid_ISR0() { pidISR_bowControlArray[0]->pidInterruptCaller(); }
void pid_ISR1() { pidISR_bowControlArray[1]->pidInterruptCaller(); }
void pid_ISR2() { pidISR_bowControlArray[2]->pidInterruptCaller(); }
void pid_ISR3() { pidISR_bowControlArray[3]->pidInterruptCaller(); }
void pid_ISR4() { pidISR_bowControlArray[4]->pidInterruptCaller(); }
void pid_ISR5() { pidISR_bowControlArray[5]->pidInterruptCaller(); }
void pid_ISR6() { pidISR_bowControlArray[6]->pidInterruptCaller(); }
void pid_ISR7() { pidISR_bowControlArray[7]->pidInterruptCaller(); }

IntervalTimer pidInterrupt0;
IntervalTimer pidInterrupt1;
IntervalTimer pidInterrupt2;
IntervalTimer pidInterrupt3;
IntervalTimer pidInterrupt4;
IntervalTimer pidInterrupt5;
IntervalTimer pidInterrupt6;
IntervalTimer pidInterrupt7;

bool pidISR_assignInterrupt(bowControl *bowControlObject) {
  if (pidISR_bowControlCounter >= pidISR_bowControlArrayMax) { return false; }
  pidISR_bowControlArray[pidISR_bowControlCounter] = bowControlObject;

  switch(pidISR_bowControlCounter)  {
    case 0:
      pidInterrupt0.begin(pid_ISR0, bowControlObject->pidUpdateInterval);
      break;
    case 1:
      pidInterrupt1.begin(pid_ISR1, bowControlObject->pidUpdateInterval);
      break;
    case 2:
      pidInterrupt2.begin(pid_ISR2, bowControlObject->pidUpdateInterval);
      break;
    case 3:
      pidInterrupt3.begin(pid_ISR3, bowControlObject->pidUpdateInterval);
      break;
    case 4:
      pidInterrupt4.begin(pid_ISR4, bowControlObject->pidUpdateInterval);
      break;
    case 5:
      pidInterrupt5.begin(pid_ISR5, bowControlObject->pidUpdateInterval);
      break;
    case 6:
      pidInterrupt6.begin(pid_ISR6, bowControlObject->pidUpdateInterval);
      break;
    case 7:
      pidInterrupt7.begin(pid_ISR7, bowControlObject->pidUpdateInterval);
      break;
    default:
      return false;
  }
  pidISR_bowControlCounter++;

  return true;
}

