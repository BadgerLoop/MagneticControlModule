#include "usbSerial.h"

void beginSerial() {
    //BREGH = 1;
    U1MODEbits.BRGH = 1; 
    U1BRG = BRATE;
    // 8 bit, no parity, 1 stop bit
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;
    
    // initialize TX and RX
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;
    
    // no hardware flow control
    U1MODEbits.UEN = 0;
    
    U1MODEbits.ON = 1;
    
}

int SerialAvailable() { return U1STAbits.URXDA; }

// Read from UART1 until you get a '\r' or '\n'
void getMessage(char * message, int maxLength) {
    char data = 0;
    int complete = 0, num_bytes = 0;
    // loop until you get a '\r' or '\n'
    while (!complete) {
        if (U1STAbits.URXDA) {
            data = U1RXREG;
            if ((data == '\n') || (data == '\r')) {
                complete = 1;
            } else {
                message[num_bytes] = data;
                ++num_bytes;
                if (num_bytes >= maxLength) {
                    num_bytes = 0;
                }
            }
        }
    }
    message[num_bytes] = '\0';
}

// Write a character array using UART1
void print(const char * string) {
    while (*string != '\0') {
        while (U1STAbits.UTXBF); // wait until tx buffer isn't full
        U1TXREG = *string;
        ++string;
    }
}

void sendByte(uint8_t byte) {
    while (U1STAbits.UTXBF); // wait until tx buffer isn't full
    U1TXREG = byte;
}

void printByte(uint8_t byte) {
    char first = '\0';
    char second = '\0';
    int hex1 = (byte & 0xf0) >> 4;
    int hex2 = byte & 0x0f;
    if (hex1 > 9) {
        switch (hex1) {
            case 10: first = 'A'; break;
            case 11: first = 'B'; break;
            case 12: first = 'C'; break;
            case 13: first = 'D'; break;
            case 14: first = 'E'; break;
            case 15: first = 'F'; break;
            default: first = 'X';
        }
    }
    else first = hex1 + 48;
    if (hex2 > 9) {
        switch(hex2) {
            case 10: second = 'A'; break;
            case 11: second = 'B'; break;
            case 12: second = 'C'; break;
            case 13: second = 'D'; break;
            case 14: second = 'E'; break;
            case 15: second = 'F'; break;
            default: second = 'X';
        }
    }
    else second = hex2 + 48;
    char toSend[] = {'0', 'x', first, second, '\0'};
    print(toSend);
}

// 0x0000
void printWord(uint16_t word) {
    char toSend[5];
    toSend[4] = '\0';
    int i = 0;
    int val;
    for (i = 0; i < 4; i++) {
        val = (word & (0xf << 4*(3 - i))) >> 4*(3 - i);         // dude...
        if (val < 10) toSend[i] = val + 48;
        else {
            switch (val) {
                case 10: toSend[i] = 'A'; break;
                case 11: toSend[i] = 'B'; break;
                case 12: toSend[i] = 'C'; break;
                case 13: toSend[i] = 'D'; break;
                case 14: toSend[i] = 'E'; break;
                case 15: toSend[i] = 'F'; break;
                default: toSend[i] = 'X';
            }
        }
    }
    print("0x");
    print(toSend);
}

// Write a character array using UART1
void println(const char * string) {
    print(string);
    print("\r\n");
}

void checkSerialErrors() {
    if (U1STAbits.PERR) changeLED(6, 1);
    //else changeLED(6, 0);
    if (U1STAbits.FERR) changeLED(5, 1);   
    //else changeLED(5, 0);
    if (U1STAbits.OERR) changeLED(4, 1); 
    //else changeLED(4, 0);
}
