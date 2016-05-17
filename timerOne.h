#include <xc.h>

// for calculating delay: Tdelay (seconds) = (Tpb) * DELAY
// shortest delay = 15.625 ns

#define DELAY1MS        64000
#define DELAY1US        64
#define T1CONSETTING    0x8000      // 1:1 prescale

void delay(int ms);
void delay1us(int us);
void initTimer1(void);
