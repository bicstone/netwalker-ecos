#include <redboot.h>
#include <stdlib.h>
#include <cyg/diagnosis/diagnosis.h>
#include <cyg/hal/plf_io.h>

#include CYGHWR_MEMORY_LAYOUT_H

static int loops1;
static unsigned int pattern1, pattern2;
static unsigned int start;
static int length;
static int burst = 0;

local_cmd_entry("ram_rw",
	 "ram read/write accessing",
	"-c iterators -b <base address> -l <length> "\
	"-p pattern -m case [-s]\n",
	ram_rw_test,
	DIAGNOSIS_cmds
);

local_cmd_entry("memcpybm",
	"ram memory copy benchmarking",
	"-c <loops> -s <start size KB> -e <end size KB>  -a <source align Byte> -b <dest align Byte>\n",
	memcpybm,
	DIAGNOSIS_cmds
);

static void raw_rw_case1(void)
{
	unsigned int * current_write;
	unsigned int * current_read;
	int round = 0;
	diag_printf("RAM diagnostical pattern from David.Young of freescale\n");
	diag_printf("burst is %s\n", burst?"enabled":"disabled");
	while( (round++) < loops1) {
		if (_rb_break(0))
			return;
		if(burst) {
			current_write =(unsigned int *)start;
			memset(current_write, (pattern1&0xFF000000)>>24, length);
		} else {
			for(current_write=(unsigned int *)start; current_write<(unsigned int *)(start + length); current_write += 2) {
				*current_write = ((unsigned int)current_write & 0x0000FFFF)|(0xFFFF0000 & pattern1);
			}
			for(current_write=(unsigned int *)start + 1; current_write<(unsigned int *)(start + length); current_write += 2) {
				*current_write = ((unsigned int)current_write & 0x0000FFFF)|(0xFFFF0000 & pattern2);
			}
		}
		for(current_read=(unsigned int *)start; current_read<(unsigned int *)(start + length); current_read ++) {
			if(burst) {
				if((*current_read) != pattern2) {
					diag_printf("\tround %d::[0x%08x]=0x%08x:0x%08x\n", round, current_read, pattern2, *current_read);
					goto fail;
				}
			} else {
				if((current_read - (unsigned int *)start) & 1) {
					if(((*current_read)&0xFFFF0000) != (pattern2&0xFFFF0000)) {
						diag_printf("\tround %d::[0x%08x]=0x%08x:0x%08x\n", round, current_read, (pattern2&0xFFFF0000)|((unsigned int)current_read)&0xFFFF, *current_read);
						goto fail;
					}
				} else {
					if(((*current_read)&0xFFFF0000) != (pattern1&0xFFFF0000)) {
						diag_printf("\tround %d::[0x%08x]=0x%08x:0x%08x\n", round, current_read, (pattern1&0xFFFF0000)|((unsigned int)current_read)&0xFFFF, *current_read);
						goto fail;
					}
				}
			}
		}
	}
	diag_printf("Diagnosis is successful!\n");
	return;
fail:
	diag_printf("Diagnosis is failure !\n");
}

static void ram_rw_test(int argc, char * argv[])
{
	int opts_map[6];
	struct option_info opts[6];
	int mode;

	memset(opts_map, 0, sizeof(int)*6);

	init_opts(&opts[0], 'c', true, OPTION_ARG_TYPE_NUM,
		 (void *)&loops1, (bool *)&opts_map[0], "the rounds of test");
	init_opts(&opts[1], 'b', true, OPTION_ARG_TYPE_NUM,
		 (void *)&start, (bool *)&opts_map[1], "accessing start address");
	init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM,
		 (void *)&length, (bool *)&opts_map[2], "accessing size(bytes)");
	init_opts(&opts[3], 'p', true, OPTION_ARG_TYPE_NUM,
		 (void *)&pattern1, (bool *)&opts_map[3], "High 16bit is valid");
	init_opts(&opts[4], 'm', true, OPTION_ARG_TYPE_NUM,
		 (void *)&mode, (bool *)&opts_map[4], "Test case number");
	init_opts(&opts[5], 's', false, OPTION_ARG_TYPE_FLG,
		 (void *)&burst, (bool *)0, "enable bust mode(based on memset)");

	if (!scan_opts(argc, argv, 2, opts, 6, 0, 0, 0)) {
		diagnosis_usage("invalid arguments");
		return;
	}

	if(!opts_map[0]) {
		loops1 = 32;
	}

	if(!opts_map[1]) {
		start = 0x80000;
	}

	if(!opts_map[2]) {
		length = 8192;
	}

	if(!opts_map[3]) {
		pattern1 = 0x55550000;
	}

	if(!opts_map[4]) {
		mode = DIAGNOSIS_MEM_RAM_RD;
	}

	if(burst) {
		pattern2 = (pattern1&0xFF000000);
		pattern2 |= pattern2>>8;
		pattern2 |= pattern2>>16;
	} else {
		pattern2 = (~pattern1)&0xFFFF0000;
	}

	if(!valid_address((unsigned char *)start)) {
		if (!verify_action("Specified address (%p) is not believed to be in RAM", (void*)start))
			return;
	}

	switch(mode) {
	case DIAGNOSIS_MEM_RAM_RD:
		raw_rw_case1();
		break;
	default:
		diag_printf("Invalid memory diagnosis case!\n");
	}
}

/* Defines */
#define SIZE_1K			1024
#define SIZE_4K			(4*SIZE_1K)
#define SIZE_1M			(1024*1024)
#define START_SIZE		(2*SIZE_1K)
#define END_SIZE		SIZE_1M
#define ALIGN			SIZE_4K
#define START_LOOPS		200000

#define OPT_SIZE        5
#define printf          diag_printf
#define CLOCKS_PER_SEC  32768
extern unsigned int hal_timer_count(void);
#define clock()         hal_timer_count()

//#define memcpy diagnosis_mem_copy_block
static void memcpybm(int argc, char * argv[])
{
	int opts_map[OPT_SIZE];
	struct option_info opts[OPT_SIZE];
	int mode;
	int size = START_SIZE / SIZE_1K;
	int end_size = END_SIZE / SIZE_1K;
	int salign = ALIGN;
	int dalign = ALIGN;
	int loops = START_LOOPS / 1000;
	int src, dst, asrc, adst;


	memset(opts_map, 0, sizeof(int)*OPT_SIZE);

	init_opts(&opts[0], 'c', true, OPTION_ARG_TYPE_NUM,
		 (void *)&loops, (bool *)&opts_map[0], "the rounds of test in thousands");
	init_opts(&opts[1], 's', true, OPTION_ARG_TYPE_NUM,
		 (void *)&size, (bool *)&opts_map[1], "start size in KB");
	init_opts(&opts[2], 'e', true, OPTION_ARG_TYPE_NUM,
		 (void *)&end_size, (bool *)&opts_map[2], "end size in KB");
	init_opts(&opts[3], 'a', true, OPTION_ARG_TYPE_NUM,
		 (void *)&salign, (bool *)&opts_map[3], "source align in byte");
	init_opts(&opts[4], 'b', true, OPTION_ARG_TYPE_NUM,
		 (void *)&dalign, (bool *)&opts_map[4], "destination align in byte");

	if (!scan_opts(argc, argv, 2, opts, OPT_SIZE, 0, 0, 0)) {
		diagnosis_usage("invalid arguments");
		return;
	}

	loops *= 1000;
	size *=  SIZE_1K;
	end_size *= SIZE_1K;
	/* Allocate buffers */
	if ((src = (int) malloc(end_size + salign + SIZE_4K)) == 0) {
		printf("%s: insufficient memory\n", argv[0]);
		return;
	}
	memset((void*)src, 0xaa, end_size + salign + SIZE_4K);
	if ((dst = (int) malloc(end_size + dalign + SIZE_4K)) == 0) {
		free((void*)src);
		printf("%s: insuficient memory\n", argv[0]);
		return;
	}
	memset((void*)dst, 0x55, end_size + dalign + SIZE_4K);

	/* Align buffers */
	if (src % SIZE_4K == 0)
		asrc = src + salign;
	else
		asrc = src + SIZE_4K - (src % SIZE_4K) + salign;
	if (dst % SIZE_4K == 0)
		adst = dst + dalign;
	else
		adst = dst + SIZE_4K - (dst % SIZE_4K) + dalign;

	/* Print Banner */
	printf("\nMEMCPY Benchmark\n\n");
	printf("Src Buffer 0x%08x\n", asrc);
	printf("Dst Buffer 0x%08x\n\n", adst);
	printf("%10s %10s\n", "Cached", "Bandwidth");
	printf("%10s %10s\n", "(KBytes)", "(MB/sec)");

	/* Loop over copy sizes */
	while (size <= end_size)
	{
		unsigned int start_time;
		unsigned int elapsed_time;
		int loop;
		unsigned long long sz;

		printf("%10d", size / SIZE_1K);

		/* Do data copies */
		start_time = clock();
		for (loop = 0; loop < loops; loop++)
			memcpy((void*)adst, (void*)asrc, size);
		elapsed_time = (clock() - start_time);

		sz = size *loops * 2;
		printf("   %d", sz*CLOCKS_PER_SEC/elapsed_time/SIZE_1M);
		printf("\t elapsed=%d", elapsed_time);
		printf("\tsize=%d, loops=%d, sz=%d", size, loops, sz);
		printf("\n");

/*
		printf(" %10.0f\n", ((float)size*loops*2)/elapsed_time/SIZE_1M);
		printf("   %d.%d.%d\n", elapsed_time / CLOCKS_PER_SEC,
               (elapsed_time % CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC,
               (((elapsed_time % CLOCKS_PER_SEC) * 1000) % CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC);
*/
		/* Adjust for next test */
		size *= 2;
		loops /= 2;
	}

	/* Free buffers */
	free((void*)src);
	free((void*)dst);
}

