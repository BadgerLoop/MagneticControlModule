#include <xc.h>
#include <sys/kmem.h>
#include "main.h"
#include "can_structs.h"
#include <stdbool.h>

static volatile unsigned int fifos[(FIFO_0_SIZE + FIFO_1_SIZE) * MB_SIZE];
int errors = 0;
bool board1 = false;
bool board2 = false;

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

int checkCANError(void) {
    int error = 0;
    if (C1TRECbits.TXBO)
    {
        error += 1;
        println("CAN Error in C1TRECbits.TXBO");
    }
    if (C1TRECbits.TXBP)
    {
        error += 2;
        println("CAN Error in C1TRECbits.TXBP");
    }
    if (C1TRECbits.RXBP)
    {
        error += 4;
        println("CAN Error in C1TRECbits.RXBP");
    }
    if (C1TRECbits.TXWARN)
    {
        error += 8;
        println("CAN Error in C1TRECbits.TXWARN");
    }
    if (C1TRECbits.RXWARN)
    {
        error += 16;
        println("CAN Error in C1TRECbits.RXWARN");
    }
    if (C1TRECbits.EWARN)
    {
        error += 32;
        println("CAN Error in C1TRECbits.EWARN");
    }
    return error;
}

static int heartbeatsTotal = 0;
#define CO_PA_TO_KVA1(pa)	((void *) ((pa) | 0xa0000000))
#define CAN_REG(base, offset) (*((volatile uint32_t *) ((base) + _CAN1_BASE_ADDRESS + (offset))))

void canHeartbeat(void) {
    char buffer[100];
    CO_CANtx_t *txBuffer = NULL;
    YELLOW1 = 1;
    delay(500);
    /* The heartbeat message is identified by a 
     * CAN-ID of 0x700 + the node ID, where the 
     * first data byte is equal to 1110.*/ 
    int heartbeat = 0; //binary - 1110 0000 0000 0000
    
    //txBuffer = (CO_CANtx_t *)PA_TO_KVA1(C1FIFOUA1);
    txBuffer = (CO_CANtx_t *)CO_PA_TO_KVA1(CAN_REG(0, 0x370+0x40));
    //txBuffer = &txArray;
    switch (EMAC1SA0) {
        case MAC1:
            println("MAC1");
            //C1RXF0bits.SID = 0x01 & 0x07FF;
            //addr[0] = 0x701; // Heartbeat | NodeID = 1
            txBuffer->CMSGSID = 0x701 & 0x07FF;
            break;
        case MAC2:
            println("MAC2");
            //C1RXF0bits.SID = 0x146;
            //addr[0] = 0x702; // Heartbeat | NodeID = 2
            break;
        default:
            println("MAC Address did not match.");
            break;
    }
    //addr[1] = 8; // only DLC field must be set, 4 bytes
    int rtr = 0;
    txBuffer->CMSGEID = (1 & 0xF) | (rtr?0x0200:0);
    txBuffer->data[0] = 0;
    txBuffer->syncFlag = 0;
    C1FIFOCON1SET = 0x2000;             // setting UINC bit tells fifo to increment counter
    C1FIFOCON1bits.TXREQ = 1;           // request that data from the queue be sent
    errors = checkCANError();
    if (errors != 0)
    {
        sprintf(buffer, "Errors: %d", errors);
        println(buffer);
    } else {
        heartbeatsTotal++;
        sprintf(buffer, "Heartbeat sent | Heartbeats total = %d",
                heartbeatsTotal);
        println(buffer);
        YELLOW1 = 0;
    }
    delay(500);
}

void canInit(void) {
    char buffer[100];
    int to_send = 0;
    unsigned int *addr;
    INTCONbits.MVEC = 1;
    
    C1CONbits.ON = 1;               // turn on the CAN module
    C1CONbits.REQOP = 4;            // request configure mode
    while (C1CONbits.OPMOD != 4);    // wait to enter config mode
    
    C1FIFOCON0bits.FSIZE = FIFO_0_SIZE - 1; // set fifo 0 size. actual size is 1 + FSIZE
    C1FIFOCON0bits.TXEN = 0;                // fifo 0 is an RX fifo
    
    C1FIFOCON1bits.FSIZE = FIFO_1_SIZE - 1;
    C1FIFOCON1bits.TXEN = 1; // fifo 1 is an TX fifo
    
    C1FIFOBA = KVA_TO_PA(fifos);            // tell CAN where the fifos are
    
    C1RXM0bits.SID = 0x000;                 // mask 0 requires all bits to match
    
    C1FLTCON0bits.FSEL0 = 0;                // filter 0 is for FIFO 0
    C1FLTCON0bits.MSEL0 = 0;                // filter 0 uses mask 0
    //C1RXF0bits.SID = MY_SID;                // filter 0 matches against SID
    C1FLTCON0bits.FLTEN0 = 1;               // enable filter 0
    // SKIP BAUD LOOPBACK ONLY
    
    // {2,   TQ_x_16}    /*CAN=1000kbps*/
    /* Structure contains timing coefficients for CAN module.
     * // TQ_x_16   1, 5, 8, 2  // good timing (1000kpbs for 64MHz clock)
     * CAN baud rate is calculated from following equations:
     * Fsys                                - System clock (MAX 80MHz for PIC32MX)
     * TQ = 2 * BRP / Fsys                 - Time Quanta
     * BaudRate = 1 / (TQ * K)             - Can bus Baud Rate
     * K = SJW + PROP + PhSeg1 + PhSeg2    - Number of Time Quantas
     */
    
    //Set bitrate to 1000kbps
    C1CFGbits.BRP = (2 - 1); //BRP
    C1CFGbits.SJW = (1 - 1); //SJW
    C1CFGbits.PRSEG = (5 - 1); //PROP
    C1CFGbits.SEG1PH = (8 - 1); //PhSeg1
    C1CFGbits.SEG2PH = (2 - 1); //PhSeg2 */
    
    //Set Initial SID to 0x146
    C1RXF0bits.SID = SID1;
    
    /* switch (EMAC1SA0) {
        case MAC1:
            println("Board 1 Connected.");
            board1 = true;
            C1RXF0bits.SID = SID1; 
            break;
        case MAC2:
            println("Board 2 Connected.");
            board2 = true;
            C1RXF0bits.SID = SID2;
            break;
        default:
            println("MAC Address did not match.");
            break;
    } */
    
    C1CONbits.REQOP = 0;                    // request loopback mode
    while (C1CONbits.OPMOD != 0);           // wait for loopback mode
    
    /* RED1 = 1;
    while(!readButton());
    RED1 = 0;
    
    //sprintf(buffer, "MAC ADDR: %x", EMAC1SA0);
    //println(buffer);
    //println("Program Start.");
    
    if (board1) {
        while (1) {
            YELLOW2 = 1;
            println("Enter number 1-6:");
            while(!SerialAvailable());
            YELLOW2 = 0;
            getMessage(buffer, 100);
            sscanf(buffer, "%d", &to_send);
            sprintf(buffer, "Sending: %d", to_send);
            println(buffer);
        
            addr = PA_TO_KVA1(C1FIFOUA1);       // get FIFO 1 (TX fifo) current msg addr
            addr[0] = SID2;                   // only the SID set for this example
            addr[1] = sizeof(to_send);          // only DLC field must be set, 4 bytes
            addr[2] = to_send;                  // 4 bytes of actual data
            C1FIFOCON1SET = 0x2000;             // setting UINC bit tells fifo to increment counter
            C1FIFOCON1bits.TXREQ = 1;           // request that data from the queue be sent
            errors = checkCANError();
            sprintf(buffer, "Errors: %d", errors);
            println(buffer);
            //RED2 = 1;
            //while(!C1FIFOINT0bits.RXNEMPTYIF);  // wait to receive data
            //RED2 = 0;
            //addr = PA_TO_KVA1(C1FIFOUA0);       // get the VA of current pointer to RX FIFO
            //sprintf(buffer, "Received %d with SID = 0x%x", addr[2], addr[0] & 0x7FF);
            //println(buffer);
            //C1FIFOCON0SET = 0x2000;             // setting UINC bit tells fifo to inc pointer
        }
    }
    if (board2) {
        while (1) {
            if (C1FIFOINT0bits.RXNEMPTYIF) {
                RED2 = 0;
                addr = PA_TO_KVA1(C1FIFOUA0);       // get the VA of current pointer to RX FIFO
                sprintf(buffer, "Received %d with SID = 0x%x", addr[2], addr[0] & 0x7FF);
                println(buffer);
                changeLED(addr[2], 1);
                delay(1000);
                changeLED(addr[2], 0);
                C1FIFOCON0SET = 0x2000;
            }
            else RED2 = 1;
            GREEN1 = 1;
            delay(1000);
            GREEN1 = 0;
            delay(1000);
            errors = checkCANError();
            if (errors != 0) {
                sprintf(buffer, "Errors: %d", errors);
                println(buffer);
                delay(1000);
            }
            //addr = PA_TO_KVA1(C1FIFOUA1);       // get FIFO 1 (TX fifo) current msg addr
            //addr[0] = SID1;                   // only the SID set for this example
            //addr[1] = sizeof(int);          // only DLC field must be set, 4 bytes
            //addr[2] = 1;                  // 4 bytes of actual data
            //C1FIFOCON1SET = 0x2000;             // setting UINC bit tells fifo to increment counter
            //C1FIFOCON1bits.TXREQ = 1;           // request that data from the queue be sent
        }
    } */
    println("canInit Completed successfully");
    println("Waiting for button press...");
    RED1 = 1;
    while(!readButton());
    RED1 = 0;
}

int main(void) {
    initializers();
    canInit();
    printMAC();
    while (1) {
        canHeartbeat();
    }
}