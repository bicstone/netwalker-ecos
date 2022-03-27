#ifndef __DIAGNOSIS_MEMORY_H_
#define __DIAGNOSIS_MEMORY_H_

extern void diagnosis_mem_read_block(unsigned long start, int size);
extern void diagnosis_mem_write_block(unsigned long start, int size);
extern int diagnosis_mem_copy_block(unsigned long start, unsigned long dest, int size);

#ifdef CYGSEM_RAM_RW_DIAGNOSIS

enum {
DIAGNOSIS_MEM_RAM_RD = 0,
};
#endif

#endif 		/* __DIAGNOSIS_MEMORY_H_ */
