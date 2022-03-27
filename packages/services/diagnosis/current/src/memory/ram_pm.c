#include <redboot.h>
#include <stdlib.h>
#include <cyg/diagnosis/diagnosis.h>
#include <cyg/hal/plf_io.h>

#include CYGHWR_MEMORY_LAYOUT_H

#define DEFAULT_TEST_TIME	2
#define MIN_TEST_TIME	1
#define MAX_TEST_TIME	20

#define DEFAULT_BLOCK_SIZE	4096
#define MAX_BLOCK_SIZE		32768
#define MIN_BLOCK_SIZE		1024

local_cmd_entry("ram_pm",
	"ram performance test",
	"-t time -b size [-m mode]\n"
	"    -t time: set test time\n"
	"    -b size: set block size < 8192\n"
	"    -m mode: set 0:read, 1:write, 2:copy\n",
	ram_pm_test,
	DIAGNOSIS_cmds
);

enum {
	RAM_PM_READ = 0,
	RAM_PM_WRITE,
	RAM_PM_COPY,
	RAM_PM_MAX,
};

static char * mode_str[RAM_PM_MAX] =
{
	"read",
	"write",
	"copy"
};

static inline void start_timer(int second)
{
	unsigned int reg;
	reg = readl(CCM_BASE_ADDR + CLKCTL_CGR1);
	writel(reg | 0x30, CCM_BASE_ADDR + CLKCTL_CGR1);

	reg = readl(GPT_BASE_ADDR + GPTCR) | 0x8002;
	writel(reg&(~1), GPT_BASE_ADDR + GPTCR);
	while(readl(GPT_BASE_ADDR + GPTCR) & 0x8000);

	reg = second * 1000;
	writel(reg, GPT_BASE_ADDR + GPTOCR1);
	writel(0, GPT_BASE_ADDR + GPTIR);
	writel(0, GPT_BASE_ADDR + GPTCNT);
	writel(31, GPT_BASE_ADDR + GPTPR);
	writel(0x3F, GPT_BASE_ADDR + GPTSR);

	reg = readl(GPT_BASE_ADDR + GPTCR) | 0x303;
	writel(reg, GPT_BASE_ADDR + GPTCR);
}

static inline int get_time(int * cycles)
{
	*cycles = readl(GPT_BASE_ADDR + GPTCNT);
	if (readl(GPT_BASE_ADDR + GPTSR)& 1)
	 	*cycles -= readl(GPT_BASE_ADDR + GPTOCR1);
	return readl(GPT_BASE_ADDR + GPTSR) & 1;
}

static inline void stop_timer(void)
{
	unsigned int reg;
	reg = readl(GPT_BASE_ADDR + GPTCR) | 0x8000;
	writel(reg&(~1), GPT_BASE_ADDR + GPTCR);
}

static int performance_read_write(int time, int bsize, int read)
{
	unsigned long start = CYGMEM_REGION_ram + 1*1024*1024;
	int size = CYGMEM_REGION_ram_SIZE / 2;
	int cycles;
	long long i;

	if ( (bsize % 32) || (size % bsize) || size < (1*1024*1024)) {
		diag_printf("size is illegal(size=%d)\n", size);
	}

	size = (size / bsize) * bsize;
	diag_printf("%s:size=%d, bsize=%d\n", read?"READ":"WRITE", size, bsize);
	start_timer(time);
	for (i=0; !get_time(&cycles); i += bsize) {
		if (read)
			diagnosis_mem_read_block(start + (i % size), bsize);
		else
			diagnosis_mem_write_block(start + (i % size), bsize);
	}
	stop_timer();
	diag_printf("Finished size=%ld ", i);
	diag_printf("time=%d", time);
	diag_printf(" %d(ms)\n", cycles);
	i = (i * 1000) / ((time * 1000) + cycles);
	return i / 1024;
}

static int performance_copy(int time, int bsize)
{
	unsigned long start = CYGMEM_REGION_ram + 1 * 1024 * 1024;
	unsigned long dest = start + 1 * 1024 * 1024;
	int size = CYGMEM_REGION_ram_SIZE / 4;
	int cycles;
	long long i;

	dest += size;

	if ( (bsize % 32) || (size % bsize) || size < (1 * 1024 * 1024)) {
		diag_printf("size is illegal(size=%d)\n", size);
	}

	start_timer(time);

	size = (size / bsize) * bsize;
	for (i = 0; !get_time(&cycles); i += bsize) {
		if (diagnosis_mem_copy_block(start + (i % size),
					    dest + (i % size), bsize)
			 			< 0) {
			diag_printf("verify data fail\n");
			break;
		}
	}
	stop_timer();
	i = (i * 1000) / ((time * 1000) + cycles);
	return i / 1024;
}

static void ram_pm_test(int argc, char * argv[])
{
	int opts_map[3];
	struct option_info opts[3];
	int time, mode, result, bsize;

	memset(opts_map, 0, sizeof(int)*2);
	init_opts(&opts[0], 't', true, OPTION_ARG_TYPE_NUM,
		 (void *)&time, (bool *)&opts_map[0], "test time");
	init_opts(&opts[1], 'b', true, OPTION_ARG_TYPE_NUM,
		 (void *)&bsize, (bool *)&opts_map[1], "block size");
	init_opts(&opts[2], 'm', true, OPTION_ARG_TYPE_NUM,
		 (void *)&mode, (bool *)&opts_map[2], "operate mode");

	if (!scan_opts(argc, argv, 2, opts, 3, 0, 0, 0)) {
		diagnosis_usage("invalid arguments");
		return;
	}

	if (!opts_map[0] || time < MIN_TEST_TIME || time > MAX_TEST_TIME)
		time = DEFAULT_TEST_TIME;

	if(!opts_map[1] || bsize < MIN_BLOCK_SIZE || bsize > MAX_BLOCK_SIZE)
		bsize = DEFAULT_BLOCK_SIZE;

	if (!opts_map[2] || mode >= RAM_PM_MAX)
		mode = RAM_PM_READ;

	diag_printf("Start memory performance test (%s)...\n",
		mode_str[mode]);
	switch(mode) {
	case RAM_PM_READ:
		result = performance_read_write(time, bsize, 1);
		break;
	case RAM_PM_WRITE:
		result = performance_read_write(time, bsize, 0);
		break;
	case RAM_PM_COPY:
		result = performance_copy(time, bsize);
		break;
	default:
		result = -1;
	}
	if (result < 0) {
		diag_printf("memory performance test fails\n");
	} else {
		diag_printf("memory performance test success:%d.%d(MB/s)\n",
			      result / 1024, (result % 1024));
	}
}
