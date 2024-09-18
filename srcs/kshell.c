#include "kernel.h"

extern void *stack_ptr(void);

static inline bool isprint(char c)
{
	return (c >= 32 && c <= 126);
}

void print_stack(void) {
	// void * ptr = stack_ptr();
	void *ptr = 0x00;

	for (size_t i = 0; i < 10; i++)
	{
		uint8_t buffer[16];
		uint8_t str[17];
		
		kmemset(str, '.', 16);
		str[16] = 0;
		kmemcpy(buffer, ptr, 16);
		
		kprint("%p  ");
		
		for (size_t j = 0; j < 16; j++) {
			if (j == 8)
				kprint(" ");
			
			char tmp[3];
			kmemset(tmp, '0', 3);
			tmp[2] = 0;
			kxitoa(tmp, buffer[j], 3, false);
			kprint("%s ", tmp);
			
			if (isprint(buffer[j]))
				str[j] = buffer[j];
		}
		
		kprint("  |%s|\n", str);
		ptr += 4 * sizeof(void *);
	}
}