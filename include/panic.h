#pragma once

#include "kernel.h"

void kpanic(char const *error);

#define ASSERT_PANIC(condition, msg)                                           \
	do {                                                                       \
		if (!(condition)) {                                                    \
			kpanic(msg);                                                       \
		}                                                                      \
	} while (0)

extern void clean_registers();
