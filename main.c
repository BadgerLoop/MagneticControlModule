#include <xc.h>
#include <sys/kmem.h>
#include "main.h"
#include <stdbool.h>
#include "CANOpen_defines.h"

static volatile unsigned int fifos[(FIFO_0_SIZE + FIFO_1_SIZE) * MB_SIZE];
int errors = 0;
bool board1 = false;
bool board2 = false;

void initializers(void) {
    DDPCONbits.JTAGEN = 0;
    initLEDs();
    initTimer1();
    beginSerial();
    createCANModuleVar();
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
static CO_CANmodule_t *canModule;

void createCANModuleVar(void)
{
    unsigned int canModuleMmap[sizeof(CO_CANmodule_t)];
    canModule = canModuleMmap;
    updateCANModuleVar();
}

void updateCANModuleVar(void) {
    //CO_CANrxMsg_t *rxMessageBuffer = malloc(sizeof(CO_CANrxMsg_t *));
    
    canModule->CANbaseAddress = 0;
    //canModule->CANmsgBuff = ?;
    canModule->CANmsgBuffSize = 33;/* PIC32 specific: Size of the above buffer == 33. Take care initial value! */
    //canModule->rxArray = (CO_CANrxMsg_t*) CO_PA_TO_KVA1(CAN_REG(CANmodule->CANbaseAddress, C_FIFOUA));
    //canModule->rxSize = ?
    canModule->txArray = CO_PA_TO_KVA1(CAN_REG(0, C_FIFOUA+0x40));
    canModule->txSize = sizeof(canModule->txArray);
    canModule->CANnormal = checkCANError();
    canModule->useCANrxFilters = false;
    canModule->bufferInhibitFlag = false;
    canModule->firstCANtxMessage = false;
    canModule->CANtxCount = heartbeatsTotal + 0;
    canModule->errOld = 0;
    //canModule->em = ?
}

void canHeartbeat(void) {
    char buffer[100];
    unsigned int *addr;
    CO_CANtx_t *txBuffer = NULL;
    YELLOW1 = 1;
    delay(500);
    /* The heartbeat message is identified by a 
     * CAN-ID of 0x700 + the node ID, where the 
     * first data byte is equal to 1110.*/ 
    int heartbeat = 0; //binary - 1110 0000 0000 0000
    
    //txBuffer = PA_TO_KVA1(C1FIFOUA1);
    //addr = PA_TO_KVA1(C1FIFOUA1);
    //txBuffer  = PA_TO_KVA1(C1FIFOUA1);
    txBuffer = &canModule->txArray[0];
    switch (EMAC1SA0) {
        case MAC1:
            println("MAC1");
            //C1RXF0bits.SID = 0x701 & 0x07FF;
            //addr[0] = 0x701;
            //addr[0] = 0x701 & 0x07FF;
            txBuffer->CMSGSID = 0x701 & 0x07FF; // Heartbeat | NodeID = 1
            break;
        case MAC2:
            println("MAC2");
            txBuffer->CMSGSID = 0x702 & 0x07FF; // Heartbeat | NodeID = 2
            //C1RXF0bits.SID = 0x146;
            //addr[0] = 0x702;
            break;
        default:
            println("MAC Address did not match.");
            break;
    }
    //addr[1] = 1; // only DLC field must be set, 4 bytes
    //addr[2] = 0b1110;
    unsigned int rtr = 1;
    unsigned int dlc = 1;
    txBuffer->CMSGEID = (dlc & 0xF) | (rtr?0x0200:0); //set DLC and RTR
    // Set first byte of data field to 0b1110
    // in accordance with CANOpen protocol specification
    txBuffer->data[0] = 0b1110;
    txBuffer->syncFlag = 1; // */
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
    
    // {4,   TQ_x_16},   /*CAN=500kbps*/
    // {2,   TQ_x_16}    /*CAN=1000kbps*/
    /* Structure contains timing coefficients for CAN module.
     * // TQ_x_16   1, 5, 8, 2  // good timing
     * CAN baud rate is calculated from following equations:
     * Fsys                                - System clock (MAX 80MHz for PIC32MX)
     * TQ = 2 * BRP / Fsys                 - Time Quanta
     * BaudRate = 1 / (TQ * K)             - Can bus Baud Rate
     * K = SJW + PROP + PhSeg1 + PhSeg2    - Number of Time Quantas
     */
    
    //Set bitrate to 500kbps
    C1CFGbits.BRP = 7; //BRP
    C1CFGbits.PRSEG = 2; //PROP
    C1CFGbits.SEG1PH = 1; //PhSeg1
    C1CFGbits.SEG2PH = 1; //PhSeg2
    C1CFGbits.SJW = 0; //SJW */
    
    //Set Initial SID to 0x146; just to give SID not default value
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
    
    C1CONbits.REQOP = 0;                    // request normal mode
    while (C1CONbits.OPMOD != 0);           // wait for normal mode
    
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

void receiveCANMessage() {
    char buffer[100];
    unsigned int *addr;
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
}
/* Controls team TODOs are below */

/* function to be called asynchronously using interrupts */
void canAsync(void) {
    //TODO: Complete async interrupt call implementation
}

/* function to be called every 1ms using interrupts */
void can1000ms(void) {
    //TODO: Complete interrupt call implementation
    updateCANModuleVar();
    canHeartbeat();
}

#ifndef USE_EXTERNAL_TIMER_1MS_INTERRUPT
unsigned int counter100ms = 0;
unsigned int counter1000ms = 0;
void __ISR(_TIMER_2_VECTOR, IPL3SOFT) TimerInterruptHandler(void){
    //Increment timer counters
    counter100ms++;
    counter1000ms++;
    
    //100ms timer
    if (counter100ms == 100)
    {
        //Put functions to be called every 100ms here
        counter100ms = 0;
    }
    //1000ms timer
    if (counter1000ms == 1000)
    {
        //Put functions to be called every 1000ms here
        can1000ms();
        counter1000ms = 0;
    }
    
}

int main(void) {
    initializers();
    canInit();
    printMAC();
    while (1) {
        updateCANModuleVar();
        switch (EMAC1SA0) {
            case MAC1:
                canHeartbeat();
                break;
            case MAC2:
                receiveCANMessage();
                break;
            default:
                break;
        }
    }
}