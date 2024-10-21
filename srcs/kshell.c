#include "kernel.h"

extern void *stack_ptr(void);

static inline bool isprint(char c) {
	return (c >= 32 && c <= 126);
}

void print_stack(void) {
	uint8_t ogcolor = terminal_color;
	void *ptr = stack_ptr();
	// void *ptr = 0x800;

	for (size_t i = 0; i < 16; i++) {
		uint8_t buffer[16];
		uint8_t str[16] = "0x00000000";
		uint8_t colors[16];
		uint8_t hex[16][3];

		kmemset(hex, '0', 16 * 3);

		kxitoa((char *)str, (uint32_t)ptr, 11, false);

		terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, ogcolor >> 4));

		kprint("%s", str);

		kmemset(str, '.', 16);

		kmemcpy(buffer, ptr, 16);

		for (size_t j = 0; j < 16; j++) {

			kxitoa((char *)hex[j], buffer[j], 3, false);
			if (isprint(buffer[j])) {
				str[j] = buffer[j];
				colors[j] = vga_entry_color(VGA_COLOR_LIGHT_BLUE, ogcolor >> 4);
			}

			else if (!buffer[j])
				colors[j] = vga_entry_color(VGA_COLOR_DARK_GREY, ogcolor >> 4);

			else
				colors[j] = vga_entry_color(VGA_COLOR_WHITE, ogcolor >> 4);
		}
		kprint("  ");
		for (size_t j = 0; j < 16; ++j) {
			if (j == 8)
				kprint(" ");
			terminal_setcolor(colors[j]);
			kprint("%s ", hex[j]);
		}
		terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, ogcolor >> 4));
		kprint(" |");
		for (size_t j = 0; j < 16; ++j) {
			terminal_setcolor(colors[j]);
			kprint("%c", str[j]);
		}
		terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, ogcolor >> 4));
		kprint("|");
		ptr += 16;
	}
	terminal_setcolor(ogcolor);
}

void clear() {
	screen_cursor[kernel_screen] = 0;

	kvgaset(&screen_buffer[kernel_screen][VGA_WIDTH * 7],
			vga_entry(' ', terminal_color), VGA_WIDTH * (VGA_HEIGHT - 7));
	screen_cursor[kernel_screen] = VGA_WIDTH * 7;
	terminal_initialize();
}

void sleep(int time) {
	if (time < 0)
		return;
	ticks = 0;
	is_hlt = true;
	while (ticks < (uint32_t)time * 100)
		halt();
	is_hlt = false;
}

void exec() {
	uint8_t *ptr = &input_buffer[VGA_WIDTH - 2];
	while (ptr > input_buffer && (!*ptr || *ptr == ' ')) {
		*ptr-- = '\0';
	}

	kmemmove(last_cmd, input_buffer, (ptr + 2) - input_buffer);

	if (!kstrcmp((char *)input_buffer, "reboot") ||
		!kstrcmp((char *)input_buffer, "wtf"))
		reboot(0);

	if (!kstrncmp((char *)input_buffer, "sleep",
				  (uint8_t *)kstrchr((char *)input_buffer, ' ') - input_buffer))
		sleep(katoi(kstrchr((char *)input_buffer, ' ')));

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

	if (!kstrcmp((char *)input_buffer, "panic")) {
		kpanic("MANUAL TRIGGER");
	}

	if (!kstrcmp((char *)input_buffer, "showcase")) {
		test_malloc();
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
		for (size_t i = 0; i < VGA_WIDTH; i++)
			input_buffer[i] = (uint8_t)screen_buffer[kernel_screen][index + i];
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
