#include "syscalls.h"

void dispatch_syscall() {

	registers_t regs;
	GET_REGISTERS(regs);

	kprint("Syscall: %d\n", regs.eax);

	if (regs.eax >= NUM_SYS)
		return kprint("Bad syscall");

	int ret;

	switch (regs.eax) {
		case SYS_DUMMY:
			ret = dummy_syscall();
			break;

		case SYS_WRITE:
			ret = write_syscall((void *)regs.ebx, regs.ecx);
			break;

		default:
			break;
	}
	asm volatile("mov %0, %%eax" : : "r"(ret) : "eax");
}

void test_syscall() {
	int ret;
	kprint("--------Dummy----------\n");
	asm volatile("mov $0, %eax");
	asm volatile("int $69");
	GET_EAX(ret);
	kprint("Dummy return %d\n", ret);

	kprint("----------Write----------\n");
	char const *message = "Hello, from write syscall :)\n";
	asm __volatile__("mov $1, %%eax;"
					 "mov %0, %%ebx;"
					 "mov %1, %%ecx;"
					 "int $69"
					 :
					 : "r"(message), "r"(kstrlen(message))
					 : "eax", "ebx", "ecx");
	GET_EAX(ret);
	kprint("Write return %d\n", ret);
	kprint("-------------------------\n");
}

int dummy_syscall() {
	kprint("I'm a dummy syscall !\n");
	return (42);
}

int write_syscall(char const *str, size_t len) {
	for (size_t i = 0; i < len; i++)
		terminal_putchar(*(str + i));
	return (len);
}
