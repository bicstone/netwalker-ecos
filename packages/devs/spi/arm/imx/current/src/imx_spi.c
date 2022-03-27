#include <redboot.h>
#include <stdlib.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>

#include <cyg/hal/fsl_board.h>
#include <cyg/io/imx_spi.h>

/*!
 * Initialization function for a spi slave device. It must be called BEFORE
 * any spi operations. The SPI module will be -disabled- after this call.
 */
int imx_spi_init_v2_3 (struct imx_spi_dev *dev)
{
    unsigned int clk_src = get_peri_clock(dev->base);
    unsigned int pre_div = 0, post_div = 0, i, reg_ctrl, reg_config;
    struct spi_v2_3_reg *reg = (struct spi_v2_3_reg *)dev->reg;

    if (dev->freq == 0) {
        diag_printf("Error: desired clock is 0\n");
        return -1;
    }
    // iomux config
    io_cfg_spi(dev);

    reg_ctrl = readl(dev->base + 0x8);
    // reset the spi
    writel(0, dev->base + 0x8);
    writel(reg_ctrl | 0x1, dev->base + 0x8);

    // control register setup
    if (clk_src > dev->freq) {
        pre_div = clk_src / dev->freq;
        if (pre_div > 16) {
            post_div = pre_div / 16;
            pre_div = 15;
        }
        if (post_div != 0) {
            for (i = 0; i < 16; i++) {
                if ((1 << i) >= post_div)
                    break;
            }
            if (i == 16) {
                diag_printf("Error: no divider can meet the freq: %d\n",
                            dev->freq);
                return -1;
            }
            post_div = i;
        }
    }
    diag_printf1("pre_div = %d, post_div=%d\n", pre_div, post_div);
    reg_ctrl = (reg_ctrl & ~(3 << 18)) | dev->ss << 18;
    reg_ctrl = (reg_ctrl & ~(0xF << 12)) | pre_div << 12;
    reg_ctrl = (reg_ctrl & ~(0xF << 8)) | post_div << 8;
    reg_ctrl |= 1 << (dev->ss + 4);     // always set to master mode !!!!
    reg_ctrl &= ~0x1;                   // disable spi

    reg_config = readl(dev->base + 0xC);
    // configuration register setup
    reg_config = (reg_config & ~(1 << ((dev->ss + 12)))) |
        (dev->ss_pol << (dev->ss + 12));
    reg_config = (reg_config & ~(1 << ((dev->ss + 20)))) |
        (dev->in_sctl << (dev->ss + 20));
    reg_config = (reg_config & ~(1 << ((dev->ss + 16)))) |
        (dev->in_dctl << (dev->ss + 16));
    reg_config = (reg_config & ~(1 << ((dev->ss + 8)))) |
        (dev->ssctl << (dev->ss + 8));
    reg_config = (reg_config & ~(1 << ((dev->ss + 4)))) |
        (dev->sclkpol << (dev->ss + 4));
    reg_config = (reg_config & ~(1 << ((dev->ss + 0)))) |
        (dev->sclkpha << (dev->ss + 0));

    diag_printf1("reg_ctrl = 0x%x\n", reg_ctrl);
    writel(reg_ctrl, dev->base + 0x8);
    diag_printf1("reg_config = 0x%x\n", reg_config);
    writel(reg_config, dev->base + 0xC);
    // save config register and control register
    reg->cfg_reg = reg_config;
    reg->ctrl_reg = reg_ctrl;

    // clear interrupt reg
    writel(0, dev->base + 0x10);
    writel(3 << 6, dev->base + 0x18);

    return 0;
}

/*!
 * This function should only be called after the imx_spi_init_xxx().
 * It sets up the spi module according to the initialized value and then
 * enables the SPI module. This function is called by the xfer function.
 *
 * Note: If one wants to change the SPI parameters such as clock, the
 *       imx_spi_init_xxx() needs to be called again.
 */
static void spi_start_v2_3(struct imx_spi_dev *dev,
                           struct spi_v2_3_reg *reg, int len)
{
    if (reg->ctrl_reg == 0) {
        diag_printf("Error: spi(base=0x%x) has not been initialized yet\n",
                    dev->base);
        return;
    }
    // iomux config
    io_cfg_spi(dev);
    reg->ctrl_reg = (reg->ctrl_reg & ~0xFFF00000) | ((len * 8 - 1) << 20);

    writel(reg->ctrl_reg | 0x1, dev->base + 0x8);
    writel(reg->cfg_reg, dev->base + 0xC);
    diag_printf1("ctrl_reg=0x%x, cfg_reg=0x%x\n",
                 readl(dev->base + 0x8), readl(dev->base + 0xC));
}

/*!
 * Stop the SPI module that the slave device is connected to.
 */
static void spi_stop_v2_3(struct imx_spi_dev *dev)
{
    writel(0, dev->base + 0x8);
}

/*!
 * Transfer up to burst_bytes bytes data via spi. The amount of data
 * is the sum of both the tx and rx.
 * After this call, the SPI module that the slave is connected to will
 * be -disabled- again.
 */
int imx_spi_xfer_v2_3 (
    struct imx_spi_dev *dev,    // spi device pointer
    unsigned char *tx_buf,      // tx buffer (has to be 4-byte aligned)
    unsigned char *rx_buf,      // rx buffer (has to be 4-byte aligned)
    int burst_bytes             // total number of bytes in one burst (or xfer)
    )
{
    int val = SPI_RETRY_TIMES;
    unsigned int *p_buf;
    unsigned int reg;
    int len, ret_val = 0;

    if (burst_bytes > dev->fifo_sz) {
        diag_printf("Error: maximum burst size is 0x%x bytes, asking 0x%x\n",
                    dev->fifo_sz, burst_bytes);
        return -1;
    }

    spi_start_v2_3(dev, dev->reg, burst_bytes);

    // move data to the tx fifo
    for (p_buf = (unsigned int *)tx_buf, len = burst_bytes; len > 0;
         p_buf++, len -= 4) {
        writel(*p_buf, dev->base + 0x4);
    }
    reg = readl(dev->base + 0x8);
    reg |= (1 << 2); // set xch bit
    diag_printf1("control reg = 0x%08x\n", reg);
    writel(reg, dev->base + 0x8);

    // poll on the TC bit (transfer complete)
    while ((val-- > 0) && (readl(dev->base + 0x18) & (1 << 7)) == 0) {
        if (dev->us_delay != 0) {
            hal_delay_us(dev->us_delay);
        }
    }

    // clear the TC bit
    writel(3 << 6, dev->base + 0x18);
    if (val == 0) {
        diag_printf("Error: re-tried %d times without response. Give up\n", SPI_RETRY_TIMES);
        ret_val = -1;
        goto error;
    }

    // move data in the rx buf
    for (p_buf = (unsigned int *)rx_buf, len = burst_bytes; len > 0;
         p_buf++, len -= 4) {
        *p_buf = readl(dev->base + 0x0);
    }
error:
    spi_stop_v2_3(dev);
    return ret_val;
}

#ifdef PMIC_SPI_BASE
extern imx_spi_init_func_t *spi_pmic_init;
extern imx_spi_xfer_func_t *spi_pmic_xfer;
extern struct imx_spi_dev imx_spi_pmic;

static void show_pmic_info(void)
{
    volatile unsigned int rev_id;

    spi_pmic_init(&imx_spi_pmic);
    rev_id = pmic_reg(7, 0, 0);
    diag_printf("PMIC ID: 0x%08x [Rev: ", rev_id);
    switch (rev_id & 0x1F) {
    case 0x1:
        diag_printf("1.0");
        break;
    case 0x9:
        diag_printf("1.1");
        break;
    case 0xA:
        diag_printf("1.2");
        break;
    case 0x10:
        if (((rev_id >> 9) & 0x3) == 0) {
        diag_printf("2.0");
        } else {
            diag_printf("2.0a");
        }
        break;
    case 0x11:
        diag_printf("2.1");
        break;
    case 0x18:
        diag_printf("3.0");
        break;
    case 0x19:
        diag_printf("3.1");
        break;
    case 0x1A:
        diag_printf("3.2");
        break;
    case 0x2:
        diag_printf("3.2A");
        break;
    case 0x1B:
        diag_printf("3.3");
        break;
    case 0x1D:
        diag_printf("3.5");
        break;
    default:
        diag_printf("unknown");
        break;
    }
    diag_printf("]\n");
}

RedBoot_init(show_pmic_info, RedBoot_INIT_PRIO(100));

static void do_pmic(int argc, char *argv[]);
RedBoot_cmd("pmic",
            "Read/Write internal PMIC register",
            "<reg num> [value to be written]",
            do_pmic
           );

static void do_pmic(int argc, char *argv[])
{
    unsigned int reg, temp, val = 0, write = 0;

    if (argc == 1) {
        diag_printf("\tRead:  pmic <reg num>\n");
        diag_printf("\tWrite: pmic <reg num> <value to be written>\n");
        return;
    }

    if (!parse_num(*(&argv[1]), (unsigned long *)&reg, &argv[1], ":")) {
        diag_printf("Error: Invalid parameter\n");
        return;
    }

    if (argc == 3) {
        if (!parse_num(*(&argv[2]), (unsigned long *)&val, &argv[2], ":")) {
            diag_printf("Error: Invalid parameter\n");
            return;
        }
        write = 1;
    }

    spi_pmic_init(&imx_spi_pmic);
    temp = pmic_reg(reg, val, write);

    diag_printf("\tval: 0x%08x\n\n", temp);
}

static unsigned int pmic_tx, pmic_rx;
/*!
 * To read/write to a PMIC register. For write, it does another read for the
 * actual register value.
 *
 * @param   reg         register number inside the PMIC
 * @param   val         data to be written to the register; don't care for read
 * @param   write       0 for read; 1 for write
 *
 * @return              the actual data in the PMIC register
 */
unsigned int pmic_reg(unsigned int reg, unsigned int val, unsigned int write)
{
    if (reg > 63 || write > 1 ) {
        diag_printf("<reg num> = %d is invalide. Should be less then 63\n", reg);
        return 0;
    }
    pmic_tx = (write << 31) | (reg << 25) | (val & 0x00FFFFFF);
    diag_printf1("reg=0x%x, val=0x%08x\n", reg, pmic_tx);

    spi_pmic_xfer(&imx_spi_pmic, (unsigned char *)&pmic_tx,
                  (unsigned char *)&pmic_rx, 4);

    if (write) {
        pmic_tx &= ~(1 << 31);
        spi_pmic_xfer(&imx_spi_pmic, (unsigned char *)&pmic_tx,
                      (unsigned char *)&pmic_rx, 4);
    }

    return pmic_rx;
}
#endif // PMIC_SPI_BASE

#ifdef CPLD_SPI_BASE

unsigned int spi_cpld_xchg_single(unsigned int data, unsigned int data1, unsigned int base)
{
    volatile unsigned int cfg_reg = readl(base + SPI_CTRL_REG_OFF);
    unsigned int temp;

    /* Activate the SS signal */
    cfg_reg |= CPLD_SPI_CHIP_SELECT_NO;
    writel(cfg_reg, CPLD_SPI_BASE + SPI_CTRL_REG_OFF);

    /* Write the data */
    writel(data, base + SPI_TX_REG_OFF);
    writel(data1, base + SPI_TX_REG_OFF);

    cfg_reg |= SPI_CTRL_REG_XCH_BIT;
    writel(cfg_reg, base + SPI_CTRL_REG_OFF);

    while ((((cfg_reg = readl(base + SPI_TEST_REG_OFF)) &
              SPI_TEST_REG_RXCNT_MASK) >> SPI_TEST_REG_RXCNT_OFFSET) != 2) {
    }

    /* Deactivate the SS signal */
    cfg_reg = readl(base + SPI_CTRL_REG_OFF);
    cfg_reg &= ~SPI_CTRL_CS_MASK;
    writel(cfg_reg, base + SPI_CTRL_REG_OFF);

    /* Read from RX FIFO, second entry contains the data */
    temp = readl(base + SPI_RX_REG_OFF);
    temp = readl(base + SPI_RX_REG_OFF);
    return ((temp >> 6) & 0xffff);
}

static void mxc_cpld_spi_init(void)
{
    unsigned int ctrl;

    ctrl = SPI_CTRL_REG_BIT_COUNT46 | CPLD_SPI_CTRL_MODE_MASTER | SPI_CTRL_EN;

    spi_init(CPLD_SPI_BASE, 18000000,      // 54MHz data rate
             ctrl);
}

RedBoot_init(mxc_cpld_spi_init, RedBoot_INIT_PRIO(102));

static void do_cpld(int argc, char *argv[]);

RedBoot_cmd("spi_cpld",
            "Read/Write 16-bit internal CPLD register over CSPI",
            "<reg num> [16-bit value to be written]",
            do_cpld
           );

static void do_cpld(int argc,char *argv[])
{
    unsigned int reg, temp, val = 0, read = 1;

    if (argc == 1) {
        diag_printf("\tRead:  spi_cpld <reg num>\n");
        diag_printf("\tWrite: spi_cpld <reg num> <value to be written>\n");
        return;
    }

    if (!parse_num(*(&argv[1]), (unsigned long *)&reg, &argv[1], ":")) {
        diag_printf("Error: Invalid parameter\n");
        return;
    }

    if (argc == 3) {
        if (!parse_num(*(&argv[2]), (unsigned long *)&val, &argv[2], ":")) {
            diag_printf("Error: Invalid parameter\n");
            return;
        }
        read = 0;
    }

    mxc_cpld_spi_init();
    temp = cpld_reg(reg, val, read);

    diag_printf("\tval: 0x%04x\n\n", temp);
}

/*!
 * To read/write to a CPLD register.
 *
 * @param   reg         register number inside the CPLD
 * @param   val         data to be written to the register; don't care for read
 * @param   read        0 for write; 1 for read
 *
 * @return              the actual data in the CPLD register
 */
unsigned int cpld_reg_xfer(unsigned int reg, unsigned int val, unsigned int read)
{
    unsigned int local_val1, local_val2;

    reg >>= 1;

    local_val1 = (read << 13) | ((reg & 0x0001FFFF) >> 5) | 0x00001000;
    if (read) {
        //local_val1 = (read << 22) | (reg << 4) | 0x00200004;
        //local_val2 = 0x1F;
        local_val2 = ( ((reg & 0x0000001F) << 27) | 0x0200001f);

    } else {
        //local_val1 = (read << 22) | (reg << 4) | 0x00200007;
        //local_val2 = ((val & 0xFFFF) << 6) | 0x00400027;
        local_val2 = ( ((reg & 0x0000001F) << 27) | ((val & 0x0000FFFF) << 6) | 0x03C00027);

    }

    diag_printf1("reg=0x%x, val=0x%08x\n", reg, val);
    return spi_cpld_xchg_single(local_val1, local_val2, CPLD_SPI_BASE);
}

/*!
 * To read/write to a CPLD register. For write, it does another read for the
 * actual register value.
 *
 * @param   reg         register number inside the CPLD
 * @param   val         data to be written to the register; don't care for read
 * @param   read        0 for write; 1 for read
 *
 * @return              the actual data in the CPLD register
 */
unsigned int cpld_reg(unsigned int reg, unsigned int val, unsigned int read)
{
    unsigned int temp;

    if (reg > 0x20068 || read > 1 ) {
        diag_printf("<reg num> = %x is invalid. Should be less then 0x20068\n", reg);
        return 0;
    }

    temp = cpld_reg_xfer(reg, val, read);
    diag_printf1("reg=0x%x, val=0x%08x\n", reg, val);

    if (read == 0) {
        temp = cpld_reg_xfer(reg, val, 1);
    }

    return temp;
}

#endif // CPLD_SPI_BASE
