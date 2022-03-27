	.text
/*
 * void diagnosis_mem_read_block(unsigned long start, int size)
 */
	.global diagnosis_mem_read_block
diagnosis_mem_read_block:
	stmdb sp!, {r2 - r9}
	add r1, r1, r0 
1:	cmp r0, r1
	ldmloia r0!, {r2-r9}
	blo 1b
	ldmia sp!, {r2 - r9}	
	mov pc, lr
/*
 * void diagnosis_mem_write_block(unsigned long start, int size)
 */
	.global diagnosis_mem_write_block
diagnosis_mem_write_block:	
	stmdb sp!, {r2 - r9}
	add r1, r1, r0
1:	cmp r0, r1
	stmloia r0!, {r2 - r9}
	blo 1b
	ldmia sp!, {r2 - r9}	
	mov pc, lr
/*
 * int diagnosis_mem_copy_block(unsigned long start, unsigned long dest, int size)
 */
	.global diagnosis_mem_copy_block
diagnosis_mem_copy_block:
	stmdb sp!, {r3 - r11}
	stmdb sp!, {r0, r1}
	add r11, r1, r2
1:	cmp r1, r11
	ldmloia r0!, {r3-r10}
	stmloia r1!, {r3-r10}
	blo 1b
	ldmia sp!, {r0, r1}
1:	cmp r1, r11
	movhs r0, #0
	bhs 2f
	ldrlo r3, [r0], #4	
	ldrlo r4, [r1], #4
	cmp r3, r4
	beq 1b
	mov r0, #-1 	
2:	ldmia sp!, {r3 - r11}
	mov pc, lr	
