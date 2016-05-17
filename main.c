#include <xc.h>
#include "main.h"

void initializers(void) {
    DDPCONbits.JTAGEN = 0;
    initLEDs();
    initTimer1();
    beginSerial();
}

int getBoardNumber(void) {
    switch(EMAC1SA0) {
        case MAC1: return 1;
        case MAC2: return 2;
        case MAC3: return 3;
        case MAC4: return 4;
        default: return -1;
    }
}

void printMAC(void) {
    sprintf(message, "MAC: %x", EMAC1SA0);
    println(message);
}

void printBoardNumber() {
    sprintf(message, "Board %d connected.", getBoardNumber());
    println(message);
}

int main(void) {
    initializers();
    while (1) {
        
    }
}