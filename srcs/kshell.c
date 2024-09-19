#include "kernel.h"

extern void *stack_ptr(void);

static inline bool isprint(char c)
{
	return (c >= 32 && c <= 126);
}

void print_stack(void) {
	uint8_t ogcolor = terminal_color;
	void * ptr = stack_ptr();
	// void *ptr = 0x800;

	for (size_t i = 0; i < 10; i++)
	{
		uint8_t buffer[16];
		uint8_t str[16] = "0x00000000";
		uint8_t colors[16];
		uint8_t hex[16][3];

		kmemset(hex, '0', 16 * 3);

		kxitoa((char *)str, (uint32_t)ptr, 11, false);

		terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));

		kprint("%s", str);

		kmemset(str, '.', 16);
		str[16] = 0;
		kmemcpy(buffer, ptr, 16);
		
		for (size_t j = 0; j < 16; j++) {
			
			kxitoa((char *)hex[j], buffer[j], 3, false);
			
			if (isprint(buffer[j])) {
				str[j] = buffer[j];
				colors[j] = vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
			}

			else if (!buffer[j])
				colors[j] = vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);

			else
				colors[j] = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
		}
		kprint("  ");
		for (size_t j = 0; j < 16; ++j) {
			if (j == 8)
				kprint(" ");
			terminal_setcolor(colors[j]);
			kprint("%s ", hex[j]);
		}
		terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
		kprint(" |");
		for (size_t j = 0; j < 16; ++j) {
			terminal_setcolor(colors[j]);
			kprint("%c", str[j]);
		}
		terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
		kprint("|");
		ptr += 16;
	}
	terminal_setcolor(ogcolor);
}