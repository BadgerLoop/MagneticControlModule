#include "ledShield.h"

void initLEDs(void) {
    // set pin directions
    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 0;
    TRISCbits.TRISC4 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD12 = 0;
    TRISGbits.TRISG1 = 0;
    TRISGbits.TRISG0 = 1;
    
    TRISAbits.TRISA4 = 0;   // for NeoPixels
            
            
    RED1 = 0;
    RED2 = 0;
    YELLOW1 = 0;
    YELLOW2 = 0;
    GREEN1 = 0;
    GREEN2 = 0;
    BUTTONPWR = 1;
}

void changeLED(int LED, int status) {
    if (!(status == 1 || status == 0)) return;
    switch (LED) {
        case 1:
            RED1 = status;
            break;
        case 2:
            RED2 = status;
            break;
        case 3:
            YELLOW1 = status;
            break;
        case 4:
            YELLOW2 = status;
            break;
        case 5:
            GREEN1 = status;
            break;
        case 6:
            GREEN2 = status;
            break;
        default: return; 
    }
}

int readButton() {
    if (!BUTTONSIG) return 1;
    return 0;
}