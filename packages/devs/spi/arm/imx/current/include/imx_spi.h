#ifndef __IMX_SPI_H__
#define __IMX_SPI_H__

#undef IMX_SPI_DEBUG
//#define IMX_SPI_DEBUG

#ifdef IMX_SPI_DEBUG
#define diag_printf1    diag_printf
#else
#define diag_printf1(fmt,args...)
#endif

#define IMX_SPI_ACTIVE_HIGH     1
#define IMX_SPI_ACTIVE_LOW      0
#define SPI_RETRY_TIMES         100

// Only for SPI master support
struct imx_spi_dev {
    unsigned int base;      // base address of SPI module the device is connected to
    unsigned int freq;      // desired clock freq in Hz for this device
    unsigned int ss_pol;    // ss polarity: 1=active high; 0=active low
    unsigned int ss;        // slave select
    unsigned int in_sctl;   // inactive sclk ctl: 1=stay low; 0=stay high
    unsigned int in_dctl;   // inactive data ctl: 1=stay low; 0=stay high
    unsigned int ssctl;     // single burst mode vs multiple: 0=single; 1=multi
    unsigned int sclkpol;   // sclk polarity: active high=0; active low=1
    unsigned int sclkpha;   // sclk phase: 0=phase 0; 1=phase1
    unsigned int fifo_sz;   // fifo size in bytes for either tx or rx. Don't add them up!
    unsigned int us_delay;  // us delay in each xfer
    void *reg;              // pointer to a set of SPI registers
};

struct spi_v2_3_reg {
    unsigned int ctrl_reg;
    unsigned int cfg_reg;
};

// setup IOMUX for the spi device
// 
int imx_spi_init_v2_3 (
    struct imx_spi_dev *dev
    );

// transfer up to fifo bytes data via spi. The data transferred is the sum of both the tx and rx
int imx_spi_xfer_v2_3 (
    struct imx_spi_dev *dev,    // spi device pointer
    unsigned char *tx_buf,      // tx buffer (has to be 4-byte aligned)
    unsigned char *rx_buf,      // rx buffer (has to be 4-byte aligned)
    int burst_bytes             // total number of bytes in one burst or xfer
    );

typedef int imx_spi_init_func_t(struct imx_spi_dev *);
typedef int imx_spi_xfer_func_t(struct imx_spi_dev *, unsigned char *, unsigned char *, int);

unsigned int pmic_reg(unsigned int reg, unsigned int val, unsigned int write);

void io_cfg_spi(struct imx_spi_dev *dev);

#define REV_ATLAS_LITE_1_0         0x8
#define REV_ATLAS_LITE_1_1         0x9
#define REV_ATLAS_LITE_2_0         0x10
#define REV_ATLAS_LITE_2_1         0x11

#endif // __IMX_SPI_H__
