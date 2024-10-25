#include "syscalls.h"

void dispatch_syscall() {

	registers_t regs;
	get_registers(&regs);
	kprint("%d\n", regs.eax);

	// int ret;
	// kprint("Syscall: %d\n", regs->eax);
	// if (regs->eax >= NUM_SYS)
	// return kprint("Bad syscall");
	// switch (regs->eax) {
	// case SYS_DUMMY:
	// ret = dummy_syscall();
	// break;
	//
	// case SYS_WRITE:
	// ret = write_syscall((void *)regs->ebx, regs->ecx);
	// break;
	//
	// default:
	// break;
	// }
	// asm volatile("mov %0, %%eax" : : "r"(ret) : "eax");
	// kprint("FINAL SYS\n");
}

void test_syscall() {
	kprint("----------Dummy----------\n");
	asm volatile("movl $42, %%eax" : : : "eax");
	asm volatile("int $69");
	registers_t regs;
	get_registers(&regs);
	kprint("%x\n", regs.eax);

	// kprint("----------Write----------\n");
	// char const *message = "Hello, syscall!\n";

	// asm volatile("movl $1, %%eax" : : : "eax");
	// asm volatile("int $69");
}

int dummy_syscall() {
	kprint("I'm a dummy syscall !\n");
	return (42);
}

int write_syscall(char const *str, size_t len) {
	for (size_t i = 0; i < len; i++)
		terminal_putchar(*(str + i));
	return (42);
}
