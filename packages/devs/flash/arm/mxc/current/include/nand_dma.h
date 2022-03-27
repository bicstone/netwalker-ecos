#ifndef _NAND_DMA_H
#define _NAND_DMA_H

#include <cyg/io/nand_dma_descriptor.h>


typedef union
{
    unsigned int  U;
    struct
    {
        unsigned COMMAND          :  2;
        unsigned CHAIN            :  1;
        unsigned IRQONCMPLT       :  1;
        unsigned NANDLOCK         :  1;
        unsigned NANDWAIT4READY   :  1;
        unsigned SEMAPHORE        :  1;
        unsigned WAIT4ENDCMD      :  1;
        unsigned HALTONTERMINATE  :  1;
        unsigned RSVD1            :  3;
        unsigned CMDWORDS         :  4;
        unsigned XFER_COUNT       : 16;
    } B;
} hw_apbh_chn_cmd_t;

typedef union
{
    unsigned int  U;
    struct
    {
        unsigned XFER_COUNT         : 16;
        unsigned ADDRESS_INCREMENT  :  1;
        unsigned ADDRESS            :  3;
        unsigned CS                 :  2;
        unsigned LOCK_CS            :  1;
        unsigned WORD_LENGTH        :  1;
        unsigned COMMAND_MODE       :  2;
        unsigned UDMA               :  1;
        unsigned TIMEOUT_IRQ_EN     :  1;
        unsigned DEV_IRQ_EN         :  1;
        unsigned RUN                :  1;
        unsigned CLKGATE            :  1;
        unsigned SFTRST             :  1;
    } B;
} hw_gpmi_ctrl0_t;



//! This structure packs the ReadID result into 6 bytes.
typedef struct _ReadIDCode{
  union{
      struct {
          unsigned char btManufacturerCode;
          unsigned char btDeviceCode;          
      }Device_Code;      
      unsigned short usDeviceID;
  }DeviceID_Code;
   // Read ID Byte 3
  unsigned int InternalChipNumber     : 2;        //!> Number of die = (1 << n)
  unsigned int CellType               : 2;        //!> Number of bits per memory cell = ( 1 << (n+1) )
  unsigned int VendorSpecific0        : 3;
  unsigned int CacheProgram           : 1;        //!> 0 == Not supported
    // Read ID Byte 4
  unsigned int PageSize               : 2;        //!> Page size in bytes = (1 << n) * 1024
  unsigned int RedundantAreaSize      : 1;        //!> Redundant area bytes per 512 data bytes = 8 * (1 << n)
  unsigned int Reserved0              : 1;
  unsigned int BlockSize              : 2;        //!> Block size in bytes = 64 * 1024 * (1 << n)
  unsigned int Organization           : 1;        //!> 0 == x8, 1 == x16
  unsigned int SamsungHSSerialAccess  : 1;        //!> 0 == 50/30ns, 1 == 25ns
  // Read ID Byte 5
  unsigned int VendorSpecific1        : 2;
  unsigned int PlaneNumber            : 2;        //!> # of planes total (see note below) = (1 << n)
  unsigned int PlaneSize              : 3;        //!> # of bytes per plane = 64 * 1024 * 1024 * (1 << n)
  unsigned int Reserved4              : 1;
    // Read ID Byte 6
  unsigned int Reserved5              : 3;
  unsigned int ToshibaHighSpeedMode   : 1;        //!> 0 == Not supported
  unsigned int Reserved6              : 4;
} ReadIDCode;

////////////////////////////////////////////////////////////////////////////////
// APBH Definitions
////////////////////////////////////////////////////////////////////////////////
typedef struct _dma_cmd
{
    struct _dma_cmd    *pNxt;
    hw_apbh_chn_cmd_t   cmd;
    void                *pBuf;
    hw_gpmi_ctrl0_t     ctrl;
    hw_gpmi_compare_t   cmp;
} dma_cmd_t;


///////////////////////////////////////////////////////////////////
//! \name APBH DMA Structure Definitions
//@{
//////////////////////////////////////////////////////////////////

typedef struct _NAND_Timing
{        
    unsigned char m_u8DataSetup;
    unsigned char m_u8DataHold;
    unsigned char m_u8AddressSetup;
    unsigned char m_u8DSAMPLE_TIME;
} NAND_Timing_t;


//! Define the APBH DMA structure without GPMI transfers.
typedef struct _apbh_dma_t
{
    struct _apbh_dma_t*   nxt;
    hw_apbh_chn_cmd_t     cmd;
    void*                 bar;
} apbh_dma_t;

//! Define the APBH DMA structure with 1 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi1_t
{
    struct _apbh_dma_gpmi1_t*	nxt;
    hw_apbh_chn_cmd_t           cmd;
    void*                       bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t       gpmi_ctrl0;
        }apbh_dma_gpmi1_ctrl;
        unsigned int                   pio[1];
    }apbh_dma_gpmi1_u;
} apbh_dma_gpmi1_t;

//! Define the APBH DMA structure with 2 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi2_t
{
    struct _apbh_dma_gpmi2_t*     nxt;
    hw_apbh_chn_cmd_t             cmd;
    void*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t       gpmi_ctrl0;
            hw_gpmi_compare_t     gpmi_compare;
        };
        unsigned int                   pio[2];
    };
} apbh_dma_gpmi2_t;


//! Define the APBH DMA structure with 3 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi3_t
{
    struct _apbh_dma_gpmi1_t*	  nxt;
    hw_apbh_chn_cmd_t             cmd;
    void*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t       gpmi_ctrl0;
            hw_gpmi_compare_t     gpmi_compare;
            hw_gpmi_eccctrl_t     gpmi_eccctrl;
        }apbh_dma_gpmi3_ctrl;
        unsigned int                   pio[3];
    }apbh_dma_gpmi3_u;
} apbh_dma_gpmi3_t;

//! Define the APBH DMA structure with 4 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi4_t
{
    struct _apbh_dma_gpmi1_t*	  nxt;
    hw_apbh_chn_cmd_t             cmd;
    void*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t       gpmi_ctrl0;
            hw_gpmi_compare_t     gpmi_compare;
            hw_gpmi_eccctrl_t     gpmi_eccctrl;
            hw_gpmi_ecccount_t    gpmi_ecccount;
        };
        unsigned int                   pio[4];
    };
} apbh_dma_gpmi4_t;

//! Define the APBH DMA structure with 5 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi5_t
{
    struct _apbh_dma_gpmi1_t*	  nxt;
    hw_apbh_chn_cmd_t             cmd;
    void*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t       gpmi_ctrl0;
            hw_gpmi_compare_t     gpmi_compare;
            hw_gpmi_eccctrl_t     gpmi_eccctrl;
            hw_gpmi_ecccount_t    gpmi_ecccount;
            hw_gpmi_payload_t     gpmi_payload;
        };
        unsigned int                   pio[5];
    };
} apbh_dma_gpmi5_t;

//! Define the APBH DMA structure with 6 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi6_t
{
    struct _apbh_dma_gpmi1_t*	  nxt;
    hw_apbh_chn_cmd_t             cmd;
    void*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t       gpmi_ctrl0;
            hw_gpmi_compare_t     gpmi_compare;
            hw_gpmi_eccctrl_t     gpmi_eccctrl;
            hw_gpmi_ecccount_t    gpmi_ecccount;
            hw_gpmi_payload_t     gpmi_payload;
            hw_gpmi_auxiliary_t   gpmi_auxiliary;
        }apbh_dma_gpmi6_ctrl;
        unsigned int                   pio[6];
    }apbh_dma_gpmi6_u;
} apbh_dma_gpmi6_t;

//@}

////////////////////////////////////////////////////////////////////////////////
//! \name NAND Addressing Structures
////////////////////////////////////////////////////////////////////////////////
#define MAX_COLUMNS     2       //!< Current NANDs only use 2 bytes for column.
#define MAX_ROWS        3       //!< Current NANDs use a max of 3 bytes for row.

//! CLE command plus up to Max Columns and Max Rows.
#define CLE1_MAX_SIZE   MAX_COLUMNS+MAX_ROWS+1

//! This structure defines the packed nature of NAND Column bytes.
typedef struct _NAND_col_t
{
    unsigned char byte1;
    unsigned char byte2;
} NAND_col_t;

//! This structure defines the packed nature of NAND Row bytes.
typedef struct _NAND_row_t
{
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
    unsigned char byte4;
} NAND_row_t;

//! This structure defines the NAND Address bytes defined as Column and Row.
typedef struct _NAND_address_t
{
    NAND_col_t col;
    NAND_row_t row;
} NAND_address_t;

//@}

///////////////////////////////////////////////////////////////////
//!  \name   DMA Chain Structure Definitions
//////////////////////////////////////////////////////////////////

//! Number of commands sent for a NAND Device Reset.
#define NAND_RESET_DEVICE_SIZE  1
//! Number of commands sent for a NAND Device Read ID.
#define NAND_READ_ID_SIZE   2
//! Number of commands read for a NAND Device Read ID.
#define NAND_READ_ID_RESULT_SIZE  6    // Reading 6 bytes back.
//! Number of commands sent to read NAND Device Status.
#define NAND_READ_STATUS_SIZE  1
//! Number of commands read for a NAND Device Status result.
#define NAND_READ_STATUS_RESULT_SIZE  1

//! \brief DMA Descriptor structure for a NAND Reset transaction.
//!
//! This structure defines the DMA chains required to send a Device 
//! Reset command to the NAND.  The following chains are required:
//!     Wait for Ready - waits for the device to enter the Ready state.
//!     Sense Ready - Sense if the device is ready, if not branch to error routine.
//!     Transmit Reset Command - Send reset command to the NAND.
//!     Wait for Ready - waits for the device to enter the Ready state.
//!     Sense Ready - Sense if the device is ready, if not branch to error routine.
typedef struct _NAND_dma_reset_device_t
{
    // descriptor sequence
    apbh_dma_gpmi1_t  wait4rdy_dma;
    apbh_dma_gpmi1_t  sense_rdy_dma;
    apbh_dma_gpmi3_t  tx_dma;
    apbh_dma_gpmi1_t  wait_dma;
    apbh_dma_gpmi1_t  sense_dma;

    // terminator functions
    apbh_dma_t        success_dma;
    apbh_dma_t        timeout_dma;

    // Buffer for Reset Command.
    unsigned char  tx_reset_command_buf[NAND_RESET_DEVICE_SIZE];
} NAND_dma_reset_device_t;

//! \brief DMA Descriptor structure for a NAND Read ID transaction.
//!
//! This structure defines the DMA chains required to send a Device 
//! Read ID command to the NAND.  The following chains are required:
//!     Transmit Read ID Command - Send Read ID command to the NAND.
//!     Read ID Command - Read ID bytes out of the NAND.
typedef struct _NAND_dma_read_id_t
{
    // descriptor sequence
    apbh_dma_gpmi1_t  wait4rdy_dma;
    apbh_dma_gpmi1_t  sense_rdy_dma;
    // now get data.
    apbh_dma_gpmi3_t  tx_dma;
    apbh_dma_gpmi1_t  rx_dma;

    // terminator functions
    apbh_dma_t        success_dma;
    apbh_dma_t        timeout_dma;

    union
    {
        // Buffer for Reset Command.
        unsigned char  tx_readid_command_buf[NAND_READ_ID_SIZE];
        struct
        {
            unsigned char  txCLEByte;
            unsigned char  txALEByte;
        }txCLE_txALE;
    }dma_read_id_buffer;

} NAND_dma_read_id_t ;

typedef struct _NAND_ECC_Params_t
{
    unsigned int u32ECCEngine;                   //!< ECC8 or BCH 
    unsigned int u32EccType;                     //!< Type of ECC - 4 or 8
    unsigned int m_u32EccBlock0Size;            //!< Number of bytes for Block0 - BCH
    unsigned int m_u32EccBlockNSize;            //!< Block size in bytes for all blocks other than Block0 - BCH
    unsigned int m_u32EccBlock0EccLevel;        //!< Ecc level for Block 0 - BCH
    unsigned int m_u32NumEccBlocksPerPage;      //!< Number of blocks per page - BCH
    unsigned int m_u32MetadataBytes;            //!< Metadata size - BCH
    unsigned int m_u32PageSize;                 //!< Size of page including redundant area
    unsigned int m_u32EraseThreshold;           //!< To set into BCH_MODE register.
} NAND_ECC_Params_t;

//! \brief Seed structure with values required for proper read.
//!
//! Dma seed structure for Read Page
//! Use data from this structure to fill in the DMA chain.
typedef struct _NAND_read_seed_t
{
    // Number of Column & Row bytes to be sent.
    unsigned int    uiAddressSize;
    // How many bytes of data do we want to read back?
    unsigned int    uiReadSize;
    // What is the word size 16 or 8 bits?
    unsigned int    uiWordSize;
    // How many chunks of 512 bytes should be ECCed?
    unsigned int    uiECCMask;
    // Enable or Disable ECC
    bool        bEnableHWECC;
    // nand ecc parameters
    //unsigned int    uiNumChunks;
    // What is the word size 16 or 8 bits?
    NAND_ECC_Params_t zNANDEccParams;
    // buffer for 'tx_cle1_addr_dma'
    union
    {
        // 1 byte CLE, up to 5 bytes of Column & Row.
        unsigned char  tx_cle1_addr_buf[MAX_COLUMNS+MAX_ROWS+1];
        struct
        {
            unsigned char tx_cle1;
            union
            {
                unsigned char tx_addr[MAX_COLUMNS+MAX_ROWS];
                // Type2 array has 2 Columns & 3 Rows.
                struct
                {
                    unsigned char  bType2Columns[MAX_COLUMNS];
                    unsigned char  bType2Rows[MAX_ROWS];
                }tx_cle1_Type2;
                // Type1 array has 1 Column & up to 3 Rows
                struct
                {
                    unsigned char  bType1Columns[1];
                    unsigned char  bType1Rows[MAX_ROWS];
                }tx_cle1_Type1;
            }tx_cle1_Columns_Rows;            
        }tx_cle1_addr_Columns_Rows;
    }tx_cle1_addr_dma_buffer;

    // buffer for 'tx_cle2_dma'
    union
    {
        unsigned char  tx_cle2_buf[1];
        struct
        {
            unsigned char tx_cle2;
        }tx_cle2_addr_dma;
    }tx_cle2_addr_dma_buffer;

    // Buffer pointer for data
    void * pDataBuffer;
    // Buffer pointer for Auxiliary data (Redundant and ECC)..
    void * pAuxBuffer;

} NAND_read_seed_t;

//! \brief DMA Descriptor structure for a NAND Read transaction.
//!
//! This structure defines the DMA chains required to send a Device 
//! Read sequence to the NAND.  The following chains are required:
//!     Transmit Read Command and Address - Send primary command and address to the NAND.
//!     Transmit Read2 Command - Send second read command to the NAND.
//!     Wait for Ready - waits for the device to enter the Ready state.
//!     Sense Ready - Sense if the device is ready, if not branch to error routine.
//!     Receive Data - Read the data page from the NAND.
//!     Disable ECC - Disable the ECC DMA (flow-through DMA on Encore).
typedef struct _NAND_dma_read_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t  tx_cle1_addr_dma;
    apbh_dma_gpmi1_t  tx_cle2_dma;
    apbh_dma_gpmi1_t  wait_dma;
    apbh_dma_gpmi1_t  sense_dma;
    apbh_dma_gpmi1_t  wait_dma1;
    apbh_dma_gpmi1_t  sense_dma1;
    apbh_dma_gpmi6_t  rx_data_dma;
    apbh_dma_gpmi3_t  rx_wait4done_dma;

    // terminator functions
    apbh_dma_t        success_dma;
    apbh_dma_t        timeout_dma;

    // Add the DMA Read Seed into the Read DMA structure.
    NAND_read_seed_t  NAND_DMA_Read_Seed;

} NAND_dma_read_t;

//! \brief Seed structure with values required for proper read.
//!
//! Dma seed structure for Read Page
//! Use data from this structure to fill in the DMA chain above.
typedef struct _NandDmaProgSize_t
{
    // Number of Column & Row bytes to be sent.
    unsigned int    uiAddressSize;
    // How many bytes do we want to write to NAND?
    unsigned int    uiWriteSize;
    // What is the word size 16 or 8 bits?
    unsigned int    uiWordSize;
    // buffer for 'tx_cle1_addr_dma'
} NandDmaProgSize_t;

//! \brief Seed structure with values required for NAND writes.
//!
//! Dma seed structure for Write Page
//! Use data from this structure to fill in the DMA chain.
typedef struct _NandDmaProgSeed_t
{
    // Command 1 along with address.
    union
    {
        // 1 byte CLE, up to 4 bytes of Column & Row.
        unsigned char  tx_cle1_addr_buf[MAX_COLUMNS+MAX_ROWS+1];
        struct
        {
            unsigned char tx_cle1;
            union
            {
                unsigned char tx_addr[MAX_COLUMNS+MAX_ROWS];
                // Type2 array has 2 Columns & 2 Rows.
                struct
                {
                    unsigned char  bType2Columns[MAX_COLUMNS];
                    unsigned char  bType2Rows[MAX_ROWS];
                };
                // Type1 array has 1 Column & up to 3 Rows
                struct
                {
                    unsigned char  bType1Columns[1];
                    unsigned char  bType1Rows[MAX_ROWS];
                };
            };            
        };
    };

    // buffer for 'tx_cle2_dma'
    union
    {
        unsigned char  tx_cle2_buf[1];
        struct
        {
            unsigned char tx_cle2;
        };
    };

    NandDmaProgSize_t   NandSizeVars;

    // Status variables for testing success of program.
    unsigned char  u8StatusCmd;
    unsigned short u16Status;
    unsigned int u32StatusMaskRef;
    bool bEnableHWECC;
    NAND_ECC_Params_t zNANDEccParams;
} NandDmaProgSeed_t;

//! \brief DMA Descriptor structure for a NAND Program transaction.
//!
//! This structure defines the DMA chains required to send a Device 
//! Program sequence to the NAND.  The following chains are required:
//!     Transmit Program1 Command and Address - Send primary command and address to the NAND.
//!     Transmit Data - Send the data page to the NAND.
//!     Transmit Program2 Command - Send second program command to the NAND.
//!     Wait for Ready - waits for the device to enter the Ready state.
//!     Sense Ready - Sense if the device is ready, if not branch to error routine.
//!  \todo - It would be best to send a Read Status command and check the result but
//!    this is much lower priority because the ROM will not perform any writes.
//!  \todo - This does not support ECC and is only working on the dillo.
typedef struct _NAND_dma_program_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t  tx_cle1_addr_dma;
    apbh_dma_gpmi6_t  tx_data_dma;
//    apbh_dma_t        tx_auxdata_dma;
    apbh_dma_gpmi3_t  tx_cle2_dma;
    apbh_dma_gpmi1_t  wait_dma;
    apbh_dma_t        sense_dma;
    // CheckStatus.
    apbh_dma_gpmi3_t  statustx_dma;
    apbh_dma_gpmi2_t  statusrx_dma;
    //apbh_dma_gpmi2_t  statcmp_dma;
    apbh_dma_t        statbranch_dma;

    // terminator functions
    apbh_dma_t        success_dma;
    apbh_dma_t        program_failed_dma;

    // The buffers needed by the DMA.
    NandDmaProgSeed_t NandProgSeed;

} NAND_dma_program_t;

//! \brief Seed structure with values required for NAND writes.
//!
//! Dma seed structure for Write Page
//! Use data from this structure to fill in the DMA chain.
typedef struct _NAND_dma_block_erase_seed_t
{
    // Number of Block bytes to be sent.
    unsigned int    uiBlockAddressBytes;

    // buffer for 'tx_cle1_row_dma'
    union
    {
        // CLE + Maximum Block bytes to send.
        unsigned char  tx_cle1_block_buf[MAX_ROWS+1];
        struct
        {
            unsigned char tx_cle1;
            unsigned char tx_block[MAX_ROWS];
        };
    };

    // buffer for 'tx_cle2_dma'
    union
    {
        unsigned char  tx_cle2_buf[1];
        struct
        {
            unsigned char tx_cle2;
        };
    };

    // Status variables for testing success of erase.
    unsigned char  u8StatusCmd;
    unsigned short u16Status;
    unsigned int u32StatusMaskRef;
} NAND_dma_block_erase_seed_t;

//! \brief DMA Descriptor structure for a NAND Erase Block transaction.
//!
//! This structure defines the DMA chains required to send a Device 
//! Erase sequence to the NAND.  The following chains are required:
//!     Transmit Erase1 Command and Address - Send primary command and address to the NAND.
//!     Transmit Erase2 Command - Send second erase command to the NAND.
//!     Wait for Ready - waits for the device to enter the Ready state.
//!     Sense Ready - Sense if the device is ready, if not branch to error routine.
//!  \todo - It would be best to send a Read Status command and check the result but
//!    this is much lower priority because the ROM will not perform any writes.
typedef struct _NAND_dma_block_erase_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t  tx_cle1_row_dma;
    apbh_dma_gpmi1_t  tx_cle2_dma;
    apbh_dma_gpmi1_t  wait_dma;
    apbh_dma_gpmi1_t  sense_dma;

    // CheckStatus.
    apbh_dma_gpmi1_t  statustx_dma;
    apbh_dma_gpmi1_t  statusrx_dma;
    //apbh_dma_gpmi2_t  statcmp_dma;
    apbh_dma_t        statbranch_dma;

    // terminator functions
    apbh_dma_t        success_dma;
    apbh_dma_t        program_failed_dma;

    NAND_dma_block_erase_seed_t NandEraseSeed;
} NAND_dma_block_erase_t;

// dma chain structure for NAND Read Status.
typedef struct _NAND_dma_read_status_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t  tx_dma;
    apbh_dma_gpmi1_t  rx_dma;

    // terminator functions
    apbh_dma_t        success_dma;
    
    // Read Status Size.
    unsigned int    uiReadStatusSize;
    // Read Status Result Size.
    unsigned int    uiReadStatusResultSize;
    // Command word for Read Status
    unsigned char tx_cle1;
    // buffer for resulting Status.
    unsigned char rx_Result;

} NAND_dma_read_status_t;

// dma chain structure for NAND Check Status.
typedef struct _NAND_dma_check_status_t
{
    // descriptor sequence
    apbh_dma_gpmi1_t  tx_dma;
    apbh_dma_gpmi2_t  cmp_dma;
    apbh_dma_t        branch_dma;

    // terminator functions
    apbh_dma_t        success_dma;
    apbh_dma_t        timeout_dma;

    // Read Status Size.
    unsigned int    uiReadStatusSize;
    // buffer for 'tx_cle1'
    unsigned char tx_cle1[2];
} NAND_dma_check_status_t;

//@}

////////////////////////////////////////////////////////////////////////////////
// NAND Definitions
////////////////////////////////////////////////////////////////////////////////

//! Defines the number of microseconds to wait before declaring a timeout.  This
//! should equate to ~12msec.
#define MAX_TRANSACTION_TIMEOUT             12000  //!< Maximum time for DMA is 12msec

//! Defines the bit offset of the NAND APBH Channel 0 in the APBH structure.  This
//! value is used to mask the correct NAND channels.
#define NAND0_APBH_CH            4  //!< Define starting bit for NANDs in APBH.

//! Define size of metadata in redundant area for 4 bit ECC
#define NAND_METADATA_SIZE_4BIT             19
//! Define size of metadata in redundant area for 8 bit ECC
#define NAND_METADATA_SIZE_8BIT             65

////////////////////////////////////////////////////////////////////////////////
// NAND GPMI CTRL0 Definitions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Externs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////

void NAND_HAL_Delay(unsigned int usec);

void  rom_nand_hal_BuildResetDma(NAND_dma_reset_device_t* pChain, 
      unsigned int u32NandDeviceNumber);

void  rom_nand_hal_BuildReadIdDma(NAND_dma_read_id_t* pChain, 
      unsigned int u32NandDeviceNumber, void*  pReadIDBuffer);

void  rom_nand_hal_BuildReadDma(NAND_dma_read_t* pChain, unsigned int u32NandDeviceNumber, 
      NAND_read_seed_t * pReadSeed);

void  rom_nand_hal_BuildQuickReadDma(NAND_dma_read_t* pChain, 
                                unsigned int u32NandDeviceNumber, 
                                NAND_read_seed_t * pReadSeed);

void  rom_nand_hal_BuildProgramDma(NAND_dma_program_t* pChain, unsigned int u32NandDeviceNumber,
                             unsigned int u32AddressSize, unsigned int u32DataSize,
                             unsigned int u32EccSize, void* pWriteBuffer, 
                             void* pAuxBuffer);

void  rom_nand_hal_BuildEraseDma(NAND_dma_block_erase_t* pChain, 
      unsigned int u32BlockAddressBytes, unsigned int u32NandDeviceNumber);

void  rom_nand_hal_BuildReadStatusDma(NAND_dma_read_status_t* pChain, 
      unsigned int u32NandDeviceNumber, void* pStatusBuffer);

void  rom_nand_hal_BuildCheckStatusDma(NAND_dma_check_status_t* pChain, 
      unsigned int u32NandDeviceNumber, unsigned short u16Mask, unsigned short u16Match);

//RtStatus_t rom_nand_hal_GetDmaStatus(unsigned int u32NandDeviceNumber,
//                                     unsigned int u32StartTime, 
//                                     unsigned int u32DmaTimeout);


#endif // _NAND_DMA_H
