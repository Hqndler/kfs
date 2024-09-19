#include "kernel.h"

extern void *stack_ptr(void);

static inline bool isprint(char c) {
	return (c >= 32 && c <= 126);
}

void print_stack(void) {
	uint8_t ogcolor = terminal_color;
	void *ptr = stack_ptr();
	// void *ptr = 0x800;

	for (size_t i = 0; i < 10; i++) {
		uint8_t buffer[16];
		uint8_t str[16] = "0x00000000";
		uint8_t colors[16];
		uint8_t hex[16][3];

		kmemset(hex, '0', 16 * 3);

		kxitoa((char *)str, (uint32_t)ptr, 11, false);

		terminal_setcolor(
			vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));

		kprint("%s", str);

		kmemset(str, '.', 16);
		str[16] = 0;
		kmemcpy(buffer, ptr, 16);

		for (size_t j = 0; j < 16; j++) {

			kxitoa((char *)hex[j], buffer[j], 3, false);

			if (isprint(buffer[j])) {
				str[j] = buffer[j];
				colors[j] =
					vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
			}

			else if (!buffer[j])
				colors[j] =
					vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);

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

void clear() {
	screen_cursor[kernel_screen] = 0;
	kmemset(&screen_buffer[kernel_screen][VGA_WIDTH * 6], 0,
			VGA_WIDTH * (VGA_HEIGHT - 6));
	screen_cursor[kernel_screen] = VGA_WIDTH * 6;
	terminal_initialize();
}

void exec() {
	uint8_t *ptr = &input_buffer[VGA_WIDTH - 2];
	while (ptr > input_buffer && (!*ptr || *ptr == ' ')) {
		*ptr-- = '\0';
	}
	if (!kstrcmp((char *)input_buffer, "reboot"))
		reboot();

	if (!kstrcmp((char *)input_buffer, "halt")) {
		is_hlt = true;
		halt();
		halt();
		kprint("HALT DONE\n");
		is_hlt = false;
	}
	if (!kstrcmp((char *)input_buffer, "clear")) {
		clear();
	}

	if (!kstrcmp((char *)input_buffer, "stack")) {
		print_stack();
	}
	terminal_putprompt();
	fb_move_cursor(screen_cursor[kernel_screen]);
}

void handle_input(char c) {
	if (!c)
		return;
	input_cursor %= sizeof(input_buffer);
	if (c == '\n') {
		kmemset(input_buffer, 0, sizeof(input_buffer));
		size_t index =
			(screen_cursor[kernel_screen] -
			 (screen_cursor[kernel_screen] % VGA_WIDTH) - VGA_WIDTH) +
			PROMPT_LEN;
		kmemmove(input_buffer, &screen_buffer[kernel_screen][index], VGA_WIDTH);
		input_cursor = 0;
		is_cmd = true;
	}
}

void prompt(char c) {
	if (is_hlt)
		return;
	if (!c || c == '\n') {
		terminal_putchar('\n');
		handle_input(c);
		fb_move_cursor(screen_cursor[kernel_screen]);
		return;
	}
	terminal_putchar(c);
	handle_input(c);
}
