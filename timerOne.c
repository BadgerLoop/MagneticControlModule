#include "timerOne.h"

void delay(int ms) {
    volatile int i = 0;
    for (i = 0; i < ms; i++) {
        TMR1 = 0;
        while (TMR1 < DELAY1MS);
    }
}

void delay1us(int us) {
    volatile int i = 0;
    for (i = 0; i < us; i++) {
        TMR1 = 0;
        while(TMR1 < DELAY1US);
    }
}

void initTimer1(void) {
    TMR1 = 0;
    T1CON = T1CONSETTING;
    PR1 = 0xffff;
}
