#include <xc.h>

// For Shield
#define RED1        LATAbits.LATA3
#define RED2        LATAbits.LATA2
#define YELLOW1     LATCbits.LATC4
#define YELLOW2     LATDbits.LATD4
#define GREEN1      LATDbits.LATD3
#define GREEN2      LATDbits.LATD12
#define BUTTONPWR   LATGbits.LATG1
#define BUTTONSIG   PORTGbits.RG0

void initLEDs(void);
int readButton(void);
