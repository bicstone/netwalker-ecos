#include <redboot.h>
#include <stdlib.h>
#include <cyg/diagnosis/diagnosis.h>
#include <cyg/hal/plf_io.h>

#include CYGHWR_MEMORY_LAYOUT_H

#define WDT_WCR 	0x00
#define WDT_WSR 	0x02

#define WDT_CNT_MASK	0xFF00
#define WDT_WCR_MASK	0xFF
#define WDT_CNT_OFF	8

#define WDT_WCR_WDW	(1 << 7)
#define WDT_WCR_WOE	(1 << 6)
#define WDT_WCR_WDA	(1 << 5)
#define WDT_WCR_SRS	(1 << 4)
#define WDT_WCR_WDT	(1 << 3)
#define WDT_WCR_WDE	(1 << 2)
#define WDT_WCR_WDBG	(1 << 1)
#define WDT_WCR_WDZST	(1)

#define WDT_MAGIC_1	(0x5555)
#define WDT_MAGIC_2	(0xAAAA)

local_cmd_entry("wdt",
	"watchdog test:Warning after run test, please reboot",
	"-s sleep_time -t timeout [-c ctrl_bits] -b\n"
	"-b:Insert memory access during ping operation\n",
	wdt_test,
	DIAGNOSIS_cmds
);

static unsigned char wdt_wcr;

static unsigned int wdt_ping_mode;

static inline void wdt_setup(unsigned int timeout)
{
	unsigned short int reg;
	reg = readw(WDOG_BASE_ADDR + WDT_WCR) & WDT_WCR_MASK;	
	reg |= (timeout * 2 << WDT_CNT_OFF) & WDT_CNT_MASK;

	if (wdt_wcr)
		reg |= wdt_wcr & WDT_WCR_MASK;
	else
		reg |= WDT_WCR_WOE | WDT_WCR_WDA | WDT_WCR_SRS|
			WDT_WCR_WDBG | WDT_WCR_WDZST;

	diag_printf("WCR=%x\n", reg | WDT_WCR_WDE);	
	writew(reg | WDT_WCR_WDE, WDOG_BASE_ADDR + WDT_WCR);
}

static inline void wdt_stop(void)
{
	unsigned short int reg;
	reg = readw(WDOG_BASE_ADDR + WDT_WCR) & (~WDT_WCR_WDE);	
	reg |= WDT_CNT_MASK;
	writew(reg, WDOG_BASE_ADDR + WDT_WCR);
}

static inline void wdt_keepalive(void)
{
	int j;
	volatile unsigned int i;

	if (wdt_ping_mode) {
		writew(WDT_MAGIC_1, WDOG_BASE_ADDR + WDT_WSR);
		for (i = 0, j &= 0x7; i <= j; i++)
			asm("nop");
	} else {
		writew(WDT_MAGIC_1, WDOG_BASE_ADDR + WDT_WSR);
	}
	writew(WDT_MAGIC_2, WDOG_BASE_ADDR + WDT_WSR);
}

static void wdt_sleep(int second)
{
	int i;
	unsigned int delayCount = 32000;

	for ( i = 0; i < second; i++) {	
		writel(0x01, EPIT_BASE_ADDR + EPITSR);
		writel(delayCount, EPIT_BASE_ADDR + EPITLR);
		while ((0x1 & readl(EPIT_BASE_ADDR + EPITSR)) == 0);
	}
}

static void wdt_test(int argc, char * argv[])
{
	int opts_map[4];
	struct option_info opts[4];
	unsigned int sleep_timeout;
	unsigned int wdt_timeout;

	memset(opts_map, 0, sizeof(int)*4);

	init_opts(&opts[0], 's', true, OPTION_ARG_TYPE_NUM,
		 (void *)&sleep_timeout, (bool *)&opts_map[0], "sleep time");
	init_opts(&opts[1], 't', true, OPTION_ARG_TYPE_NUM,
		 (void *)&wdt_timeout, (bool *)&opts_map[1], "watchdog timeout");
	init_opts(&opts[2], 'c', true, OPTION_ARG_TYPE_NUM,
		 (void *)&wdt_wcr, (bool *)&opts_map[2], "watchdog control bits");
	init_opts(&opts[3], 'b', false, OPTION_ARG_TYPE_FLG,
		 (void *)&wdt_ping_mode, (bool *)0, "Add nop between ping operation");

	if (!scan_opts(argc, argv, 2, opts, 4, 0, 0, 0)) {
		diagnosis_usage("invalid arguments");
		return;
	}

	if(!opts_map[0]) {
		 sleep_timeout = 1;
	} 

	if(!opts_map[1]) {
		wdt_timeout = 2;
	}

	if(!opts_map[2]) {
		wdt_wcr = 0;
	}

	diag_printf("Watchdog sleeptime=%d timeout=%d %s in ping", 
			sleep_timeout, wdt_timeout,
			wdt_ping_mode?"Add memory access":"No memory access");
	wdt_setup(wdt_timeout);
	
	while(1) {
		if(_rb_break(0)) {
			diag_printf("break Watchdog test\n");
			wdt_sleep(wdt_timeout*2);
			break;
		}
		wdt_keepalive();
		wdt_sleep(sleep_timeout);
	}
	wdt_stop(); 
	diag_printf("Exit Watchdog test\n");
}
