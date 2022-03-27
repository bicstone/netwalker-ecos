#ifndef CARD_MX32_H
#define CARD_MX32_H

#include <cyg/infra/cyg_type.h>

/*sdhc memory map*/
typedef struct _sdhc
{
    cyg_uint32 sdhc_clk;
    cyg_uint32 sdhc_status;
    cyg_uint32 sdhc_clk_rate;
    cyg_uint32 sdhc_dat_cont;
    cyg_uint32 sdhc_response_to;
    cyg_uint32 sdhc_read_to;
    cyg_uint32 sdhc_blk_len;
    cyg_uint32 sdhc_nob;
    cyg_uint32 sdhc_rev_no;
    cyg_uint32 sdhc_int_cntr;
    cyg_uint32 sdhc_cmd;
    cyg_uint32 sdhc_arg;
    cyg_uint32 sdhc_reserved;
    cyg_uint32 sdhc_res_fifo;
    cyg_uint32 sdhc_buffer_access;
}sdhc_t, *psdhc_t;

/* Defines for card types */
typedef enum
{
	TYPE_NONE,
	SD_CSD_1_0,
	SD_CSD_2_0,
	MMC_CSD_1_0,
	MMC_CSD_1_1,
	MMC_CSD_1_2,
	MMC_UNKNOWN
}card_type;

typedef struct _card_specific_data
{
	cyg_uint32 csd0;
	cyg_uint32 csd1;
	cyg_uint32 csd2;
	cyg_uint32 csd3;
}CARD_SPECIFIC_DATA;

/* Defines for card types */
typedef struct _card_id
{
	cyg_uint32 cid0;
	cyg_uint32 cid1;
	cyg_uint32 cid2;
	cyg_uint32 cid3;
}CARD_ID;

enum sdhc_clk_val
{
    SDHC_CLK_START = 0x2,
    SDHC_CLK_STOP = 0x1,
    SDHC_CLK_RESET = 0x8
};

typedef enum frequency_mode
{
	iden_mode = 0x1,
	trans_mode = 0x2
} frequency_mode_t;

typedef struct command
{
	cyg_uint32 index;
	cyg_uint32 data_control;
	cyg_uint32 arg;
}command_t;


#define NO_ARG 0
#define ENABLE 1
#define DISABLE 0
#define PASS 0
#define SUCCESS 0
#define FAIL 1

#define CARD_STATE 0x1E00
#define CARD_STATE_SHIFT 9

/*Defines of CSD data*/
#define CSD_STRUCT_MSK 0x00C00000
#define CSD_STRUCT_SHIFT	22


/* Define the states of the card*/
enum states
{
	IDLE,
	READY,
	IDENT,
	STBY,
	TRAN,
	DATA,
	RCV,
	PRG,
	DIS
};


/* SDHC Response */
typedef struct _response
{
    cyg_uint32 rsp0;
    cyg_uint32 rsp1;
    cyg_uint32 rsp2;
    cyg_uint32 rsp3;
}response_t;


typedef enum card_mode
{
	NONE = 0,
	SD = 1,
	MMC = 2
}card_mode_t;

enum RW
{
	READ = 0,
	WRITE = 1
};

enum cmd_response
{
	RESPONSE_NO = 0x0,
	RESPONSE_48_CRC = 0x1,
	RESPONSE_136 = 0x2,
	RESPONSE_48_WITHOUT_CRC = 0x3
};

enum status_bus_width
{
	ONE = 0x0,
	FOUR = 0x2
};


#define SDHC_INT                  0xc015

#define OCR_VALUE 0x80ff8000
#define OCR_VALUE_MASK 0x00ff8000
#define CARD_BUSY 0x80000000
#define SD_R1_APP_CMD_MSK 0x20

#define BLOCK_LEN 0x200



/* Status regsiter Masks */
#define SDHC_STATUS_END_CMD_RESP_MSK          0x2000
#define SDHC_STATUS_WRITE_OP_DONE_MSK         0x1000
#define SDHC_STATUS_READ_OP_DONE_MSK          0x800
#define SDHC_STATUS_WR_CRC_ERR_CODE_MSK       0x600
#define SDHC_STATUS_CARD_BUS_CLK_RUN_MSK               0x100
#define SDHC_STATUS_RESP_CRC_ERR_MSK          0x20
#define SDHC_STATUS_BUF_READ_RDY_MSK          0x80
#define SDHC_STATUS_BUF_WRITE_RDY_MSK         0x40
#define SDHC_STATUS_READ_CRC_ERR_MSK          0x8
#define SDHC_STATUS_WRITE_CRC_ERR_MSK         0x4
#define SDHC_STATUS_TIME_OUT_RESP_MSK         0x2
#define SDHC_STATUS_TIME_OUT_READ             0x1

#define SDHC_STATUS_CLEAR                     ((cyg_uint32)(0xC0007E2F))



/* Command (data control) masks */
#define SDHC_CMD_FROMAT_OF_RESP      0x00000007
#define SDHC_CMD_DATA_ENABLE         0x00000008
#define SDHC_CMD_WRITE_READ          0x00000010
#define SDHC_CMD_INIT                0x00000080
#define SDHC_CMD_BUS_WIDTH           0x00000300
#define SDHC_CMD_START_READWAIT     0x00000400
#define SDHC_CMD_STOP_READWAIT      0x00000800
#define SDHC_CMD_DATA_CTRL_CMD_RESP_LONG_OFF   0x00001000

/* Command (data control) shift */
#define SDHC_CMD_FROMAT_OF_RESP_SHIFT     0x0
#define SDHC_CMD_DATA_ENABLE_SHIFT        0x3
#define SDHC_CMD_BUS_WIDTH_SHIFT          0x8
#define SDHC_CMD_WRITE_READ_SHIFT         0x4
#define SDHC_CMD_INIT_SHIFT               0x7

//#define SDHC_CMD_FROMAT_OF_RESP_NONE      0x0
//#define SDHC_CMD_DATA_CTRL_FROMAT_OF_RESP_48        0x1
//#define SDHC_CMD_DATA_CTRL_FROMAT_OF_RESP_136       0x2
//#define SDHC_CMD_DATA_CTRL_FROMAT_OF_RESP_48_N0_CRC 0x3
//#define SDHC_CMD_DATA_CTRL_BUS_WIDTH_1_BIT          0x0
//#define SDHC_CMD_DATA_CTRL_BUS_WIDTH_4_BIT          0x2

/* Define  each command */
enum commands
{
	CMD0= 0,
	CMD1= 1,
	CMD2= 2,
	CMD3= 3,
	CMD5= 5,
	CMD6=6,
	ACMD6= 6,
	CMD7= 7,
	CMD8 = 8,
	CMD9=9,
	CMD12   = 12,
	CMD13   = 13,
	CMD16   = 16,
	CMD17   = 17,
	CMD18   = 18,
	CMD24   = 24,
	CMD25   = 25,
	CMD26   = 26,
	CMD32   = 32,
	CMD33   = 33,
	CMD35   = 35,
	CMD36   = 36,
	CMD38   = 38,
	ACMD41  = 41,
	ACMD51  = 51,
	CMD55   = 55
};

extern cyg_uint32 CCC; /* Card Command Class */

extern cyg_uint32 mxcmci_init (cyg_uint32 bus_width, cyg_uint32 base_address);
extern cyg_uint32 mmc_data_write (cyg_uint32 *src_ptr,cyg_uint32 length,cyg_uint32 offset);
extern cyg_uint32 mmc_data_erase (cyg_uint32 offset, cyg_uint32 size);
extern cyg_uint32 mmc_data_read (cyg_uint32 *,cyg_uint32 ,cyg_uint32);
extern cyg_uint32 card_flash_query(void* data);
extern cyg_uint32 card_get_capacity_size (void);

struct csd_v1_0 {
	cyg_uint32 rsv3:1,
        crc:7,
        rsv2:2,
        file_format:2,
        tmp_write_protect:1,
        perm_write_protect:1,
        copy:1,
        file_format_grp:1,
        rsv1:5,
        write_bl_partial:1,
        write_bl_len:4,
        r2w_factor:3,
        rsv0:2,
        wp_grp_enable:1;
	cyg_uint32 wp_grp_size:7,
        sector_size:7,
        erase_blk_en:1,
        c_size_mult:3,
        vdd_w_curr_max:3,
        vdd_w_curr_min:3,
        vdd_r_curr_max:3,
        vdd_r_curr_min:3,
        c_size_lo:2;
    cyg_uint32 c_size_up:10,
        rsv4:2,
        dsr_imp:1,
        read_blk_misalign:1,
        write_blk_misalign:1,
        read_bl_partial:1,
        read_bl_len:4,
        ccc:12;
    cyg_uint32 tran_speed:8,
        nsac:8,
        taac:8,
        rsv5:6,
        csd_structure:2;
} __attribute__ ((packed));

struct csd_v2_0 {
	cyg_uint32
        rsv3:1,
        crc:7,
        rsv2:2,
        file_format:2,
        tmp_write_protect:1,
        perm_write_protect:1,
        copy:1,
        file_format_grp:1,
        rsv1:5,
        write_bl_partial:1,
        write_bl_len:4,
        r2w_factor:3,
        rsv0:2,
        wp_grp_enable:1;
	cyg_uint32
        wp_grp_size:7,
        sector_size:7,
        erase_blk_en:1,
        rsv9:1,
        c_size_lo:16;
    cyg_uint32
        c_size_up:6,
        rsv4:6,
        dsr_imp:1,
        read_blk_misalign:1,
        write_blk_misalign:1,
        read_bl_partial:1,
        read_bl_len:4,
        ccc:12;
    cyg_uint32 tran_speed:8,
        nsac:8,
        taac:8,
        rsv5:6,
        csd_structure:2;
} __attribute__ ((packed));

#endif
