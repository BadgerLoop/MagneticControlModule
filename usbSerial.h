#include <xc.h>
#include <sys/attribs.h>
#include <stdint.h>
#include <string.h>

// If we want a baud of 115200,

#define BRATE   138

void beginSerial(void);
int SerialAvailable(void);
void getMessage(char * string, int maxLength);
void print(const char * string);
void println(const char * string);
void checkSerialErrors(void);
void printByte(uint8_t byte);
void printWord(uint16_t word);
void sendByte(uint8_t byte);
