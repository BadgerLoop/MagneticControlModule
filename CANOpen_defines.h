#include <stdbool.h>
#define CPA_TO_KVAO_PA_TO_KVA1(pa)	((void *) ((pa) | 0xa0000000))
#define CAN_REG(base, offset) (*((volatile uint32_t *) ((base) + _CAN1_BASE_ADDRESS + (offset))))
    #define C_CON        0x000                         /* Control Register */
    #define C_CFG        0x010                         /* Baud Rate Configuration Register */
    #define C_INT        0x020                         /* Interrupt Register */
    #define C_VEC        0x030                         /* Interrupt Code Register */
    #define C_TREC       0x040                         /* Transmit/Receive Error Counter Register */
    #define C_FSTAT      0x050                         /* FIFO Status  Register */
    #define C_RXOVF      0x060                         /* Receive FIFO Overflow Status Register */
    #define C_TMR        0x070                         /* CAN Timer Register */
    #define C_RXM        0x080 /*  + (0..3 x 0x10)      //Acceptance Filter Mask Register */
    #define C_FLTCON     0x0C0 /*  + (0..7 x 0x10)      //Filter Control Register */
    #define C_RXF        0x140 /*  + (0..31 x 0x10)     //Acceptance Filter Register */
    #define C_FIFOBA     0x340                         /* Message Buffer Base Address Register */
    #define C_FIFOCON    0x350 /*  + (0..31 x 0x40)     //FIFO Control Register */
    #define C_FIFOINT    0x360 /*  + (0..31 x 0x40)     //FIFO Interrupt Register */
    #define C_FIFOUA     0x370 /*  + (0..31 x 0x40)     //FIFO User Address Register */
    #define C_FIFOCI     0x380 /*  + (0..31 x 0x40)     //Module Message Index Register */
/* Indexes for CANopenNode message objects ************************************/
    /*#ifdef ODL_consumerHeartbeatTime_arrayLength
        #define CO_NO_HB_CONS   ODL_consumerHeartbeatTime_arrayLength
    #else
        #define CO_NO_HB_CONS   0
    #endif */

    #define CO_RXCAN_NMT       0                                      /*  index for NMT message */
    #define CO_RXCAN_SYNC      1                                      /*  index for SYNC message */
    #define CO_RXCAN_RPDO     (CO_RXCAN_SYNC+CO_NO_SYNC)              /*  start index for RPDO messages */
    #define CO_RXCAN_SDO_SRV  (CO_RXCAN_RPDO+CO_NO_RPDO)              /*  start index for SDO server message (request) */
    #define CO_RXCAN_SDO_CLI  (CO_RXCAN_SDO_SRV+CO_NO_SDO_SERVER)     /*  start index for SDO client message (response) */
    #define CO_RXCAN_CONS_HB  (CO_RXCAN_SDO_CLI+CO_NO_SDO_CLIENT)     /*  start index for Heartbeat Consumer messages */
    /* total number of received CAN messages */
    #define CO_RXCAN_NO_MSGS (1+CO_NO_SYNC+CO_NO_RPDO+CO_NO_SDO_SERVER+CO_NO_SDO_CLIENT+CO_NO_HB_CONS)

    #define CO_TXCAN_NMT       0                                      /*  index for NMT master message */
    #define CO_TXCAN_SYNC      CO_TXCAN_NMT+CO_NO_NMT_MASTER          /*  index for SYNC message */
    #define CO_TXCAN_EMERG    (CO_TXCAN_SYNC+CO_NO_SYNC)              /*  index for Emergency message */
    #define CO_TXCAN_TPDO     (CO_TXCAN_EMERG+CO_NO_EMERGENCY)        /*  start index for TPDO messages */
    #define CO_TXCAN_SDO_SRV  (CO_TXCAN_TPDO+CO_NO_TPDO)              /*  start index for SDO server message (response) */
    #define CO_TXCAN_SDO_CLI  (CO_TXCAN_SDO_SRV+CO_NO_SDO_SERVER)     /*  start index for SDO client message (request) */
    #define CO_TXCAN_HB       (CO_TXCAN_SDO_CLI+CO_NO_SDO_CLIENT)     /*  index for Heartbeat message */
    /* total number of transmitted CAN messages */
    #define CO_TXCAN_NO_MSGS (CO_NO_NMT_MASTER+CO_NO_SYNC+CO_NO_EMERGENCY+CO_NO_TPDO+CO_NO_SDO_SERVER+CO_NO_SDO_CLIENT+1)
/* Structure contains timing coefficients for CAN module.
 *
 * CAN baud rate is calculated from following equations:
 * Fsys                                - System clock (MAX 80MHz for PIC32MX)
 * TQ = 2 * BRP / Fsys                 - Time Quanta
 * BaudRate = 1 / (TQ * K)             - Can bus Baud Rate
 * K = SJW + PROP + PhSeg1 + PhSeg2    - Number of Time Quantas
 */
typedef struct{
    uint8_t   BRP;      /* (1...64) Baud Rate Prescaler */
    uint8_t   SJW;      /* (1...4) SJW time */
    uint8_t   PROP;     /* (1...8) PROP time */
    uint8_t   phSeg1;   /* (1...8) Phase Segment 1 time */
    uint8_t   phSeg2;   /* (1...8) Phase Segment 2 time */
}CO_CANbitRateData_t;

/* CAN receive message structure as aligned in CAN module. */
typedef struct{
    unsigned    ident    :11;   /* Standard Identifier */
    unsigned    FILHIT   :5;    /* Filter hit, see PIC32MX documentation */
    unsigned    CMSGTS   :16;   /* CAN message timestamp, see PIC32MX documentation */
    unsigned    DLC      :4;    /* Data length code (bits 0...3) */
    unsigned             :5;
    unsigned    RTR      :1;    /* Remote Transmission Request bit */
    unsigned             :22;
    uint8_t     data[8];        /* 8 data bytes */
}CO_CANrxMsg_t;

/* Received message object */
typedef struct{
    uint16_t            ident;
    uint16_t            mask;
    void               *object;
    void              (*pFunct)(void *object, const CO_CANrxMsg_t *message);
}CO_CANrx_t;

/* Transmit message object. */
typedef struct{
    uint32_t            CMSGSID;     /* Equal to register in transmit message buffer. Includes standard Identifier */
    uint32_t            CMSGEID;     /* Equal to register in transmit message buffer. Includes data length code and RTR */
    uint8_t             data[8];
    volatile bool     bufferFull;
    volatile bool     syncFlag;
}CO_CANtx_t;


/* CAN module object. */
typedef struct{
    uint16_t            CANbaseAddress;
    CO_CANrxMsg_t       CANmsgBuff[33]; /* PIC32 specific: CAN message buffer for CAN module. 32 buffers for receive, 1 buffer for transmit */
    uint8_t             CANmsgBuffSize; /* PIC32 specific: Size of the above buffer == 33. Take care initial value! */
    CO_CANrx_t         *rxArray;
    uint16_t            rxSize;
    CO_CANtx_t         *txArray;
    uint16_t            txSize;
    volatile bool     CANnormal;
    volatile bool     useCANrxFilters;
    volatile bool     bufferInhibitFlag;
    volatile bool     firstCANtxMessage;
    volatile uint16_t   CANtxCount;
    uint32_t            errOld;
    void               *em;
}CO_CANmodule_t;