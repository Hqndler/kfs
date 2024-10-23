#include "kernel.h"

extern void *stack_ptr(void);

static inline bool isprint(char c) {
	return (c >= 32 && c <= 126);
}

char *get_line(char const *msg) {
	kprint(msg);
	while (!is_cmd)
		halt();
	is_cmd = !is_cmd;
	// kprint("%s\n", kstrdup());

	char *res = kstrdup(input_buffer.buffer);
	kmemset(input_buffer.buffer, 0, input_buffer.size);
	return res;

	// uint8_t *ptr = &input_buffer[VGA_WIDTH - 2];
	// while (ptr > input_buffer && (!*ptr || *ptr == ' ')) {
	// *ptr-- = '\0';
	// }
	//
	// kmemmove(last_cmd, input_buffer, (ptr + 2) - input_buffer);
	//
	// return (char *)input_buffer;
}

void dump(void *ptr) {
	uint8_t ogcolor = terminal_color;

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

void print_stack(void) {
	dump(&dump); // WHERE ??????
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

void trigger_interrupt(uint8_t interrupt_number) {
	switch (interrupt_number) {
		case 0x00:
			asm volatile("int $0x00");
			break;
		case 0x01:
			asm volatile("int $0x01");
			break;
		case 0x02:
			asm volatile("int $0x02");
			break;
		case 0x03:
			asm volatile("int $0x03");
			break;
		case 0x04:
			asm volatile("int $0x04");
			break;
		case 0x05:
			asm volatile("int $0x05");
			break;
		case 0x06:
			asm volatile("int $0x06");
			break;
		case 0x07:
			asm volatile("int $0x07");
			break;
		case 0x08:
			asm volatile("int $0x08");
			break;
		case 0x09:
			asm volatile("int $0x09");
			break;
		case 0x0A:
			asm volatile("int $0x0A");
			break;
		case 0x0B:
			asm volatile("int $0x0B");
			break;
		case 0x0C:
			asm volatile("int $0x0C");
			break;
		case 0x0D:
			asm volatile("int $0x0D");
			break;
		case 0x0E:
			asm volatile("int $0x0E");
			break;
		case 0x0F:
			asm volatile("int $0x0F");
			break;
		case 0x10:
			asm volatile("int $0x10");
			break;
		case 0x11:
			asm volatile("int $0x11");
			break;
		case 0x12:
			asm volatile("int $0x12");
			break;
		case 0x13:
			asm volatile("int $0x13");
			break;
		case 0x14:
			asm volatile("int $0x14");
			break;
		case 0x15:
			asm volatile("int $0x15");
			break;
		case 0x16:
			asm volatile("int $0x16");
			break;
		case 0x17:
			asm volatile("int $0x17");
			break;
		case 0x18:
			asm volatile("int $0x18");
			break;
		case 0x19:
			asm volatile("int $0x19");
			break;
		case 0x1A:
			asm volatile("int $0x1A");
			break;
		case 0x1B:
			asm volatile("int $0x1B");
			break;
		case 0x1C:
			asm volatile("int $0x1C");
			break;
		case 0x1D:
			asm volatile("int $0x1D");
			break;
		case 0x1E:
			asm volatile("int $0x1E");
			break;
		case 0x1F:
			asm volatile("int $0x1F");
			break;
		default:
			break;
	}
}

void exec() {
	char *line = input_buffer.buffer;

	if (!kstrcmp(line, "reboot") || !kstrcmp(line, "wtf"))
		reboot(0);

	if (!kstrncmp(line, "sleep", (size_t)(kstrchr(line, ' ') - line)))
		sleep(katoi(kstrchr(line, ' ')));

	if (!kstrcmp(line, "halt")) {
		is_hlt = true;
		halt();
		kprint("HALT DONE\n");
		is_hlt = false;
	}
	if (!kstrcmp(line, "clear")) {
		clear();
	}

	if (!kstrcmp(line, "panic")) {
		kpanic("MANUAL TRIGGER");
	}

	if (!kstrcmp(line, "showcase")) {
		test_malloc();
	}

	if (!kstrcmp(line, "stack")) {
		print_stack();
	}

	if (!kstrncmp(line, "dump", (size_t)(kstrchr(line, ' ') - line))) {
		line = kstrchr(line, ' ');
		while (*line == ' ')
			line++;
		if (!kstrncmp(line, "0x", 2))
			dump((void *)kaxtoi(line + 2));
		else
			dump((void *)kaxtoi(line));
	}

	if (!kstrncmp(line, "int", (size_t)(kstrchr(line, ' ') - line))) {
		int32_t marche = katoi(kstrchr(line, ' '));
		if (marche >= 0 && marche < 32) {
			trigger_interrupt(marche);
		}
	}

	terminal_putprompt();
	fb_move_cursor(screen_cursor[kernel_screen]);

	last_cmd = krealloc(last_cmd, input_buffer.size);
	kmemmove(last_cmd, input_buffer.buffer, input_buffer.size);

	kmemset(input_buffer.buffer, 0, input_buffer.capacity);
	input_buffer.cursor = 0;
	input_buffer.size = 0;
}

void handle_input(char c) {
	if (!c)
		return;
	if (c == '\n') {
		is_cmd = true;
		return;
	}
	insert_buff_char(c);
	// static size_t start_pos = 0;
	// if (!start_pos) {
	// 	start_pos = screen_cursor[kernel_screen];
	// 	input_cursor = 0;
	// }

	// if (!c)
	// 	return;
	// // kprint("%d\n", (screen_cursor[kernel_screen] % VGA_WIDTH));
	// if (input_cursor == len_line) {
	// 	len_line *= 2;
	// 	cur_line = krealloc(cur_line, len_line);
	// }
	// //
	// // kprint("%d", screen_cursor[kernel_screen] - start_pos);

	// if (c == '\n') {
	// 	// kprint("---%s---\n", cur_line);
	// 	is_cmd = true;
	// 	start_pos = screen_cursor[kernel_screen] + 1;
	// 	return;
	// }

	// // input_cursor = screen_cursor[kernel_screen] - start_pos;
	// cur_line[input_cursor++] = c;
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
