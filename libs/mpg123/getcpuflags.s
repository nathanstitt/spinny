// getcpuflags.s: get CPUFLAGS

// copyright ?-2006 by the mpg123 project - free software under the terms of the LGPL 2.1
// see COPYING and AUTHORS files in distribution or http://mpg123.de
// initially written by KIMURA Takuhiro

// extern int getcpuid(void) 
// -> 0x00000000 (CPUID instruction not supported)
// or CPUFLAGS

.text
	.align 4
.globl getcpuflags
	.type	 getcpuflags,@function
getcpuflags:
	pushl %ebp
	movl %esp,%ebp
	subl $4,%esp
	pushl %edx
	pushl %ecx
	pushl %ebx
	movl $0x80000000,%eax 
	
	pushfl
	pushfl
	popl %eax
	movl %eax,%ebx
	xorl $0x00200000,%eax
	pushl %eax
	popfl
	pushfl
	popl %eax
	popfl
	cmpl %ebx,%eax
	/ for detect 3DNow! support (bit 31)
	movl $0x80000001,%eax
	cpuid
	movl %edx,%eax
	jmp .L1
	.align 4
.L0:	
	movl $0,%eax
	.align 4
.L1:
	movl %eax,-4(%esp)
	popl %ebx
	popl %ecx
	popl %edx
	movl %ebp,%esp
	popl %ebp
	ret
