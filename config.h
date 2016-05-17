// our oscillator configurations
#pragma config FNOSC    = PRIPLL                        // Oscillator Selection
#pragma config POSCMOD  = XT //HS                       // Primary Oscillator
#pragma config FPLLIDIV = DIV_2                         // PLL input divider
#pragma config FPLLMUL  = MUL_16 //MUL_16               // PLL multiplier
#pragma config FPLLODIV = DIV_1                         // PLL output divider
#pragma config FPBDIV   = DIV_1                         // Peripheral bus clock divider
#pragma config FSOSCEN  = OFF                           // Secondary Oscillator Enable

// Clock control settings
#pragma config IESO     = OFF   //ON                    // Internal/External Switch-over
#pragma config FCKSM    = CSDCMD                        // Clock Switching & Fail Safe Clock Monitor
#pragma config OSCIOFNC = OFF                           // CLKO Enable

// Code Protection settings
#pragma config CP       = OFF                           // Code Protect
#pragma config BWP      = OFF                           // Boot Flash Write Protect
#pragma config PWP      = OFF                           // Program Flash Write Protect

// for USB
#pragma config UPLLEN = ON          // USB clock uses PLL
#pragma config UPLLIDIV = DIV_2     // Divide 8 MHz input by 2, mult by 12 for 48 MHz

#define MAC1    0xA7B5
#define MAC2    0xFFC
#define MAC3    0x50BE
#define MAC4    0x3CEE

