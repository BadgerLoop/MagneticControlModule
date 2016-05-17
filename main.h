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