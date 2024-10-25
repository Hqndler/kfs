#pragma once

#include "kernel.h"

enum SYS_CALL
{
	SYS_DUMMY = 0,
	SYS_WRITE = 1,
	NUM_SYS
};

typedef struct {
	uint32_t eax, ebx, ecx, edx, esi, edi, ebp;
} syscall_regs_t;

void test_syscall();

int dummy_syscall();
// void dispatch_syscall(syscall_regs_t *regs);
int write_syscall(char const *buf, size_t size);
