#ifndef _NAND_DMA_DESCRIPTOR_H
#define _NAND_DMA_DESCRIPTOR_H

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define REGS_GPMI_BASE 0x8000C000

/*
 * constants & macros for individual HW_APBH_CHn_CMD multi-register bitfields
 */
/* --- Register HW_APBH_CHn_CMD, field XFER_COUNT */

#define BP_APBH_CHn_CMD_XFER_COUNT      16
#define BM_APBH_CHn_CMD_XFER_COUNT      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_APBH_CHn_CMD_XFER_COUNT(v)   ((((unsigned int) v) << 16) & BM_APBH_CHn_CMD_XFER_COUNT)
#else
#define BF_APBH_CHn_CMD_XFER_COUNT(v)   (((v) << 16) & BM_APBH_CHn_CMD_XFER_COUNT)
#endif

/* --- Register HW_APBH_CHn_CMD, field CMDWORDS */

#define BP_APBH_CHn_CMD_CMDWORDS      12
#define BM_APBH_CHn_CMD_CMDWORDS      0x0000F000

#define BF_APBH_CHn_CMD_CMDWORDS(v)   (((v) << 12) & BM_APBH_CHn_CMD_CMDWORDS)

/* --- Register HW_APBH_CHn_CMD, field HALTONTERMINATE */

#define BP_APBH_CHn_CMD_HALTONTERMINATE      8
#define BM_APBH_CHn_CMD_HALTONTERMINATE      0x00000100

#define BF_APBH_CHn_CMD_HALTONTERMINATE(v)   (((v) << 8) & BM_APBH_CHn_CMD_HALTONTERMINATE)

/* --- Register HW_APBH_CHn_CMD, field WAIT4ENDCMD */

#define BP_APBH_CHn_CMD_WAIT4ENDCMD      7
#define BM_APBH_CHn_CMD_WAIT4ENDCMD      0x00000080

#define BF_APBH_CHn_CMD_WAIT4ENDCMD(v)   (((v) << 7) & BM_APBH_CHn_CMD_WAIT4ENDCMD)

/* --- Register HW_APBH_CHn_CMD, field SEMAPHORE */

#define BP_APBH_CHn_CMD_SEMAPHORE      6
#define BM_APBH_CHn_CMD_SEMAPHORE      0x00000040

#define BF_APBH_CHn_CMD_SEMAPHORE(v)   (((v) << 6) & BM_APBH_CHn_CMD_SEMAPHORE)

/* --- Register HW_APBH_CHn_CMD, field NANDWAIT4READY */

#define BP_APBH_CHn_CMD_NANDWAIT4READY      5
#define BM_APBH_CHn_CMD_NANDWAIT4READY      0x00000020

#define BF_APBH_CHn_CMD_NANDWAIT4READY(v)   (((v) << 5) & BM_APBH_CHn_CMD_NANDWAIT4READY)

/* --- Register HW_APBH_CHn_CMD, field NANDLOCK */

#define BP_APBH_CHn_CMD_NANDLOCK      4
#define BM_APBH_CHn_CMD_NANDLOCK      0x00000010

#define BF_APBH_CHn_CMD_NANDLOCK(v)   (((v) << 4) & BM_APBH_CHn_CMD_NANDLOCK)

/* --- Register HW_APBH_CHn_CMD, field IRQONCMPLT */

#define BP_APBH_CHn_CMD_IRQONCMPLT      3
#define BM_APBH_CHn_CMD_IRQONCMPLT      0x00000008

#define BF_APBH_CHn_CMD_IRQONCMPLT(v)   (((v) << 3) & BM_APBH_CHn_CMD_IRQONCMPLT)

/* --- Register HW_APBH_CHn_CMD, field CHAIN */

#define BP_APBH_CHn_CMD_CHAIN      2
#define BM_APBH_CHn_CMD_CHAIN      0x00000004

#define BF_APBH_CHn_CMD_CHAIN(v)   (((v) << 2) & BM_APBH_CHn_CMD_CHAIN)

/* --- Register HW_APBH_CHn_CMD, field COMMAND */

#define BP_APBH_CHn_CMD_COMMAND      0
#define BM_APBH_CHn_CMD_COMMAND      0x00000003

#define BF_APBH_CHn_CMD_COMMAND(v)   (((v) << 0) & BM_APBH_CHn_CMD_COMMAND)



/* --- Register HW_APBH_CHn_CMD, field CHAIN */

#define BP_APBH_CHn_CMD_CHAIN      2
#define BM_APBH_CHn_CMD_CHAIN      0x00000004


/* --- Register HW_APBH_CHn_CMD, field COMMAND */

#define BP_APBH_CHn_CMD_COMMAND      0
#define BM_APBH_CHn_CMD_COMMAND      0x00000003


//
// macros for single instance registers
//

#define BF_SET(reg, field)       HW_##reg##_SET(BM_##reg##_##field)
#define BF_CLR(reg, field)       HW_##reg##_CLR(BM_##reg##_##field)
#define BF_TOG(reg, field)       HW_##reg##_TOG(BM_##reg##_##field)

#define BF_SETV(reg, field, v)   HW_##reg##_SET(BF_##reg##_##field(v))
#define BF_CLRV(reg, field, v)   HW_##reg##_CLR(BF_##reg##_##field(v))
#define BF_TOGV(reg, field, v)   HW_##reg##_TOG(BF_##reg##_##field(v))

#define BV_FLD(reg, field, sym)  BF_##reg##_##field(BV_##reg##_##field##__##sym)
#define BV_VAL(reg, field, sym)  BV_##reg##_##field##__##sym

#define BF_RD(reg, field)        HW_##reg.B.field
#define BF_WR(reg, field, v)     BW_##reg##_##field(v)

#define BF_CS1(reg, f1, v1)  \
        (HW_##reg##_CLR(BM_##reg##_##f1),      \
         HW_##reg##_SET(BF_##reg##_##f1(v1)))

#define BF_CS2(reg, f1, v1, f2, v2)  \
        (HW_##reg##_CLR(BM_##reg##_##f1 |      \
                        BM_##reg##_##f2),      \
         HW_##reg##_SET(BF_##reg##_##f1(v1) |  \
                        BF_##reg##_##f2(v2)))

#define BF_CS3(reg, f1, v1, f2, v2, f3, v3)  \
        (HW_##reg##_CLR(BM_##reg##_##f1 |      \
                        BM_##reg##_##f2 |      \
                        BM_##reg##_##f3),      \
         HW_##reg##_SET(BF_##reg##_##f1(v1) |  \
                        BF_##reg##_##f2(v2) |  \
                        BF_##reg##_##f3(v3)))

#define BF_CS4(reg, f1, v1, f2, v2, f3, v3, f4, v4)  \
        (HW_##reg##_CLR(BM_##reg##_##f1 |      \
                        BM_##reg##_##f2 |      \
                        BM_##reg##_##f3 |      \
                        BM_##reg##_##f4),      \
         HW_##reg##_SET(BF_##reg##_##f1(v1) |  \
                        BF_##reg##_##f2(v2) |  \
                        BF_##reg##_##f3(v3) |  \
                        BF_##reg##_##f4(v4)))

#define BF_CS5(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5)  \
        (HW_##reg##_CLR(BM_##reg##_##f1 |      \
                        BM_##reg##_##f2 |      \
                        BM_##reg##_##f3 |      \
                        BM_##reg##_##f4 |      \
                        BM_##reg##_##f5),      \
         HW_##reg##_SET(BF_##reg##_##f1(v1) |  \
                        BF_##reg##_##f2(v2) |  \
                        BF_##reg##_##f3(v3) |  \
                        BF_##reg##_##f4(v4) |  \
                        BF_##reg##_##f5(v5)))

#define BF_CS6(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6)  \
        (HW_##reg##_CLR(BM_##reg##_##f1 |      \
                        BM_##reg##_##f2 |      \
                        BM_##reg##_##f3 |      \
                        BM_##reg##_##f4 |      \
                        BM_##reg##_##f5 |      \
                        BM_##reg##_##f6),      \
         HW_##reg##_SET(BF_##reg##_##f1(v1) |  \
                        BF_##reg##_##f2(v2) |  \
                        BF_##reg##_##f3(v3) |  \
                        BF_##reg##_##f4(v4) |  \
                        BF_##reg##_##f5(v5) |  \
                        BF_##reg##_##f6(v6)))

#define BF_CS7(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7)  \
        (HW_##reg##_CLR(BM_##reg##_##f1 |      \
                        BM_##reg##_##f2 |      \
                        BM_##reg##_##f3 |      \
                        BM_##reg##_##f4 |      \
                        BM_##reg##_##f5 |      \
                        BM_##reg##_##f6 |      \
                        BM_##reg##_##f7),      \
         HW_##reg##_SET(BF_##reg##_##f1(v1) |  \
                        BF_##reg##_##f2(v2) |  \
                        BF_##reg##_##f3(v3) |  \
                        BF_##reg##_##f4(v4) |  \
                        BF_##reg##_##f5(v5) |  \
                        BF_##reg##_##f6(v6) |  \
                        BF_##reg##_##f7(v7)))

#define BF_CS8(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8)  \
        (HW_##reg##_CLR(BM_##reg##_##f1 |      \
                        BM_##reg##_##f2 |      \
                        BM_##reg##_##f3 |      \
                        BM_##reg##_##f4 |      \
                        BM_##reg##_##f5 |      \
                        BM_##reg##_##f6 |      \
                        BM_##reg##_##f7 |      \
                        BM_##reg##_##f8),      \
         HW_##reg##_SET(BF_##reg##_##f1(v1) |  \
                        BF_##reg##_##f2(v2) |  \
                        BF_##reg##_##f3(v3) |  \
                        BF_##reg##_##f4(v4) |  \
                        BF_##reg##_##f5(v5) |  \
                        BF_##reg##_##f6(v6) |  \
                        BF_##reg##_##f7(v7) |  \
                        BF_##reg##_##f8(v8)))


//
// macros for multiple instance registers
//

#define BF_SETn(reg, n, field)       HW_##reg##_SET(n, BM_##reg##_##field)
#define BF_CLRn(reg, n, field)       HW_##reg##_CLR(n, BM_##reg##_##field)
#define BF_TOGn(reg, n, field)       HW_##reg##_TOG(n, BM_##reg##_##field)

#define BF_SETVn(reg, n, field, v)   HW_##reg##_SET(n, BF_##reg##_##field(v))
#define BF_CLRVn(reg, n, field, v)   HW_##reg##_CLR(n, BF_##reg##_##field(v))
#define BF_TOGVn(reg, n, field, v)   HW_##reg##_TOG(n, BF_##reg##_##field(v))

#define BV_FLDn(reg, n, field, sym)  BF_##reg##_##field(BV_##reg##_##field##__##sym)
#define BV_VALn(reg, n, field, sym)  BV_##reg##_##field##__##sym

#define BF_RDn(reg, n, field)        HW_##reg(n).B.field
#define BF_WRn(reg, n, field, v)     BW_##reg##_##field(n, v)

#define BF_CS1n(reg, n, f1, v1)  \
        (HW_##reg##_CLR(n, (BM_##reg##_##f1)),      \
         HW_##reg##_SET(n, (BF_##reg##_##f1(v1))))

#define BF_CS2n(reg, n, f1, v1, f2, v2)  \
        (HW_##reg##_CLR(n, (BM_##reg##_##f1 |       \
                            BM_##reg##_##f2)),      \
         HW_##reg##_SET(n, (BF_##reg##_##f1(v1) |   \
                            BF_##reg##_##f2(v2))))

#define BF_CS3n(reg, n, f1, v1, f2, v2, f3, v3)  \
        (HW_##reg##_CLR(n, (BM_##reg##_##f1 |       \
                            BM_##reg##_##f2 |       \
                            BM_##reg##_##f3)),      \
         HW_##reg##_SET(n, (BF_##reg##_##f1(v1) |   \
                            BF_##reg##_##f2(v2) |   \
                            BF_##reg##_##f3(v3))))

#define BF_CS4n(reg, n, f1, v1, f2, v2, f3, v3, f4, v4)  \
        (HW_##reg##_CLR(n, (BM_##reg##_##f1 |       \
                            BM_##reg##_##f2 |       \
                            BM_##reg##_##f3 |       \
                            BM_##reg##_##f4)),      \
         HW_##reg##_SET(n, (BF_##reg##_##f1(v1) |   \
                            BF_##reg##_##f2(v2) |   \
                            BF_##reg##_##f3(v3) |   \
                            BF_##reg##_##f4(v4))))

#define BF_CS5n(reg, n, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5)  \
        (HW_##reg##_CLR(n, (BM_##reg##_##f1 |       \
                            BM_##reg##_##f2 |       \
                            BM_##reg##_##f3 |       \
                            BM_##reg##_##f4 |       \
                            BM_##reg##_##f5)),      \
         HW_##reg##_SET(n, (BF_##reg##_##f1(v1) |   \
                            BF_##reg##_##f2(v2) |   \
                            BF_##reg##_##f3(v3) |   \
                            BF_##reg##_##f4(v4) |   \
                            BF_##reg##_##f5(v5))))

#define BF_CS6n(reg, n, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6)  \
        (HW_##reg##_CLR(n, (BM_##reg##_##f1 |       \
                            BM_##reg##_##f2 |       \
                            BM_##reg##_##f3 |       \
                            BM_##reg##_##f4 |       \
                            BM_##reg##_##f5 |       \
                            BM_##reg##_##f6)),      \
         HW_##reg##_SET(n, (BF_##reg##_##f1(v1) |   \
                            BF_##reg##_##f2(v2) |   \
                            BF_##reg##_##f3(v3) |   \
                            BF_##reg##_##f4(v4) |   \
                            BF_##reg##_##f5(v5) |   \
                            BF_##reg##_##f6(v6))))

#define BF_CS7n(reg, n, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7)  \
        (HW_##reg##_CLR(n, (BM_##reg##_##f1 |       \
                            BM_##reg##_##f2 |       \
                            BM_##reg##_##f3 |       \
                            BM_##reg##_##f4 |       \
                            BM_##reg##_##f5 |       \
                            BM_##reg##_##f6 |       \
                            BM_##reg##_##f7)),      \
         HW_##reg##_SET(n, (BF_##reg##_##f1(v1) |   \
                            BF_##reg##_##f2(v2) |   \
                            BF_##reg##_##f3(v3) |   \
                            BF_##reg##_##f4(v4) |   \
                            BF_##reg##_##f5(v5) |   \
                            BF_##reg##_##f6(v6) |   \
                            BF_##reg##_##f7(v7))))

#define BF_CS8n(reg, n, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8)  \
        (HW_##reg##_CLR(n, (BM_##reg##_##f1 |       \
                            BM_##reg##_##f2 |       \
                            BM_##reg##_##f3 |       \
                            BM_##reg##_##f4 |       \
                            BM_##reg##_##f5 |       \
                            BM_##reg##_##f6 |       \
                            BM_##reg##_##f7 |       \
                            BM_##reg##_##f8)),      \
         HW_##reg##_SET(n, (BF_##reg##_##f1(v1) |   \
                            BF_##reg##_##f2(v2) |   \
                            BF_##reg##_##f3(v3) |   \
                            BF_##reg##_##f4(v4) |   \
                            BF_##reg##_##f5(v5) |   \
                            BF_##reg##_##f6(v6) |   \
                            BF_##reg##_##f7(v7) |   \
                            BF_##reg##_##f8(v8))))

#define BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER  0x0
#define BV_APBH_CHn_CMD_COMMAND__DMA_WRITE    0x1
#define BV_APBH_CHn_CMD_COMMAND__DMA_READ     0x2
#define BV_APBH_CHn_CMD_COMMAND__DMA_SENSE    0x3

// Macro/Defines used to create a DMA command word in the chain.

//! \brief APBH DMA Macro for Wait4Ready command.
//!
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Wait for Ready before starting next DMA descriptor in chain.
//! Don't lock the nand while waiting for Ready to go high.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.

#define NAND_DMA_WAIT4RDY_CMD \
    (BF_APBH_CHn_CMD_CMDWORDS(1) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_NANDWAIT4READY(1) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro for Wait4Ready command.
//!
//! Wait for Ready before sending IRQ interrupt.
//! Use 8 bit word length (doesn't really matter since no transfer).
//! Watch u32ChipSelect.
#define NAND_DMA_WAIT4RDY_PIO(u32ChipSelect) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY) | \
     BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect))

//! \brief APBH DMA Macro for Transmit Data command.
//!
//! Transfer TransferSize bytes with DMA.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Lock the NAND while waiting for this DMA chain to complete.
//! Decrement semaphore if this is the last part of the chain.
//! Another descriptor follows this one in the chain.
//! This DMA is a read from System Memory - write to device.
                                    // TGT_3700, TGT_CHIP and others
#define NAND_DMA_TXDATA_CMD(TransferSize,Semaphore,CommandWords,Wait4End) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(CommandWords) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(Wait4End) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro for Transmit Data command.
//!
//! Setup transfer as a write.
//! Transfer NumBitsInWord bits per DMA cycle.
//! Lock CS during this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Data transfer (0b00)
//! Transfer TransferSize - NumBitsInWord values.
#define NAND_DMA_TXDATA_PIO(u32ChipSelect,NumBitsInWord,TransferSize) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) | \
     BF_GPMI_CTRL0_WORD_LENGTH(NumBitsInWord) | \
     BF_GPMI_CTRL0_LOCK_CS(1) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
     BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief APBH DMA Macro for Sense command.
//!
//! Transfer no Bytes with DMA.
//! Transfer no Words to PIO.
//! Don't lock the NAND while waiting for Ready to go high.
//! Decrement semaphore if this is the last part of the chain.
//! Another descriptor follows this one in the chain.
#define NAND_DMA_SENSE_CMD(SenseSemaphore) \
    (BF_APBH_CHn_CMD_CMDWORDS(0) | \
     BF_APBH_CHn_CMD_SEMAPHORE(SenseSemaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_SENSE))

//! \brief APBH DMA Macro for Read Data command.
//!
//! Receive TransferSize bytes with DMA.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Decrement semaphore if this is the last part of the chain.
//! Unlock the NAND after this DMA chain completes.
//! Another descriptor follows this one in the chain.
//! This DMA is a write to System Memory - read from device.
#ifdef TGT_DILLO
#define NAND_DMA_RX_CMD(TransferSize,Semaphore) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(1) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE))
#else
#define NAND_DMA_RX_CMD_NOECC(TransferSize,CmdWords,Semaphore) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(CmdWords) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE))

//! \brief APBH DMA Macro for Read Data command with ECC.
//!
//! Receive TransferSize bytes with DMA.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Decrement semaphore if this is the last part of the chain.
//! Unlock the NAND after this DMA chain completes.
//! Another descriptor follows this one in the chain.
//! No DMA transfer here; the ECC8 block becomes the bus master and
//! performs the memory writes itself instead of the DMA.
#define NAND_DMA_RX_CMD_ECC(TransferSize,Semaphore) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(6) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))
#endif

//! \brief APBH DMA Macro for Recieve Data with no ECC command.
//!
//! Receive TransferSize bytes with DMA but no ECC.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Decrement semaphore if this is the last part of the chain.
//! Unlock the NAND after this DMA chain completes.
//! Another descriptor follows this one in the chain.
//! This DMA is a write to System Memory - read from device.
#define NAND_DMA_RX_NO_ECC_CMD(TransferSize,Semaphore) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(1) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE)) 

//! \brief GPMI PIO DMA Macro for Receive command.
//!
//! Setup transfer as a READ.
//! Transfer NumBitsInWord bits per DMA cycle.
//! Select the appropriate chip.
//! Address lines need to specify Data transfer (0b00)
//! Transfer TransferSize - NumBitsInWord values.
#define NAND_DMA_RX_PIO(u32ChipSelect,NumBitsInWord,TransferSize) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ) | \
     BF_GPMI_CTRL0_WORD_LENGTH(NumBitsInWord) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect) | \
     BF_GPMI_CTRL0_LOCK_CS(0) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
     BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief APBH DMA Macro for sending NAND Command sequence.
//!
//! Transmit TransferSize bytes to DMA.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Decrement semaphore if this is the last part of the chain.
//! Lock the NAND until the next chain.
//! Another descriptor follows this one in the chain.
//! This DMA is a read from System Memory - write to device.
#define NAND_DMA_COMMAND_CMD(TransferSize,Semaphore,NandLock,CmdWords) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(CmdWords) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(NandLock) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ))

//! \brief GPMI PIO DMA Macro when sending a command.
//!
//! Setup transfer as a WRITE.
//! Transfer NumBitsInWord bits per DMA cycle.
//! Lock CS during and after this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Command transfer (0b01)
//! Increment the Address lines if AddrIncr is set.
//! Transfer TransferSize - NumBitsInWord values.
#define NAND_DMA_COMMAND_PIO(u32ChipSelect,TransferSize,AddrInc,AssertCS) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) | \
    BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
    BF_GPMI_CTRL0_LOCK_CS(AssertCS) | \
    BF_GPMI_CTRL0_CS(u32ChipSelect) | \
    BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE) | \
    BF_GPMI_CTRL0_ADDRESS_INCREMENT(AddrInc) | \
    BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief GPMI PIO DMA Macro for disabling ECC during this write.
#define NAND_DMA_ECC_PIO(EnableDisable) \
    (BW_GPMI_ECCCTRL_ENABLE_ECC(EnableDisable))

//! \brief APBH DMA Macro for Sending NAND Address sequence.
//!
//! Setup transfer as a WRITE.
//! Transfer NumBitsInWord bits per DMA cycle.
//! Lock CS during and after this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Address transfer (0b10)
//! Transfer TransferSize - NumBitsInWord values.
#define NAND_DMA_ADDRESS_PIO(u32ChipSelect,TransferSize) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) | \
     BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
     BF_GPMI_CTRL0_LOCK_CS(1) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_ALE) | \
     BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief APBH DMA Macro for NAND Compare sequence
//!
//! Transfer TransferSize Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Lock the NAND until the next chain.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_COMPARE_CMD(TransferSize) \
    (BF_APBH_CHn_CMD_CMDWORDS(TransferSize) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro for NAND Compare sequence
//!
//! Setup transfer as a Read and Compare.
//! Transfer 8 bits per DMA cycle.
//! Lock CS during and after this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Data transfer (0b00)
//! Transfer TransferSize - 8 Bit values.
#define NAND_DMA_COMPARE_PIO(u32ChipSelect,TransferSize) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ_AND_COMPARE) | \
     BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
     BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief APBH DMA Macro for NAND Dummy transfer sequence
//!
//! Dummy Transfer.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Lock the NAND until the next chain.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_DUMMY_TRANSFER \
    (BF_APBH_CHn_CMD_CMDWORDS(0) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro sequence for ECC decode.
//!
//! Setup READ transfer ECC Control register.
//! Setup for ECC Decode, 4 Bit.
//! Enable the ECC block
//! The ECC Buffer Mask determines which fields are corrected.
#define NAND_DMA_ECC_CTRL_PIO(EccBufferMask, decode_encode_size) \
    (BW_GPMI_ECCCTRL_ECC_CMD(decode_encode_size) | \
     BW_GPMI_ECCCTRL_ENABLE_ECC(BV_GPMI_ECCCTRL_ENABLE_ECC__ENABLE) | \
     BW_GPMI_ECCCTRL_BUFFER_MASK(EccBufferMask) )

//! \brief APBH DMA Macro for Disabling the ECC block sequence
//!
//! Disable ECC Block Transfer.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Lock the NAND until the next chain.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_DISABLE_ECC_TRANSFER \
    (BF_APBH_CHn_CMD_CMDWORDS(3) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_NANDWAIT4READY(1) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro sequence for disabling ECC block
//!
//! Setup transfer as a READ.
//! Release CS during and after this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Data transfer
//! Transfer NO data.
#define NAND_DMA_DISABLE_ECC_PIO(u32ChipSelect) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY) | \
    BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
    BF_GPMI_CTRL0_LOCK_CS(0) | \
    BF_GPMI_CTRL0_CS(u32ChipSelect) | \
    BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
    BF_GPMI_CTRL0_ADDRESS_INCREMENT(0) | \
    BF_GPMI_CTRL0_XFER_COUNT(0))

//! \brief APBH DMA Macro for Removing the NAND Lock in APBH.
//!
//! Remove NAND Lock.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Unlock the NAND to allow another NAND to run.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_REMOVE_NAND_LOCK \
    (BF_APBH_CHn_CMD_CMDWORDS(0) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_NANDWAIT4READY(0) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))


#define DECR_SEMAPHORE  1   //!< Decrement DMA semaphore this time.
#define NAND_LOCK       1   //!< Lock the NAND to prevent contention.
#define ASSERT_CS       1   //!< Assert the Chip Select during this operation.
#define ECC_ENCODE      1   //!< Perform an ECC Encode during this operation.
#define ECC_DECODE      0   //!< Perform an ECC Decode during this operation.
#define ECC_4_BIT       0   //!< Perform a 4 bit ECC operation.
#define ECC_8_BIT       1   //!< Perform an 8 bit ECC operation.



/*
 * constants & macros for entire HW_GPMI_CTRL0 register
 */
#define HW_GPMI_CTRL0_ADDR      (REGS_GPMI_BASE + 0x00000000)
#define HW_GPMI_CTRL0_SET_ADDR  (REGS_GPMI_BASE + 0x00000004)
#define HW_GPMI_CTRL0_CLR_ADDR  (REGS_GPMI_BASE + 0x00000008)
#define HW_GPMI_CTRL0_TOG_ADDR  (REGS_GPMI_BASE + 0x0000000C)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_CTRL0           (*(volatile hw_gpmi_ctrl0_t *) HW_GPMI_CTRL0_ADDR)
#define HW_GPMI_CTRL0_RD()      (HW_GPMI_CTRL0.U)
#define HW_GPMI_CTRL0_WR(v)     (HW_GPMI_CTRL0.U = (v))
#define HW_GPMI_CTRL0_SET(v)    ((*(volatile unsigned int *) HW_GPMI_CTRL0_SET_ADDR) = (v))
#define HW_GPMI_CTRL0_CLR(v)    ((*(volatile unsigned int *) HW_GPMI_CTRL0_CLR_ADDR) = (v))
#define HW_GPMI_CTRL0_TOG(v)    ((*(volatile unsigned int *) HW_GPMI_CTRL0_TOG_ADDR) = (v))
#endif


/*
 * constants & macros for individual HW_GPMI_CTRL0 bitfields
 */
/* --- Register HW_GPMI_CTRL0, field SFTRST */

#define BP_GPMI_CTRL0_SFTRST      31
#define BM_GPMI_CTRL0_SFTRST      0x80000000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_CTRL0_SFTRST(v)   ((((unsigned int) v) << 31) & BM_GPMI_CTRL0_SFTRST)
#else
#define BF_GPMI_CTRL0_SFTRST(v)   (((v) << 31) & BM_GPMI_CTRL0_SFTRST)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_SFTRST(v)   BF_CS1(GPMI_CTRL0, SFTRST, v)
#endif

#define BV_GPMI_CTRL0_SFTRST__RUN    0x0
#define BV_GPMI_CTRL0_SFTRST__RESET  0x1

/* --- Register HW_GPMI_CTRL0, field CLKGATE */

#define BP_GPMI_CTRL0_CLKGATE      30
#define BM_GPMI_CTRL0_CLKGATE      0x40000000

#define BF_GPMI_CTRL0_CLKGATE(v)   (((v) << 30) & BM_GPMI_CTRL0_CLKGATE)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_CLKGATE(v)   BF_CS1(GPMI_CTRL0, CLKGATE, v)
#endif

#define BV_GPMI_CTRL0_CLKGATE__RUN      0x0
#define BV_GPMI_CTRL0_CLKGATE__NO_CLKS  0x1

/* --- Register HW_GPMI_CTRL0, field RUN */

#define BP_GPMI_CTRL0_RUN      29
#define BM_GPMI_CTRL0_RUN      0x20000000

#define BF_GPMI_CTRL0_RUN(v)   (((v) << 29) & BM_GPMI_CTRL0_RUN)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_RUN(v)   BF_CS1(GPMI_CTRL0, RUN, v)
#endif

#define BV_GPMI_CTRL0_RUN__IDLE  0x0
#define BV_GPMI_CTRL0_RUN__BUSY  0x1

/* --- Register HW_GPMI_CTRL0, field DEV_IRQ_EN */

#define BP_GPMI_CTRL0_DEV_IRQ_EN      28
#define BM_GPMI_CTRL0_DEV_IRQ_EN      0x10000000

#define BF_GPMI_CTRL0_DEV_IRQ_EN(v)   (((v) << 28) & BM_GPMI_CTRL0_DEV_IRQ_EN)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_DEV_IRQ_EN(v)   BF_CS1(GPMI_CTRL0, DEV_IRQ_EN, v)
#endif

/* --- Register HW_GPMI_CTRL0, field TIMEOUT_IRQ_EN */

#define BP_GPMI_CTRL0_TIMEOUT_IRQ_EN      27
#define BM_GPMI_CTRL0_TIMEOUT_IRQ_EN      0x08000000

#define BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(v)   (((v) << 27) & BM_GPMI_CTRL0_TIMEOUT_IRQ_EN)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_TIMEOUT_IRQ_EN(v)   BF_CS1(GPMI_CTRL0, TIMEOUT_IRQ_EN, v)
#endif

/* --- Register HW_GPMI_CTRL0, field UDMA */

#define BP_GPMI_CTRL0_UDMA      26
#define BM_GPMI_CTRL0_UDMA      0x04000000

#define BF_GPMI_CTRL0_UDMA(v)   (((v) << 26) & BM_GPMI_CTRL0_UDMA)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_UDMA(v)   BF_CS1(GPMI_CTRL0, UDMA, v)
#endif

#define BV_GPMI_CTRL0_UDMA__DISABLED  0x0
#define BV_GPMI_CTRL0_UDMA__ENABLED   0x1

/* --- Register HW_GPMI_CTRL0, field COMMAND_MODE */

#define BP_GPMI_CTRL0_COMMAND_MODE      24
#define BM_GPMI_CTRL0_COMMAND_MODE      0x03000000

#define BF_GPMI_CTRL0_COMMAND_MODE(v)   (((v) << 24) & BM_GPMI_CTRL0_COMMAND_MODE)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_COMMAND_MODE(v)   BF_CS1(GPMI_CTRL0, COMMAND_MODE, v)
#endif

#define BV_GPMI_CTRL0_COMMAND_MODE__WRITE             0x0
#define BV_GPMI_CTRL0_COMMAND_MODE__READ              0x1
#define BV_GPMI_CTRL0_COMMAND_MODE__READ_AND_COMPARE  0x2
#define BV_GPMI_CTRL0_COMMAND_MODE__WAIT_FOR_READY    0x3

/* --- Register HW_GPMI_CTRL0, field WORD_LENGTH */

#define BP_GPMI_CTRL0_WORD_LENGTH      23
#define BM_GPMI_CTRL0_WORD_LENGTH      0x00800000

#define BF_GPMI_CTRL0_WORD_LENGTH(v)   (((v) << 23) & BM_GPMI_CTRL0_WORD_LENGTH)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_WORD_LENGTH(v)   BF_CS1(GPMI_CTRL0, WORD_LENGTH, v)
#endif

#define BV_GPMI_CTRL0_WORD_LENGTH__16_BIT  0x0
#define BV_GPMI_CTRL0_WORD_LENGTH__8_BIT   0x1

/* --- Register HW_GPMI_CTRL0, field LOCK_CS */

#define BP_GPMI_CTRL0_LOCK_CS      22
#define BM_GPMI_CTRL0_LOCK_CS      0x00400000

#define BF_GPMI_CTRL0_LOCK_CS(v)   (((v) << 22) & BM_GPMI_CTRL0_LOCK_CS)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_LOCK_CS(v)   BF_CS1(GPMI_CTRL0, LOCK_CS, v)
#endif

#define BV_GPMI_CTRL0_LOCK_CS__DISABLED  0x0
#define BV_GPMI_CTRL0_LOCK_CS__ENABLED   0x1

/* --- Register HW_GPMI_CTRL0, field CS */

#define BP_GPMI_CTRL0_CS      20
#define BM_GPMI_CTRL0_CS      0x00300000

#define BF_GPMI_CTRL0_CS(v)   (((v) << 20) & BM_GPMI_CTRL0_CS)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_CS(v)   BF_CS1(GPMI_CTRL0, CS, v)
#endif

/* --- Register HW_GPMI_CTRL0, field ADDRESS */

#define BP_GPMI_CTRL0_ADDRESS      17
#define BM_GPMI_CTRL0_ADDRESS      0x000E0000

#define BF_GPMI_CTRL0_ADDRESS(v)   (((v) << 17) & BM_GPMI_CTRL0_ADDRESS)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_ADDRESS(v)   BF_CS1(GPMI_CTRL0, ADDRESS, v)
#endif

#define BV_GPMI_CTRL0_ADDRESS__NAND_DATA  0x0
#define BV_GPMI_CTRL0_ADDRESS__NAND_CLE   0x1
#define BV_GPMI_CTRL0_ADDRESS__NAND_ALE   0x2

/* --- Register HW_GPMI_CTRL0, field ADDRESS_INCREMENT */

#define BP_GPMI_CTRL0_ADDRESS_INCREMENT      16
#define BM_GPMI_CTRL0_ADDRESS_INCREMENT      0x00010000

#define BF_GPMI_CTRL0_ADDRESS_INCREMENT(v)   (((v) << 16) & BM_GPMI_CTRL0_ADDRESS_INCREMENT)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_ADDRESS_INCREMENT(v)   BF_CS1(GPMI_CTRL0, ADDRESS_INCREMENT, v)
#endif

#define BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED  0x0
#define BV_GPMI_CTRL0_ADDRESS_INCREMENT__ENABLED   0x1

/* --- Register HW_GPMI_CTRL0, field XFER_COUNT */

#define BP_GPMI_CTRL0_XFER_COUNT      0
#define BM_GPMI_CTRL0_XFER_COUNT      0x0000FFFF

#define BF_GPMI_CTRL0_XFER_COUNT(v)   (((v) << 0) & BM_GPMI_CTRL0_XFER_COUNT)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL0_XFER_COUNT(v)   (HW_GPMI_CTRL0.B.XFER_COUNT = (v))
#endif



/*
 * HW_GPMI_COMPARE - GPMI Compare Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned REFERENCE  : 16;
        unsigned MASK       : 16;
    } B;
} hw_gpmi_compare_t;
#endif

/*
 * constants & macros for entire HW_GPMI_COMPARE register
 */
#define HW_GPMI_COMPARE_ADDR      (REGS_GPMI_BASE + 0x00000010)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_COMPARE           (*(volatile hw_gpmi_compare_t *) HW_GPMI_COMPARE_ADDR)
#define HW_GPMI_COMPARE_RD()      (HW_GPMI_COMPARE.U)
#define HW_GPMI_COMPARE_WR(v)     (HW_GPMI_COMPARE.U = (v))
#define HW_GPMI_COMPARE_SET(v)    (HW_GPMI_COMPARE_WR(HW_GPMI_COMPARE_RD() |  (v)))
#define HW_GPMI_COMPARE_CLR(v)    (HW_GPMI_COMPARE_WR(HW_GPMI_COMPARE_RD() & ~(v)))
#define HW_GPMI_COMPARE_TOG(v)    (HW_GPMI_COMPARE_WR(HW_GPMI_COMPARE_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_COMPARE bitfields
 */
/* --- Register HW_GPMI_COMPARE, field MASK */

#define BP_GPMI_COMPARE_MASK      16
#define BM_GPMI_COMPARE_MASK      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_COMPARE_MASK(v)   ((((unsigned int) v) << 16) & BM_GPMI_COMPARE_MASK)
#else
#define BF_GPMI_COMPARE_MASK(v)   (((v) << 16) & BM_GPMI_COMPARE_MASK)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_COMPARE_MASK(v)   (HW_GPMI_COMPARE.B.MASK = (v))
#endif

/* --- Register HW_GPMI_COMPARE, field REFERENCE */

#define BP_GPMI_COMPARE_REFERENCE      0
#define BM_GPMI_COMPARE_REFERENCE      0x0000FFFF

#define BF_GPMI_COMPARE_REFERENCE(v)   (((v) << 0) & BM_GPMI_COMPARE_REFERENCE)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_COMPARE_REFERENCE(v)   (HW_GPMI_COMPARE.B.REFERENCE = (v))
#endif



/*
 * HW_GPMI_ECCCTRL - GPMI Integrated ECC Control Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned BUFFER_MASK  :  9;
        unsigned RSVD1        :  3;
        unsigned ENABLE_ECC   :  1;
        unsigned ECC_CMD      :  2;
        unsigned RSVD2        :  1;
        unsigned HANDLE       : 16;
    } B;
} hw_gpmi_eccctrl_t;
#endif

/*
 * constants & macros for entire HW_GPMI_ECCCTRL register
 */
#define HW_GPMI_ECCCTRL_ADDR      (REGS_GPMI_BASE + 0x00000020)
#define HW_GPMI_ECCCTRL_SET_ADDR  (REGS_GPMI_BASE + 0x00000024)
#define HW_GPMI_ECCCTRL_CLR_ADDR  (REGS_GPMI_BASE + 0x00000028)
#define HW_GPMI_ECCCTRL_TOG_ADDR  (REGS_GPMI_BASE + 0x0000002C)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_ECCCTRL           (*(volatile hw_gpmi_eccctrl_t *) HW_GPMI_ECCCTRL_ADDR)
#define HW_GPMI_ECCCTRL_RD()      (HW_GPMI_ECCCTRL.U)
#define HW_GPMI_ECCCTRL_WR(v)     (HW_GPMI_ECCCTRL.U = (v))
#define HW_GPMI_ECCCTRL_SET(v)    ((*(volatile unsigned int *) HW_GPMI_ECCCTRL_SET_ADDR) = (v))
#define HW_GPMI_ECCCTRL_CLR(v)    ((*(volatile unsigned int *) HW_GPMI_ECCCTRL_CLR_ADDR) = (v))
#define HW_GPMI_ECCCTRL_TOG(v)    ((*(volatile unsigned int *) HW_GPMI_ECCCTRL_TOG_ADDR) = (v))
#endif


/*
 * constants & macros for individual HW_GPMI_ECCCTRL bitfields
 */
/* --- Register HW_GPMI_ECCCTRL, field HANDLE */

#define BP_GPMI_ECCCTRL_HANDLE      16
#define BM_GPMI_ECCCTRL_HANDLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_ECCCTRL_HANDLE(v)   ((((unsigned int) v) << 16) & BM_GPMI_ECCCTRL_HANDLE)
#else
#define BF_GPMI_ECCCTRL_HANDLE(v)   (((v) << 16) & BM_GPMI_ECCCTRL_HANDLE)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_ECCCTRL_HANDLE(v)   (HW_GPMI_ECCCTRL.B.HANDLE = (v))
#endif

/* --- Register HW_GPMI_ECCCTRL, field ECC_CMD */

#define BP_GPMI_ECCCTRL_ECC_CMD      13
#define BM_GPMI_ECCCTRL_ECC_CMD      0x00006000

#define BF_GPMI_ECCCTRL_ECC_CMD(v)   (((v) << 13) & BM_GPMI_ECCCTRL_ECC_CMD)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_ECCCTRL_ECC_CMD(v)   BF_CS1(GPMI_ECCCTRL, ECC_CMD, v)
#endif

#define BV_GPMI_ECCCTRL_ECC_CMD__DECODE_4_BIT  0x0
#define BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_4_BIT  0x1
#define BV_GPMI_ECCCTRL_ECC_CMD__DECODE_8_BIT  0x2
#define BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_8_BIT  0x3

/* --- Register HW_GPMI_ECCCTRL, field ENABLE_ECC */

#define BP_GPMI_ECCCTRL_ENABLE_ECC      12
#define BM_GPMI_ECCCTRL_ENABLE_ECC      0x00001000

#define BF_GPMI_ECCCTRL_ENABLE_ECC(v)   (((v) << 12) & BM_GPMI_ECCCTRL_ENABLE_ECC)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_ECCCTRL_ENABLE_ECC(v)   BF_CS1(GPMI_ECCCTRL, ENABLE_ECC, v)
#endif

#define BV_GPMI_ECCCTRL_ENABLE_ECC__ENABLE   0x1
#define BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE  0x0

/* --- Register HW_GPMI_ECCCTRL, field BUFFER_MASK */

#define BP_GPMI_ECCCTRL_BUFFER_MASK      0
#define BM_GPMI_ECCCTRL_BUFFER_MASK      0x000001FF

#define BF_GPMI_ECCCTRL_BUFFER_MASK(v)   (((v) << 0) & BM_GPMI_ECCCTRL_BUFFER_MASK)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_ECCCTRL_BUFFER_MASK(v)   BF_CS1(GPMI_ECCCTRL, BUFFER_MASK, v)
#endif

#define BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_AUXONLY  0x100
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE     0x1FF
#define BV_GPMI_ECCCTRL_BUFFER_MASK__AUXILIARY    0x100
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BUFFER7      0x080
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BUFFER6      0x040
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BUFFER5      0x020
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BUFFER4      0x010
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BUFFER3      0x008
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BUFFER2      0x004
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BUFFER1      0x002
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BUFFER0      0x001



/*
 * HW_GPMI_ECCCOUNT - GPMI Integrated ECC Transfer Count Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned COUNT  : 16;
        unsigned RSVD2  : 16;
    } B;
} hw_gpmi_ecccount_t;
#endif

/*
 * constants & macros for entire HW_GPMI_ECCCOUNT register
 */
#define HW_GPMI_ECCCOUNT_ADDR      (REGS_GPMI_BASE + 0x00000030)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_ECCCOUNT           (*(volatile hw_gpmi_ecccount_t *) HW_GPMI_ECCCOUNT_ADDR)
#define HW_GPMI_ECCCOUNT_RD()      (HW_GPMI_ECCCOUNT.U)
#define HW_GPMI_ECCCOUNT_WR(v)     (HW_GPMI_ECCCOUNT.U = (v))
#define HW_GPMI_ECCCOUNT_SET(v)    (HW_GPMI_ECCCOUNT_WR(HW_GPMI_ECCCOUNT_RD() |  (v)))
#define HW_GPMI_ECCCOUNT_CLR(v)    (HW_GPMI_ECCCOUNT_WR(HW_GPMI_ECCCOUNT_RD() & ~(v)))
#define HW_GPMI_ECCCOUNT_TOG(v)    (HW_GPMI_ECCCOUNT_WR(HW_GPMI_ECCCOUNT_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_ECCCOUNT bitfields
 */
/* --- Register HW_GPMI_ECCCOUNT, field COUNT */

#define BP_GPMI_ECCCOUNT_COUNT      0
#define BM_GPMI_ECCCOUNT_COUNT      0x0000FFFF

#define BF_GPMI_ECCCOUNT_COUNT(v)   (((v) << 0) & BM_GPMI_ECCCOUNT_COUNT)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_ECCCOUNT_COUNT(v)   (HW_GPMI_ECCCOUNT.B.COUNT = (v))
#endif



/*
 * HW_GPMI_PAYLOAD - GPMI Payload Address Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned RSVD0    :  2;
        unsigned ADDRESS  : 30;
    } B;
} hw_gpmi_payload_t;
#endif

/*
 * constants & macros for entire HW_GPMI_PAYLOAD register
 */
#define HW_GPMI_PAYLOAD_ADDR      (REGS_GPMI_BASE + 0x00000040)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_PAYLOAD           (*(volatile hw_gpmi_payload_t *) HW_GPMI_PAYLOAD_ADDR)
#define HW_GPMI_PAYLOAD_RD()      (HW_GPMI_PAYLOAD.U)
#define HW_GPMI_PAYLOAD_WR(v)     (HW_GPMI_PAYLOAD.U = (v))
#define HW_GPMI_PAYLOAD_SET(v)    (HW_GPMI_PAYLOAD_WR(HW_GPMI_PAYLOAD_RD() |  (v)))
#define HW_GPMI_PAYLOAD_CLR(v)    (HW_GPMI_PAYLOAD_WR(HW_GPMI_PAYLOAD_RD() & ~(v)))
#define HW_GPMI_PAYLOAD_TOG(v)    (HW_GPMI_PAYLOAD_WR(HW_GPMI_PAYLOAD_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_PAYLOAD bitfields
 */
/* --- Register HW_GPMI_PAYLOAD, field ADDRESS */

#define BP_GPMI_PAYLOAD_ADDRESS      2
#define BM_GPMI_PAYLOAD_ADDRESS      0xFFFFFFFC

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_PAYLOAD_ADDRESS(v)   ((((unsigned int) v) << 2) & BM_GPMI_PAYLOAD_ADDRESS)
#else
#define BF_GPMI_PAYLOAD_ADDRESS(v)   (((v) << 2) & BM_GPMI_PAYLOAD_ADDRESS)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_PAYLOAD_ADDRESS(v)   BF_CS1(GPMI_PAYLOAD, ADDRESS, v)
#endif



/*
 * HW_GPMI_AUXILIARY - GPMI Auxiliary Address Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned RSVD0    :  2;
        unsigned ADDRESS  : 30;
    } B;
} hw_gpmi_auxiliary_t;
#endif

/*
 * constants & macros for entire HW_GPMI_AUXILIARY register
 */
#define HW_GPMI_AUXILIARY_ADDR      (REGS_GPMI_BASE + 0x00000050)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_AUXILIARY           (*(volatile hw_gpmi_auxiliary_t *) HW_GPMI_AUXILIARY_ADDR)
#define HW_GPMI_AUXILIARY_RD()      (HW_GPMI_AUXILIARY.U)
#define HW_GPMI_AUXILIARY_WR(v)     (HW_GPMI_AUXILIARY.U = (v))
#define HW_GPMI_AUXILIARY_SET(v)    (HW_GPMI_AUXILIARY_WR(HW_GPMI_AUXILIARY_RD() |  (v)))
#define HW_GPMI_AUXILIARY_CLR(v)    (HW_GPMI_AUXILIARY_WR(HW_GPMI_AUXILIARY_RD() & ~(v)))
#define HW_GPMI_AUXILIARY_TOG(v)    (HW_GPMI_AUXILIARY_WR(HW_GPMI_AUXILIARY_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_AUXILIARY bitfields
 */
/* --- Register HW_GPMI_AUXILIARY, field ADDRESS */

#define BP_GPMI_AUXILIARY_ADDRESS      2
#define BM_GPMI_AUXILIARY_ADDRESS      0xFFFFFFFC

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_AUXILIARY_ADDRESS(v)   ((((unsigned int) v) << 2) & BM_GPMI_AUXILIARY_ADDRESS)
#else
#define BF_GPMI_AUXILIARY_ADDRESS(v)   (((v) << 2) & BM_GPMI_AUXILIARY_ADDRESS)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_AUXILIARY_ADDRESS(v)   BF_CS1(GPMI_AUXILIARY, ADDRESS, v)
#endif



/*
 * HW_GPMI_CTRL1 - GPMI Control Register 1
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned GPMI_MODE              :  1;
        unsigned CAMERA_MODE            :  1;
        unsigned ATA_IRQRDY_POLARITY    :  1;
        unsigned DEV_RESET              :  1;
        unsigned ABORT_WAIT_FOR_READY0  :  1;
        unsigned ABORT_WAIT_FOR_READY1  :  1;
        unsigned ABORT_WAIT_FOR_READY2  :  1;
        unsigned ABORT_WAIT_FOR_READY3  :  1;
        unsigned BURST_EN               :  1;
        unsigned TIMEOUT_IRQ            :  1;
        unsigned DEV_IRQ                :  1;
        unsigned DMA2ECC_MODE           :  1;
        unsigned RDN_DELAY              :  4;
        unsigned HALF_PERIOD            :  1;
        unsigned DLL_ENABLE             :  1;
        unsigned BCH_MODE               :  1;
        unsigned GANGED_RDYBUSY         :  1;
        unsigned CE0_SEL                :  1;
        unsigned CE1_SEL                :  1;
        unsigned CE2_SEL                :  1;
        unsigned CE3_SEL                :  1;
        unsigned RSVD2                  :  8;
    } B;
} hw_gpmi_ctrl1_t;
#endif

/*
 * constants & macros for entire HW_GPMI_CTRL1 register
 */
#define HW_GPMI_CTRL1_ADDR      (REGS_GPMI_BASE + 0x00000060)
#define HW_GPMI_CTRL1_SET_ADDR  (REGS_GPMI_BASE + 0x00000064)
#define HW_GPMI_CTRL1_CLR_ADDR  (REGS_GPMI_BASE + 0x00000068)
#define HW_GPMI_CTRL1_TOG_ADDR  (REGS_GPMI_BASE + 0x0000006C)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_CTRL1           (*(volatile hw_gpmi_ctrl1_t *) HW_GPMI_CTRL1_ADDR)
#define HW_GPMI_CTRL1_RD()      (HW_GPMI_CTRL1.U)
#define HW_GPMI_CTRL1_WR(v)     (HW_GPMI_CTRL1.U = (v))
#define HW_GPMI_CTRL1_SET(v)    ((*(volatile unsigned int *) HW_GPMI_CTRL1_SET_ADDR) = (v))
#define HW_GPMI_CTRL1_CLR(v)    ((*(volatile unsigned int *) HW_GPMI_CTRL1_CLR_ADDR) = (v))
#define HW_GPMI_CTRL1_TOG(v)    ((*(volatile unsigned int *) HW_GPMI_CTRL1_TOG_ADDR) = (v))
#endif


/*
 * constants & macros for individual HW_GPMI_CTRL1 bitfields
 */
/* --- Register HW_GPMI_CTRL1, field CE3_SEL */

#define BP_GPMI_CTRL1_CE3_SEL      23
#define BM_GPMI_CTRL1_CE3_SEL      0x00800000

#define BF_GPMI_CTRL1_CE3_SEL(v)   (((v) << 23) & BM_GPMI_CTRL1_CE3_SEL)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_CE3_SEL(v)   BF_CS1(GPMI_CTRL1, CE3_SEL, v)
#endif

/* --- Register HW_GPMI_CTRL1, field CE2_SEL */

#define BP_GPMI_CTRL1_CE2_SEL      22
#define BM_GPMI_CTRL1_CE2_SEL      0x00400000

#define BF_GPMI_CTRL1_CE2_SEL(v)   (((v) << 22) & BM_GPMI_CTRL1_CE2_SEL)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_CE2_SEL(v)   BF_CS1(GPMI_CTRL1, CE2_SEL, v)
#endif

/* --- Register HW_GPMI_CTRL1, field CE1_SEL */

#define BP_GPMI_CTRL1_CE1_SEL      21
#define BM_GPMI_CTRL1_CE1_SEL      0x00200000

#define BF_GPMI_CTRL1_CE1_SEL(v)   (((v) << 21) & BM_GPMI_CTRL1_CE1_SEL)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_CE1_SEL(v)   BF_CS1(GPMI_CTRL1, CE1_SEL, v)
#endif

/* --- Register HW_GPMI_CTRL1, field CE0_SEL */

#define BP_GPMI_CTRL1_CE0_SEL      20
#define BM_GPMI_CTRL1_CE0_SEL      0x00100000

#define BF_GPMI_CTRL1_CE0_SEL(v)   (((v) << 20) & BM_GPMI_CTRL1_CE0_SEL)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_CE0_SEL(v)   BF_CS1(GPMI_CTRL1, CE0_SEL, v)
#endif

/* --- Register HW_GPMI_CTRL1, field GANGED_RDYBUSY */

#define BP_GPMI_CTRL1_GANGED_RDYBUSY      19
#define BM_GPMI_CTRL1_GANGED_RDYBUSY      0x00080000

#define BF_GPMI_CTRL1_GANGED_RDYBUSY(v)   (((v) << 19) & BM_GPMI_CTRL1_GANGED_RDYBUSY)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_GANGED_RDYBUSY(v)   BF_CS1(GPMI_CTRL1, GANGED_RDYBUSY, v)
#endif

/* --- Register HW_GPMI_CTRL1, field BCH_MODE */

#define BP_GPMI_CTRL1_BCH_MODE      18
#define BM_GPMI_CTRL1_BCH_MODE      0x00040000

#define BF_GPMI_CTRL1_BCH_MODE(v)   (((v) << 18) & BM_GPMI_CTRL1_BCH_MODE)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_BCH_MODE(v)   BF_CS1(GPMI_CTRL1, BCH_MODE, v)
#endif

/* --- Register HW_GPMI_CTRL1, field DLL_ENABLE */

#define BP_GPMI_CTRL1_DLL_ENABLE      17
#define BM_GPMI_CTRL1_DLL_ENABLE      0x00020000

#define BF_GPMI_CTRL1_DLL_ENABLE(v)   (((v) << 17) & BM_GPMI_CTRL1_DLL_ENABLE)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_DLL_ENABLE(v)   BF_CS1(GPMI_CTRL1, DLL_ENABLE, v)
#endif

/* --- Register HW_GPMI_CTRL1, field HALF_PERIOD */

#define BP_GPMI_CTRL1_HALF_PERIOD      16
#define BM_GPMI_CTRL1_HALF_PERIOD      0x00010000

#define BF_GPMI_CTRL1_HALF_PERIOD(v)   (((v) << 16) & BM_GPMI_CTRL1_HALF_PERIOD)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_HALF_PERIOD(v)   BF_CS1(GPMI_CTRL1, HALF_PERIOD, v)
#endif

/* --- Register HW_GPMI_CTRL1, field RDN_DELAY */

#define BP_GPMI_CTRL1_RDN_DELAY      12
#define BM_GPMI_CTRL1_RDN_DELAY      0x0000F000

#define BF_GPMI_CTRL1_RDN_DELAY(v)   (((v) << 12) & BM_GPMI_CTRL1_RDN_DELAY)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_RDN_DELAY(v)   BF_CS1(GPMI_CTRL1, RDN_DELAY, v)
#endif

/* --- Register HW_GPMI_CTRL1, field DMA2ECC_MODE */

#define BP_GPMI_CTRL1_DMA2ECC_MODE      11
#define BM_GPMI_CTRL1_DMA2ECC_MODE      0x00000800

#define BF_GPMI_CTRL1_DMA2ECC_MODE(v)   (((v) << 11) & BM_GPMI_CTRL1_DMA2ECC_MODE)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_DMA2ECC_MODE(v)   BF_CS1(GPMI_CTRL1, DMA2ECC_MODE, v)
#endif

/* --- Register HW_GPMI_CTRL1, field DEV_IRQ */

#define BP_GPMI_CTRL1_DEV_IRQ      10
#define BM_GPMI_CTRL1_DEV_IRQ      0x00000400

#define BF_GPMI_CTRL1_DEV_IRQ(v)   (((v) << 10) & BM_GPMI_CTRL1_DEV_IRQ)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_DEV_IRQ(v)   BF_CS1(GPMI_CTRL1, DEV_IRQ, v)
#endif

/* --- Register HW_GPMI_CTRL1, field TIMEOUT_IRQ */

#define BP_GPMI_CTRL1_TIMEOUT_IRQ      9
#define BM_GPMI_CTRL1_TIMEOUT_IRQ      0x00000200

#define BF_GPMI_CTRL1_TIMEOUT_IRQ(v)   (((v) << 9) & BM_GPMI_CTRL1_TIMEOUT_IRQ)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_TIMEOUT_IRQ(v)   BF_CS1(GPMI_CTRL1, TIMEOUT_IRQ, v)
#endif

/* --- Register HW_GPMI_CTRL1, field BURST_EN */

#define BP_GPMI_CTRL1_BURST_EN      8
#define BM_GPMI_CTRL1_BURST_EN      0x00000100

#define BF_GPMI_CTRL1_BURST_EN(v)   (((v) << 8) & BM_GPMI_CTRL1_BURST_EN)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_BURST_EN(v)   BF_CS1(GPMI_CTRL1, BURST_EN, v)
#endif

/* --- Register HW_GPMI_CTRL1, field ABORT_WAIT_FOR_READY3 */

#define BP_GPMI_CTRL1_ABORT_WAIT_FOR_READY3      7
#define BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY3      0x00000080

#define BF_GPMI_CTRL1_ABORT_WAIT_FOR_READY3(v)   (((v) << 7) & BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY3)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_ABORT_WAIT_FOR_READY3(v)   BF_CS1(GPMI_CTRL1, ABORT_WAIT_FOR_READY3, v)
#endif

/* --- Register HW_GPMI_CTRL1, field ABORT_WAIT_FOR_READY2 */

#define BP_GPMI_CTRL1_ABORT_WAIT_FOR_READY2      6
#define BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY2      0x00000040

#define BF_GPMI_CTRL1_ABORT_WAIT_FOR_READY2(v)   (((v) << 6) & BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY2)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_ABORT_WAIT_FOR_READY2(v)   BF_CS1(GPMI_CTRL1, ABORT_WAIT_FOR_READY2, v)
#endif

/* --- Register HW_GPMI_CTRL1, field ABORT_WAIT_FOR_READY1 */

#define BP_GPMI_CTRL1_ABORT_WAIT_FOR_READY1      5
#define BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY1      0x00000020

#define BF_GPMI_CTRL1_ABORT_WAIT_FOR_READY1(v)   (((v) << 5) & BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY1)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_ABORT_WAIT_FOR_READY1(v)   BF_CS1(GPMI_CTRL1, ABORT_WAIT_FOR_READY1, v)
#endif

/* --- Register HW_GPMI_CTRL1, field ABORT_WAIT_FOR_READY0 */

#define BP_GPMI_CTRL1_ABORT_WAIT_FOR_READY0      4
#define BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY0      0x00000010

#define BF_GPMI_CTRL1_ABORT_WAIT_FOR_READY0(v)   (((v) << 4) & BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY0)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_ABORT_WAIT_FOR_READY0(v)   BF_CS1(GPMI_CTRL1, ABORT_WAIT_FOR_READY0, v)
#endif

/* --- Register HW_GPMI_CTRL1, field DEV_RESET */

#define BP_GPMI_CTRL1_DEV_RESET      3
#define BM_GPMI_CTRL1_DEV_RESET      0x00000008

#define BF_GPMI_CTRL1_DEV_RESET(v)   (((v) << 3) & BM_GPMI_CTRL1_DEV_RESET)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_DEV_RESET(v)   BF_CS1(GPMI_CTRL1, DEV_RESET, v)
#endif

#define BV_GPMI_CTRL1_DEV_RESET__ENABLED   0x0
#define BV_GPMI_CTRL1_DEV_RESET__DISABLED  0x1

/* --- Register HW_GPMI_CTRL1, field ATA_IRQRDY_POLARITY */

#define BP_GPMI_CTRL1_ATA_IRQRDY_POLARITY      2
#define BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY      0x00000004

#define BF_GPMI_CTRL1_ATA_IRQRDY_POLARITY(v)   (((v) << 2) & BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_ATA_IRQRDY_POLARITY(v)   BF_CS1(GPMI_CTRL1, ATA_IRQRDY_POLARITY, v)
#endif

#define BV_GPMI_CTRL1_ATA_IRQRDY_POLARITY__ACTIVELOW   0x0
#define BV_GPMI_CTRL1_ATA_IRQRDY_POLARITY__ACTIVEHIGH  0x1

/* --- Register HW_GPMI_CTRL1, field CAMERA_MODE */

#define BP_GPMI_CTRL1_CAMERA_MODE      1
#define BM_GPMI_CTRL1_CAMERA_MODE      0x00000002

#define BF_GPMI_CTRL1_CAMERA_MODE(v)   (((v) << 1) & BM_GPMI_CTRL1_CAMERA_MODE)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_CAMERA_MODE(v)   BF_CS1(GPMI_CTRL1, CAMERA_MODE, v)
#endif

/* --- Register HW_GPMI_CTRL1, field GPMI_MODE */

#define BP_GPMI_CTRL1_GPMI_MODE      0
#define BM_GPMI_CTRL1_GPMI_MODE      0x00000001

#define BF_GPMI_CTRL1_GPMI_MODE(v)   (((v) << 0) & BM_GPMI_CTRL1_GPMI_MODE)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_CTRL1_GPMI_MODE(v)   BF_CS1(GPMI_CTRL1, GPMI_MODE, v)
#endif

#define BV_GPMI_CTRL1_GPMI_MODE__NAND  0x0
#define BV_GPMI_CTRL1_GPMI_MODE__ATA   0x1



/*
 * HW_GPMI_TIMING0 - GPMI Timing Register 0
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned DATA_SETUP     :  8;
        unsigned DATA_HOLD      :  8;
        unsigned ADDRESS_SETUP  :  8;
        unsigned RSVD1          :  8;
    } B;
} hw_gpmi_timing0_t;
#endif

/*
 * constants & macros for entire HW_GPMI_TIMING0 register
 */
#define HW_GPMI_TIMING0_ADDR      (REGS_GPMI_BASE + 0x00000070)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_TIMING0           (*(volatile hw_gpmi_timing0_t *) HW_GPMI_TIMING0_ADDR)
#define HW_GPMI_TIMING0_RD()      (HW_GPMI_TIMING0.U)
#define HW_GPMI_TIMING0_WR(v)     (HW_GPMI_TIMING0.U = (v))
#define HW_GPMI_TIMING0_SET(v)    (HW_GPMI_TIMING0_WR(HW_GPMI_TIMING0_RD() |  (v)))
#define HW_GPMI_TIMING0_CLR(v)    (HW_GPMI_TIMING0_WR(HW_GPMI_TIMING0_RD() & ~(v)))
#define HW_GPMI_TIMING0_TOG(v)    (HW_GPMI_TIMING0_WR(HW_GPMI_TIMING0_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_TIMING0 bitfields
 */
/* --- Register HW_GPMI_TIMING0, field ADDRESS_SETUP */

#define BP_GPMI_TIMING0_ADDRESS_SETUP      16
#define BM_GPMI_TIMING0_ADDRESS_SETUP      0x00FF0000

#define BF_GPMI_TIMING0_ADDRESS_SETUP(v)   (((v) << 16) & BM_GPMI_TIMING0_ADDRESS_SETUP)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_TIMING0_ADDRESS_SETUP(v)   (HW_GPMI_TIMING0.B.ADDRESS_SETUP = (v))
#endif

/* --- Register HW_GPMI_TIMING0, field DATA_HOLD */

#define BP_GPMI_TIMING0_DATA_HOLD      8
#define BM_GPMI_TIMING0_DATA_HOLD      0x0000FF00

#define BF_GPMI_TIMING0_DATA_HOLD(v)   (((v) << 8) & BM_GPMI_TIMING0_DATA_HOLD)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_TIMING0_DATA_HOLD(v)   (HW_GPMI_TIMING0.B.DATA_HOLD = (v))
#endif

/* --- Register HW_GPMI_TIMING0, field DATA_SETUP */

#define BP_GPMI_TIMING0_DATA_SETUP      0
#define BM_GPMI_TIMING0_DATA_SETUP      0x000000FF

#define BF_GPMI_TIMING0_DATA_SETUP(v)   (((v) << 0) & BM_GPMI_TIMING0_DATA_SETUP)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_TIMING0_DATA_SETUP(v)   (HW_GPMI_TIMING0.B.DATA_SETUP = (v))
#endif

#define NAND_GPMI_TIMING0(AddSetup, DataSetup, DataHold) \
           (BF_GPMI_TIMING0_ADDRESS_SETUP(AddSetup) | \
            BF_GPMI_TIMING0_DATA_HOLD(DataHold) | \
            BF_GPMI_TIMING0_DATA_SETUP(DataSetup))

/*
 * HW_GPMI_TIMING1 - GPMI Timing Register 1
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned RSVD1                : 16;
        unsigned DEVICE_BUSY_TIMEOUT  : 16;
    } B;
} hw_gpmi_timing1_t;
#endif

/*
 * constants & macros for entire HW_GPMI_TIMING1 register
 */
#define HW_GPMI_TIMING1_ADDR      (REGS_GPMI_BASE + 0x00000080)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_TIMING1           (*(volatile hw_gpmi_timing1_t *) HW_GPMI_TIMING1_ADDR)
#define HW_GPMI_TIMING1_RD()      (HW_GPMI_TIMING1.U)
#define HW_GPMI_TIMING1_WR(v)     (HW_GPMI_TIMING1.U = (v))
#define HW_GPMI_TIMING1_SET(v)    (HW_GPMI_TIMING1_WR(HW_GPMI_TIMING1_RD() |  (v)))
#define HW_GPMI_TIMING1_CLR(v)    (HW_GPMI_TIMING1_WR(HW_GPMI_TIMING1_RD() & ~(v)))
#define HW_GPMI_TIMING1_TOG(v)    (HW_GPMI_TIMING1_WR(HW_GPMI_TIMING1_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_TIMING1 bitfields
 */
/* --- Register HW_GPMI_TIMING1, field DEVICE_BUSY_TIMEOUT */

#define BP_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT      16
#define BM_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT(v)   ((((unsigned int) v) << 16) & BM_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT)
#else
#define BF_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT(v)   (((v) << 16) & BM_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT(v)   (HW_GPMI_TIMING1.B.DEVICE_BUSY_TIMEOUT = (v))
#endif



/*
 * HW_GPMI_TIMING2 - GPMI Timing Register 2
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned UDMA_SETUP  :  8;
        unsigned UDMA_HOLD   :  8;
        unsigned UDMA_ENV    :  8;
        unsigned UDMA_TRP    :  8;
    } B;
} hw_gpmi_timing2_t;
#endif

/*
 * constants & macros for entire HW_GPMI_TIMING2 register
 */
#define HW_GPMI_TIMING2_ADDR      (REGS_GPMI_BASE + 0x00000090)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_TIMING2           (*(volatile hw_gpmi_timing2_t *) HW_GPMI_TIMING2_ADDR)
#define HW_GPMI_TIMING2_RD()      (HW_GPMI_TIMING2.U)
#define HW_GPMI_TIMING2_WR(v)     (HW_GPMI_TIMING2.U = (v))
#define HW_GPMI_TIMING2_SET(v)    (HW_GPMI_TIMING2_WR(HW_GPMI_TIMING2_RD() |  (v)))
#define HW_GPMI_TIMING2_CLR(v)    (HW_GPMI_TIMING2_WR(HW_GPMI_TIMING2_RD() & ~(v)))
#define HW_GPMI_TIMING2_TOG(v)    (HW_GPMI_TIMING2_WR(HW_GPMI_TIMING2_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_TIMING2 bitfields
 */
/* --- Register HW_GPMI_TIMING2, field UDMA_TRP */

#define BP_GPMI_TIMING2_UDMA_TRP      24
#define BM_GPMI_TIMING2_UDMA_TRP      0xFF000000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_TIMING2_UDMA_TRP(v)   ((((unsigned int) v) << 24) & BM_GPMI_TIMING2_UDMA_TRP)
#else
#define BF_GPMI_TIMING2_UDMA_TRP(v)   (((v) << 24) & BM_GPMI_TIMING2_UDMA_TRP)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_TIMING2_UDMA_TRP(v)   (HW_GPMI_TIMING2.B.UDMA_TRP = (v))
#endif

/* --- Register HW_GPMI_TIMING2, field UDMA_ENV */

#define BP_GPMI_TIMING2_UDMA_ENV      16
#define BM_GPMI_TIMING2_UDMA_ENV      0x00FF0000

#define BF_GPMI_TIMING2_UDMA_ENV(v)   (((v) << 16) & BM_GPMI_TIMING2_UDMA_ENV)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_TIMING2_UDMA_ENV(v)   (HW_GPMI_TIMING2.B.UDMA_ENV = (v))
#endif

/* --- Register HW_GPMI_TIMING2, field UDMA_HOLD */

#define BP_GPMI_TIMING2_UDMA_HOLD      8
#define BM_GPMI_TIMING2_UDMA_HOLD      0x0000FF00

#define BF_GPMI_TIMING2_UDMA_HOLD(v)   (((v) << 8) & BM_GPMI_TIMING2_UDMA_HOLD)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_TIMING2_UDMA_HOLD(v)   (HW_GPMI_TIMING2.B.UDMA_HOLD = (v))
#endif

/* --- Register HW_GPMI_TIMING2, field UDMA_SETUP */

#define BP_GPMI_TIMING2_UDMA_SETUP      0
#define BM_GPMI_TIMING2_UDMA_SETUP      0x000000FF

#define BF_GPMI_TIMING2_UDMA_SETUP(v)   (((v) << 0) & BM_GPMI_TIMING2_UDMA_SETUP)

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_TIMING2_UDMA_SETUP(v)   (HW_GPMI_TIMING2.B.UDMA_SETUP = (v))
#endif



/*
 * HW_GPMI_DATA - GPMI DMA Data Transfer Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned DATA  : 32;
    } B;
} hw_gpmi_data_t;
#endif

/*
 * constants & macros for entire HW_GPMI_DATA register
 */
#define HW_GPMI_DATA_ADDR      (REGS_GPMI_BASE + 0x000000A0)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_DATA           (*(volatile hw_gpmi_data_t *) HW_GPMI_DATA_ADDR)
#define HW_GPMI_DATA_RD()      (HW_GPMI_DATA.U)
#define HW_GPMI_DATA_WR(v)     (HW_GPMI_DATA.U = (v))
#define HW_GPMI_DATA_SET(v)    (HW_GPMI_DATA_WR(HW_GPMI_DATA_RD() |  (v)))
#define HW_GPMI_DATA_CLR(v)    (HW_GPMI_DATA_WR(HW_GPMI_DATA_RD() & ~(v)))
#define HW_GPMI_DATA_TOG(v)    (HW_GPMI_DATA_WR(HW_GPMI_DATA_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_DATA bitfields
 */
/* --- Register HW_GPMI_DATA, field DATA */

#define BP_GPMI_DATA_DATA      0
#define BM_GPMI_DATA_DATA      0xFFFFFFFF

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_DATA_DATA(v)   ((unsigned int) v)
#else
#define BF_GPMI_DATA_DATA(v)   (v)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_GPMI_DATA_DATA(v)   (HW_GPMI_DATA.B.DATA = (v))
#endif



/*
 * HW_GPMI_STAT - GPMI Status Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned DEV0_ERROR           :  1;
        unsigned DEV1_ERROR           :  1;
        unsigned DEV2_ERROR           :  1;
        unsigned DEV3_ERROR           :  1;
        unsigned FIFO_FULL            :  1;
        unsigned FIFO_EMPTY           :  1;
        unsigned INVALID_BUFFER_MASK  :  1;
        unsigned ATA_IRQ              :  1;
        unsigned RDY_TIMEOUT          :  4;
        unsigned RSVD1                : 19;
        unsigned PRESENT              :  1;
    } B;
} hw_gpmi_stat_t;
#endif

/*
 * constants & macros for entire HW_GPMI_STAT register
 */
#define HW_GPMI_STAT_ADDR      (REGS_GPMI_BASE + 0x000000B0)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_STAT           (*(volatile hw_gpmi_stat_t *) HW_GPMI_STAT_ADDR)
#define HW_GPMI_STAT_RD()      (HW_GPMI_STAT.U)
#endif


/*
 * constants & macros for individual HW_GPMI_STAT bitfields
 */
/* --- Register HW_GPMI_STAT, field PRESENT */

#define BP_GPMI_STAT_PRESENT      31
#define BM_GPMI_STAT_PRESENT      0x80000000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_STAT_PRESENT(v)   ((((unsigned int) v) << 31) & BM_GPMI_STAT_PRESENT)
#else
#define BF_GPMI_STAT_PRESENT(v)   (((v) << 31) & BM_GPMI_STAT_PRESENT)
#endif

#define BV_GPMI_STAT_PRESENT__UNAVAILABLE  0x0
#define BV_GPMI_STAT_PRESENT__AVAILABLE    0x1

/* --- Register HW_GPMI_STAT, field RDY_TIMEOUT */

#define BP_GPMI_STAT_RDY_TIMEOUT      8
#define BM_GPMI_STAT_RDY_TIMEOUT      0x00000F00

#define BF_GPMI_STAT_RDY_TIMEOUT(v)   (((v) << 8) & BM_GPMI_STAT_RDY_TIMEOUT)

/* --- Register HW_GPMI_STAT, field ATA_IRQ */

#define BP_GPMI_STAT_ATA_IRQ      7
#define BM_GPMI_STAT_ATA_IRQ      0x00000080

#define BF_GPMI_STAT_ATA_IRQ(v)   (((v) << 7) & BM_GPMI_STAT_ATA_IRQ)

/* --- Register HW_GPMI_STAT, field INVALID_BUFFER_MASK */

#define BP_GPMI_STAT_INVALID_BUFFER_MASK      6
#define BM_GPMI_STAT_INVALID_BUFFER_MASK      0x00000040

#define BF_GPMI_STAT_INVALID_BUFFER_MASK(v)   (((v) << 6) & BM_GPMI_STAT_INVALID_BUFFER_MASK)

/* --- Register HW_GPMI_STAT, field FIFO_EMPTY */

#define BP_GPMI_STAT_FIFO_EMPTY      5
#define BM_GPMI_STAT_FIFO_EMPTY      0x00000020

#define BF_GPMI_STAT_FIFO_EMPTY(v)   (((v) << 5) & BM_GPMI_STAT_FIFO_EMPTY)

#define BV_GPMI_STAT_FIFO_EMPTY__NOT_EMPTY  0x0
#define BV_GPMI_STAT_FIFO_EMPTY__EMPTY      0x1

/* --- Register HW_GPMI_STAT, field FIFO_FULL */

#define BP_GPMI_STAT_FIFO_FULL      4
#define BM_GPMI_STAT_FIFO_FULL      0x00000010

#define BF_GPMI_STAT_FIFO_FULL(v)   (((v) << 4) & BM_GPMI_STAT_FIFO_FULL)

#define BV_GPMI_STAT_FIFO_FULL__NOT_FULL  0x0
#define BV_GPMI_STAT_FIFO_FULL__FULL      0x1

/* --- Register HW_GPMI_STAT, field DEV3_ERROR */

#define BP_GPMI_STAT_DEV3_ERROR      3
#define BM_GPMI_STAT_DEV3_ERROR      0x00000008

#define BF_GPMI_STAT_DEV3_ERROR(v)   (((v) << 3) & BM_GPMI_STAT_DEV3_ERROR)

/* --- Register HW_GPMI_STAT, field DEV2_ERROR */

#define BP_GPMI_STAT_DEV2_ERROR      2
#define BM_GPMI_STAT_DEV2_ERROR      0x00000004

#define BF_GPMI_STAT_DEV2_ERROR(v)   (((v) << 2) & BM_GPMI_STAT_DEV2_ERROR)

/* --- Register HW_GPMI_STAT, field DEV1_ERROR */

#define BP_GPMI_STAT_DEV1_ERROR      1
#define BM_GPMI_STAT_DEV1_ERROR      0x00000002

#define BF_GPMI_STAT_DEV1_ERROR(v)   (((v) << 1) & BM_GPMI_STAT_DEV1_ERROR)

/* --- Register HW_GPMI_STAT, field DEV0_ERROR */

#define BP_GPMI_STAT_DEV0_ERROR      0
#define BM_GPMI_STAT_DEV0_ERROR      0x00000001

#define BF_GPMI_STAT_DEV0_ERROR(v)   (((v) << 0) & BM_GPMI_STAT_DEV0_ERROR)



/*
 * HW_GPMI_DEBUG - GPMI Debug Information Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned MAIN_STATE           :  4;
        unsigned PIN_STATE            :  3;
        unsigned BUSY                 :  1;
        unsigned UDMA_STATE           :  4;
        unsigned CMD_END              :  4;
        unsigned DMAREQ0              :  1;
        unsigned DMAREQ1              :  1;
        unsigned DMAREQ2              :  1;
        unsigned DMAREQ3              :  1;
        unsigned SENSE0               :  1;
        unsigned SENSE1               :  1;
        unsigned SENSE2               :  1;
        unsigned SENSE3               :  1;
        unsigned WAIT_FOR_READY_END0  :  1;
        unsigned WAIT_FOR_READY_END1  :  1;
        unsigned WAIT_FOR_READY_END2  :  1;
        unsigned WAIT_FOR_READY_END3  :  1;
        unsigned READY0               :  1;
        unsigned READY1               :  1;
        unsigned READY2               :  1;
        unsigned READY3               :  1;
    } B;
} hw_gpmi_debug_t;
#endif

/*
 * constants & macros for entire HW_GPMI_DEBUG register
 */
#define HW_GPMI_DEBUG_ADDR      (0x8000c000 + 0x000000C0)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_DEBUG           (*(volatile hw_gpmi_debug_t *) HW_GPMI_DEBUG_ADDR)
#define HW_GPMI_DEBUG_RD()      (HW_GPMI_DEBUG.U)
#endif


/*
 * constants & macros for individual HW_GPMI_DEBUG bitfields
 */
/* --- Register HW_GPMI_DEBUG, field READY3 */

#define BP_GPMI_DEBUG_READY3      31
#define BM_GPMI_DEBUG_READY3      0x80000000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_DEBUG_READY3(v)   ((((unsigned int) v) << 31) & BM_GPMI_DEBUG_READY3)
#else
#define BF_GPMI_DEBUG_READY3(v)   (((v) << 31) & BM_GPMI_DEBUG_READY3)
#endif

/* --- Register HW_GPMI_DEBUG, field READY2 */

#define BP_GPMI_DEBUG_READY2      30
#define BM_GPMI_DEBUG_READY2      0x40000000

#define BF_GPMI_DEBUG_READY2(v)   (((v) << 30) & BM_GPMI_DEBUG_READY2)

/* --- Register HW_GPMI_DEBUG, field READY1 */

#define BP_GPMI_DEBUG_READY1      29
#define BM_GPMI_DEBUG_READY1      0x20000000

#define BF_GPMI_DEBUG_READY1(v)   (((v) << 29) & BM_GPMI_DEBUG_READY1)

/* --- Register HW_GPMI_DEBUG, field READY0 */

#define BP_GPMI_DEBUG_READY0      28
#define BM_GPMI_DEBUG_READY0      0x10000000

#define BF_GPMI_DEBUG_READY0(v)   (((v) << 28) & BM_GPMI_DEBUG_READY0)

/* --- Register HW_GPMI_DEBUG, field WAIT_FOR_READY_END3 */

#define BP_GPMI_DEBUG_WAIT_FOR_READY_END3      27
#define BM_GPMI_DEBUG_WAIT_FOR_READY_END3      0x08000000

#define BF_GPMI_DEBUG_WAIT_FOR_READY_END3(v)   (((v) << 27) & BM_GPMI_DEBUG_WAIT_FOR_READY_END3)

/* --- Register HW_GPMI_DEBUG, field WAIT_FOR_READY_END2 */

#define BP_GPMI_DEBUG_WAIT_FOR_READY_END2      26
#define BM_GPMI_DEBUG_WAIT_FOR_READY_END2      0x04000000

#define BF_GPMI_DEBUG_WAIT_FOR_READY_END2(v)   (((v) << 26) & BM_GPMI_DEBUG_WAIT_FOR_READY_END2)

/* --- Register HW_GPMI_DEBUG, field WAIT_FOR_READY_END1 */

#define BP_GPMI_DEBUG_WAIT_FOR_READY_END1      25
#define BM_GPMI_DEBUG_WAIT_FOR_READY_END1      0x02000000

#define BF_GPMI_DEBUG_WAIT_FOR_READY_END1(v)   (((v) << 25) & BM_GPMI_DEBUG_WAIT_FOR_READY_END1)

/* --- Register HW_GPMI_DEBUG, field WAIT_FOR_READY_END0 */

#define BP_GPMI_DEBUG_WAIT_FOR_READY_END0      24
#define BM_GPMI_DEBUG_WAIT_FOR_READY_END0      0x01000000

#define BF_GPMI_DEBUG_WAIT_FOR_READY_END0(v)   (((v) << 24) & BM_GPMI_DEBUG_WAIT_FOR_READY_END0)

/* --- Register HW_GPMI_DEBUG, field SENSE3 */

#define BP_GPMI_DEBUG_SENSE3      23
#define BM_GPMI_DEBUG_SENSE3      0x00800000

#define BF_GPMI_DEBUG_SENSE3(v)   (((v) << 23) & BM_GPMI_DEBUG_SENSE3)

/* --- Register HW_GPMI_DEBUG, field SENSE2 */

#define BP_GPMI_DEBUG_SENSE2      22
#define BM_GPMI_DEBUG_SENSE2      0x00400000

#define BF_GPMI_DEBUG_SENSE2(v)   (((v) << 22) & BM_GPMI_DEBUG_SENSE2)

/* --- Register HW_GPMI_DEBUG, field SENSE1 */

#define BP_GPMI_DEBUG_SENSE1      21
#define BM_GPMI_DEBUG_SENSE1      0x00200000

#define BF_GPMI_DEBUG_SENSE1(v)   (((v) << 21) & BM_GPMI_DEBUG_SENSE1)

/* --- Register HW_GPMI_DEBUG, field SENSE0 */

#define BP_GPMI_DEBUG_SENSE0      20
#define BM_GPMI_DEBUG_SENSE0      0x00100000

#define BF_GPMI_DEBUG_SENSE0(v)   (((v) << 20) & BM_GPMI_DEBUG_SENSE0)

/* --- Register HW_GPMI_DEBUG, field DMAREQ3 */

#define BP_GPMI_DEBUG_DMAREQ3      19
#define BM_GPMI_DEBUG_DMAREQ3      0x00080000

#define BF_GPMI_DEBUG_DMAREQ3(v)   (((v) << 19) & BM_GPMI_DEBUG_DMAREQ3)

/* --- Register HW_GPMI_DEBUG, field DMAREQ2 */

#define BP_GPMI_DEBUG_DMAREQ2      18
#define BM_GPMI_DEBUG_DMAREQ2      0x00040000

#define BF_GPMI_DEBUG_DMAREQ2(v)   (((v) << 18) & BM_GPMI_DEBUG_DMAREQ2)

/* --- Register HW_GPMI_DEBUG, field DMAREQ1 */

#define BP_GPMI_DEBUG_DMAREQ1      17
#define BM_GPMI_DEBUG_DMAREQ1      0x00020000

#define BF_GPMI_DEBUG_DMAREQ1(v)   (((v) << 17) & BM_GPMI_DEBUG_DMAREQ1)

/* --- Register HW_GPMI_DEBUG, field DMAREQ0 */

#define BP_GPMI_DEBUG_DMAREQ0      16
#define BM_GPMI_DEBUG_DMAREQ0      0x00010000

#define BF_GPMI_DEBUG_DMAREQ0(v)   (((v) << 16) & BM_GPMI_DEBUG_DMAREQ0)

/* --- Register HW_GPMI_DEBUG, field CMD_END */

#define BP_GPMI_DEBUG_CMD_END      12
#define BM_GPMI_DEBUG_CMD_END      0x0000F000

#define BF_GPMI_DEBUG_CMD_END(v)   (((v) << 12) & BM_GPMI_DEBUG_CMD_END)

/* --- Register HW_GPMI_DEBUG, field UDMA_STATE */

#define BP_GPMI_DEBUG_UDMA_STATE      8
#define BM_GPMI_DEBUG_UDMA_STATE      0x00000F00

#define BF_GPMI_DEBUG_UDMA_STATE(v)   (((v) << 8) & BM_GPMI_DEBUG_UDMA_STATE)

/* --- Register HW_GPMI_DEBUG, field BUSY */

#define BP_GPMI_DEBUG_BUSY      7
#define BM_GPMI_DEBUG_BUSY      0x00000080

#define BF_GPMI_DEBUG_BUSY(v)   (((v) << 7) & BM_GPMI_DEBUG_BUSY)

#define BV_GPMI_DEBUG_BUSY__DISABLED  0x0
#define BV_GPMI_DEBUG_BUSY__ENABLED   0x1

/* --- Register HW_GPMI_DEBUG, field PIN_STATE */

#define BP_GPMI_DEBUG_PIN_STATE      4
#define BM_GPMI_DEBUG_PIN_STATE      0x00000070

#define BF_GPMI_DEBUG_PIN_STATE(v)   (((v) << 4) & BM_GPMI_DEBUG_PIN_STATE)

#define BV_GPMI_DEBUG_PIN_STATE__PSM_IDLE    0x0
#define BV_GPMI_DEBUG_PIN_STATE__PSM_BYTCNT  0x1
#define BV_GPMI_DEBUG_PIN_STATE__PSM_ADDR    0x2
#define BV_GPMI_DEBUG_PIN_STATE__PSM_STALL   0x3
#define BV_GPMI_DEBUG_PIN_STATE__PSM_STROBE  0x4
#define BV_GPMI_DEBUG_PIN_STATE__PSM_ATARDY  0x5
#define BV_GPMI_DEBUG_PIN_STATE__PSM_DHOLD   0x6
#define BV_GPMI_DEBUG_PIN_STATE__PSM_DONE    0x7

/* --- Register HW_GPMI_DEBUG, field MAIN_STATE */

#define BP_GPMI_DEBUG_MAIN_STATE      0
#define BM_GPMI_DEBUG_MAIN_STATE      0x0000000F

#define BF_GPMI_DEBUG_MAIN_STATE(v)   (((v) << 0) & BM_GPMI_DEBUG_MAIN_STATE)

#define BV_GPMI_DEBUG_MAIN_STATE__MSM_IDLE    0x0
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_BYTCNT  0x1
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_WAITFE  0x2
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_WAITFR  0x3
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_DMAREQ  0x4
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_DMAACK  0x5
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_WAITFF  0x6
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_LDFIFO  0x7
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_LDDMAR  0x8
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_RDCMP   0x9
#define BV_GPMI_DEBUG_MAIN_STATE__MSM_DONE    0xA



/*
 * HW_GPMI_VERSION - GPMI Version Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned STEP   : 16;
        unsigned MINOR  :  8;
        unsigned MAJOR  :  8;
    } B;
} hw_gpmi_version_t;
#endif

/*
 * constants & macros for entire HW_GPMI_VERSION register
 */
#define HW_GPMI_VERSION_ADDR      (REGS_GPMI_BASE + 0x000000D0)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_VERSION           (*(volatile hw_gpmi_version_t *) HW_GPMI_VERSION_ADDR)
#define HW_GPMI_VERSION_RD()      (HW_GPMI_VERSION.U)
#endif


/*
 * constants & macros for individual HW_GPMI_VERSION bitfields
 */
/* --- Register HW_GPMI_VERSION, field MAJOR */

#define BP_GPMI_VERSION_MAJOR      24
#define BM_GPMI_VERSION_MAJOR      0xFF000000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_VERSION_MAJOR(v)   ((((unsigned int) v) << 24) & BM_GPMI_VERSION_MAJOR)
#else
#define BF_GPMI_VERSION_MAJOR(v)   (((v) << 24) & BM_GPMI_VERSION_MAJOR)
#endif

/* --- Register HW_GPMI_VERSION, field MINOR */

#define BP_GPMI_VERSION_MINOR      16
#define BM_GPMI_VERSION_MINOR      0x00FF0000

#define BF_GPMI_VERSION_MINOR(v)   (((v) << 16) & BM_GPMI_VERSION_MINOR)

/* --- Register HW_GPMI_VERSION, field STEP */

#define BP_GPMI_VERSION_STEP      0
#define BM_GPMI_VERSION_STEP      0x0000FFFF

#define BF_GPMI_VERSION_STEP(v)   (((v) << 0) & BM_GPMI_VERSION_STEP)



/*
 * HW_GPMI_DEBUG2 - GPMI Debug2 Information Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned RDN_TAP           :  6;
        unsigned UPDATE_WINDOW     :  1;
        unsigned VIEW_DELAYED_RDN  :  1;
        unsigned SYND2GPMI_READY   :  1;
        unsigned SYND2GPMI_VALID   :  1;
        unsigned GPMI2SYND_READY   :  1;
        unsigned GPMI2SYND_VALID   :  1;
        unsigned SYND2GPMI_BE      :  4;
        unsigned RSVD1             : 16;
    } B;
} hw_gpmi_debug2_t;
#endif

/*
 * constants & macros for entire HW_GPMI_DEBUG2 register
 */
#define HW_GPMI_DEBUG2_ADDR      (0x8000c000 + 0x000000E0)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_DEBUG2           (*(volatile hw_gpmi_debug2_t *) HW_GPMI_DEBUG2_ADDR)
#define HW_GPMI_DEBUG2_RD()      (HW_GPMI_DEBUG2.U)
#define HW_GPMI_DEBUG2_WR(v)     (HW_GPMI_DEBUG2.U = (v))
#define HW_GPMI_DEBUG2_SET(v)    (HW_GPMI_DEBUG2_WR(HW_GPMI_DEBUG2_RD() |  (v)))
#define HW_GPMI_DEBUG2_CLR(v)    (HW_GPMI_DEBUG2_WR(HW_GPMI_DEBUG2_RD() & ~(v)))
#define HW_GPMI_DEBUG2_TOG(v)    (HW_GPMI_DEBUG2_WR(HW_GPMI_DEBUG2_RD() ^  (v)))
#endif


/*
 * constants & macros for individual HW_GPMI_DEBUG2 bitfields
 */
/* --- Register HW_GPMI_DEBUG2, field SYND2GPMI_BE */

#define BP_GPMI_DEBUG2_SYND2GPMI_BE      12
#define BM_GPMI_DEBUG2_SYND2GPMI_BE      0x0000F000

#define BF_GPMI_DEBUG2_SYND2GPMI_BE(v)   (((v) << 12) & BM_GPMI_DEBUG2_SYND2GPMI_BE)

/* --- Register HW_GPMI_DEBUG2, field GPMI2SYND_VALID */

#define BP_GPMI_DEBUG2_GPMI2SYND_VALID      11
#define BM_GPMI_DEBUG2_GPMI2SYND_VALID      0x00000800

#define BF_GPMI_DEBUG2_GPMI2SYND_VALID(v)   (((v) << 11) & BM_GPMI_DEBUG2_GPMI2SYND_VALID)

/* --- Register HW_GPMI_DEBUG2, field GPMI2SYND_READY */

#define BP_GPMI_DEBUG2_GPMI2SYND_READY      10
#define BM_GPMI_DEBUG2_GPMI2SYND_READY      0x00000400

#define BF_GPMI_DEBUG2_GPMI2SYND_READY(v)   (((v) << 10) & BM_GPMI_DEBUG2_GPMI2SYND_READY)

/* --- Register HW_GPMI_DEBUG2, field SYND2GPMI_VALID */

#define BP_GPMI_DEBUG2_SYND2GPMI_VALID      9
#define BM_GPMI_DEBUG2_SYND2GPMI_VALID      0x00000200

#define BF_GPMI_DEBUG2_SYND2GPMI_VALID(v)   (((v) << 9) & BM_GPMI_DEBUG2_SYND2GPMI_VALID)

/* --- Register HW_GPMI_DEBUG2, field SYND2GPMI_READY */

#define BP_GPMI_DEBUG2_SYND2GPMI_READY      8
#define BM_GPMI_DEBUG2_SYND2GPMI_READY      0x00000100

#define BF_GPMI_DEBUG2_SYND2GPMI_READY(v)   (((v) << 8) & BM_GPMI_DEBUG2_SYND2GPMI_READY)

/* --- Register HW_GPMI_DEBUG2, field VIEW_DELAYED_RDN */

#define BP_GPMI_DEBUG2_VIEW_DELAYED_RDN      7
#define BM_GPMI_DEBUG2_VIEW_DELAYED_RDN      0x00000080

#define BF_GPMI_DEBUG2_VIEW_DELAYED_RDN(v)   (((v) << 7) & BM_GPMI_DEBUG2_VIEW_DELAYED_RDN)

#ifndef __LANGUAGE_ASM__
//#define BW_GPMI_DEBUG2_VIEW_DELAYED_RDN(v)   BF_CS1(GPMI_DEBUG2, VIEW_DELAYED_RDN, v)
#endif

/* --- Register HW_GPMI_DEBUG2, field UPDATE_WINDOW */

#define BP_GPMI_DEBUG2_UPDATE_WINDOW      6
#define BM_GPMI_DEBUG2_UPDATE_WINDOW      0x00000040

#define BF_GPMI_DEBUG2_UPDATE_WINDOW(v)   (((v) << 6) & BM_GPMI_DEBUG2_UPDATE_WINDOW)

/* --- Register HW_GPMI_DEBUG2, field RDN_TAP */

#define BP_GPMI_DEBUG2_RDN_TAP      0
#define BM_GPMI_DEBUG2_RDN_TAP      0x0000003F

#define BF_GPMI_DEBUG2_RDN_TAP(v)   (((v) << 0) & BM_GPMI_DEBUG2_RDN_TAP)



/*
 * HW_GPMI_DEBUG3 - GPMI Debug3 Information Register
 */
#ifndef __LANGUAGE_ASM__
typedef union
{
    unsigned int  U;
    struct
    {
        unsigned DEV_WORD_CNTR  : 16;
        unsigned APB_WORD_CNTR  : 16;
    } B;
} hw_gpmi_debug3_t;
#endif

/*
 * constants & macros for entire HW_GPMI_DEBUG3 register
 */
#define HW_GPMI_DEBUG3_ADDR      (0x8000c000 + 0x000000F0)

#ifndef __LANGUAGE_ASM__
#define HW_GPMI_DEBUG3           (*(volatile hw_gpmi_debug3_t *) HW_GPMI_DEBUG3_ADDR)
#define HW_GPMI_DEBUG3_RD()      (HW_GPMI_DEBUG3.U)
#endif


/*
 * constants & macros for individual HW_GPMI_DEBUG3 bitfields
 */
/* --- Register HW_GPMI_DEBUG3, field APB_WORD_CNTR */

#define BP_GPMI_DEBUG3_APB_WORD_CNTR      16
#define BM_GPMI_DEBUG3_APB_WORD_CNTR      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_GPMI_DEBUG3_APB_WORD_CNTR(v)   ((((unsigned int) v) << 16) & BM_GPMI_DEBUG3_APB_WORD_CNTR)
#else
#define BF_GPMI_DEBUG3_APB_WORD_CNTR(v)   (((v) << 16) & BM_GPMI_DEBUG3_APB_WORD_CNTR)
#endif

/* --- Register HW_GPMI_DEBUG3, field DEV_WORD_CNTR */

#define BP_GPMI_DEBUG3_DEV_WORD_CNTR      0
#define BM_GPMI_DEBUG3_DEV_WORD_CNTR      0x0000FFFF

#define BF_GPMI_DEBUG3_DEV_WORD_CNTR(v)   (((v) << 0) & BM_GPMI_DEBUG3_DEV_WORD_CNTR)

typedef union
{
    unsigned int  U;
    struct
    {
        unsigned CMD_ADDR  : 32;
    } B;
} hw_apbh_chn_nxtcmdar_t;


#define HW_APBH_CHn_NXTCMDAR_COUNT        8
#define HW_APBH_CHn_NXTCMDAR_ADDR(n)      (0x80004000 + 0x00000050 + ((n) * 0x70))

#ifndef __LANGUAGE_ASM__
#define HW_APBH_CHn_NXTCMDAR(n)           (*(volatile hw_apbh_chn_nxtcmdar_t *) HW_APBH_CHn_NXTCMDAR_ADDR(n))
#define HW_APBH_CHn_NXTCMDAR_RD(n)        (HW_APBH_CHn_NXTCMDAR(n).U)
#define HW_APBH_CHn_NXTCMDAR_WR(n, v)     (HW_APBH_CHn_NXTCMDAR(n).U = (v))
#define HW_APBH_CHn_NXTCMDAR_SET(n, v)    (HW_APBH_CHn_NXTCMDAR_WR(n, HW_APBH_CHn_NXTCMDAR_RD(n) |  (v)))
#define HW_APBH_CHn_NXTCMDAR_CLR(n, v)    (HW_APBH_CHn_NXTCMDAR_WR(n, HW_APBH_CHn_NXTCMDAR_RD(n) & ~(v)))
#define HW_APBH_CHn_NXTCMDAR_TOG(n, v)    (HW_APBH_CHn_NXTCMDAR_WR(n, HW_APBH_CHn_NXTCMDAR_RD(n) ^  (v)))
#endif

typedef union
{
    unsigned int  U;
    struct
    {
        unsigned INCREMENT_SEMA  :  8;
        unsigned RSVD1           :  8;
        unsigned PHORE           :  8;
        unsigned RSVD2           :  8;
    } B;
} hw_apbh_chn_sema_t;

#define HW_APBH_CHn_SEMA_COUNT        8
#define HW_APBH_CHn_SEMA_ADDR(n)      (0x80004000 + 0x00000080 + ((n) * 0x70))

#ifndef __LANGUAGE_ASM__
#define HW_APBH_CHn_SEMA(n)           (*(volatile hw_apbh_chn_sema_t *) HW_APBH_CHn_SEMA_ADDR(n))
#define HW_APBH_CHn_SEMA_RD(n)        (HW_APBH_CHn_SEMA(n).U)
#define HW_APBH_CHn_SEMA_WR(n, v)     (HW_APBH_CHn_SEMA(n).U = (v))
#define HW_APBH_CHn_SEMA_SET(n, v)    (HW_APBH_CHn_SEMA_WR(n, HW_APBH_CHn_SEMA_RD(n) |  (v)))
#define HW_APBH_CHn_SEMA_CLR(n, v)    (HW_APBH_CHn_SEMA_WR(n, HW_APBH_CHn_SEMA_RD(n) & ~(v)))
#define HW_APBH_CHn_SEMA_TOG(n, v)    (HW_APBH_CHn_SEMA_WR(n, HW_APBH_CHn_SEMA_RD(n) ^  (v)))
#endif


/*
 * constants & macros for individual HW_APBH_CHn_SEMA multi-register bitfields
 */
/* --- Register HW_APBH_CHn_SEMA, field PHORE */

#define BP_APBH_CHn_SEMA_PHORE      16
#define BM_APBH_CHn_SEMA_PHORE      0x00FF0000

#define BF_APBH_CHn_SEMA_PHORE(v)   (((v) << 16) & BM_APBH_CHn_SEMA_PHORE)

/* --- Register HW_APBH_CHn_SEMA, field INCREMENT_SEMA */

#define BP_APBH_CHn_SEMA_INCREMENT_SEMA      0
#define BM_APBH_CHn_SEMA_INCREMENT_SEMA      0x000000FF

#define BF_APBH_CHn_SEMA_INCREMENT_SEMA(v)   (((v) << 0) & BM_APBH_CHn_SEMA_INCREMENT_SEMA)

#ifndef __LANGUAGE_ASM__
#define BW_APBH_CHn_SEMA_INCREMENT_SEMA(n, v)  (HW_APBH_CHn_SEMA(n).B.INCREMENT_SEMA = (v))
#endif

#endif  //_NAND_DMA_DESCRIPTOR_H

