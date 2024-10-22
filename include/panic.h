#pragma once

#include "kernel.h"
#include <stdint.h>

void kpanic(char const *error);

typedef struct {
	uint32_t eax, ebx, ecx, edx;
	uint32_t esi, edi, ebp, esp;
	uint32_t eip, eflags;
	uint16_t cs, ds, es, fs, gs, ss;
} registers_t;

#define ASSERT_PANIC(condition, msg)                                           \
	do {                                                                       \
		if (!(condition)) {                                                    \
			kpanic(msg);                                                       \
		}                                                                      \
	} while (0)

extern void get_registers(registers_t *regs);
extern void clean_registers();
