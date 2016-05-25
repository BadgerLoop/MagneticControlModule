#include <xc.h>
#include <string.h>
#include "config.h"
#include "usbSerial.h"
#include "ledShield.h"
#include "timerOne.h"

#define MAX_LENGTH 255

char message[MAX_LENGTH];

#define SYSCLK      (64000000ul)    // Hz
#define PBCLK       (64000000ul)

#define SID1    0x146       // the SID that this module responds to
#define SID2    0x146    

#define FIFO_0_SIZE     4   // RX, in # message buffers
#define FIFO_1_SIZE     2   // TX, in # message buffers
#define MB_SIZE         4   //