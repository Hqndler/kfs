#include "kernel.h"

inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

inline uint16_t vga_entry(uint8_t uc, uint8_t color) {
	return (uint16_t)uc | (uint16_t)color << 8;
}

void vga_init() {
	// terminal_buffer = (uint16_t *)0xC00B8000;
	terminal_buffer = (uint16_t *)0xB8000;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
		terminal_buffer[i] = vga_entry(' ', terminal_color);
}

void terminal_initialize(void) {
	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
		terminal_buffer[i] = screen_buffer[kernel_screen][i];
	fb_move_cursor(screen_cursor[kernel_screen]);
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t index) {
	kvgashift(screen_buffer[kernel_screen], vga_entry(c, color), index,
			  (VGA_WIDTH * VGA_HEIGHT));
	kvgashift(terminal_buffer, vga_entry(c, color), index,
			  (VGA_WIDTH * VGA_HEIGHT));
}

void terminal_putprompt(void) {
	if (is_hlt)
		return;
	for (size_t i = 0; i < PROMPT_LEN; i++)
		terminal_putentryat(PROMPT_STR[i], terminal_color,
							screen_cursor[kernel_screen]++);
}

void terminal_putchar(char c) {
	if (c == '\n') {
		screen_cursor[kernel_screen] += VGA_WIDTH;
		screen_cursor[kernel_screen] -=
			screen_cursor[kernel_screen] % VGA_WIDTH;
	}
	else
		terminal_putentryat(c, terminal_color, screen_cursor[kernel_screen]++);

	if (screen_cursor[kernel_screen] >= (VGA_WIDTH * VGA_HEIGHT)) {

		kmemmove(&screen_buffer[kernel_screen][VGA_WIDTH * 7],
				 &screen_buffer[kernel_screen][VGA_WIDTH * 8],
				 VGA_WIDTH * (VGA_HEIGHT - 8) * sizeof(uint16_t));

		kmemmove(&terminal_buffer[VGA_WIDTH * 7],
				 &terminal_buffer[VGA_WIDTH * 8],
				 VGA_WIDTH * (VGA_HEIGHT - 8) * sizeof(uint16_t));

		kvgaset(&screen_buffer[kernel_screen][(VGA_HEIGHT - 1) * VGA_WIDTH],
				vga_entry(' ', terminal_color), VGA_WIDTH);
		kvgaset(&terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH],
				vga_entry(' ', terminal_color), VGA_WIDTH);
		screen_cursor[kernel_screen] = ((VGA_HEIGHT - 1) * (VGA_WIDTH));
	}
	fb_move_cursor(screen_cursor[kernel_screen]);
}

void terminal_write(char const *data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(char const *data) {
	terminal_write(data, kstrlen(data));
}

void terminal_puthexa(uint32_t n) {
	if (n / 16)
		terminal_puthexa(n / 16);
	terminal_putchar("0123456789ABCDEF"[n % 16]);
}

void terminal_putnbr(uint32_t n) {
	if (n / 10)
		terminal_putnbr(n / 10);
	terminal_putchar((n % 10) + '0');
}

static const uint8_t colors[][2] = {
	{VGA_COLOR_LIGHT_CYAN,	   VGA_COLOR_BLACK},
	{VGA_COLOR_LIGHT_BLUE,	   VGA_COLOR_BLACK},
	{VGA_COLOR_GREEN,		  VGA_COLOR_BLACK},
	{VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK},
	{VGA_COLOR_RED,			VGA_COLOR_BLACK},
	{VGA_COLOR_WHITE,		  VGA_COLOR_BLACK},
	{VGA_COLOR_LIGHT_GREY,	   VGA_COLOR_BLACK},
	{VGA_COLOR_LIGHT_BROWN,	VGA_COLOR_BLACK},
	{VGA_COLOR_WHITE,		  VGA_COLOR_BLACK},
	{VGA_COLOR_LIGHT_BROWN,	VGA_COLOR_RED	 },
};

void switch_screen(int n) {
	if (is_hlt)
		return;
	if (n < 0)
		n = 9;
	if (n == (int)kernel_screen)
		return;
	kernel_screen = n;
	terminal_setcolor(vga_entry_color(colors[n][0], colors[n][1]));
	terminal_initialize();
}

void disable_cursor() {
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void delete_char(uint8_t code) {
	if (is_hlt)
		return;
	if (screen_cursor[kernel_screen] == (VGA_WIDTH * 6) + 2 + (code != 0x0E))
		return;

	if (screen_cursor[kernel_screen] % VGA_WIDTH == (2 - (code != 0x0E)))
		return;

	if (code == 0x0E) {
		--screen_cursor[kernel_screen];
	}
	size_t len = (VGA_WIDTH * VGA_HEIGHT) - screen_cursor[kernel_screen] - 1;
	kmemmove(&screen_buffer[kernel_screen][screen_cursor[kernel_screen]],
			 &screen_buffer[kernel_screen][screen_cursor[kernel_screen]] + 1,
			 len * sizeof(uint16_t));
	kmemmove(&terminal_buffer[screen_cursor[kernel_screen]],
			 &terminal_buffer[screen_cursor[kernel_screen]] + 1,
			 len * sizeof(uint16_t));
	fb_move_cursor(screen_cursor[kernel_screen]);
}

void write_string_buffer(char const *str) {
	for (size_t i = 0; i < kstrlen(str); i++) {
		char c = str[i];
		if (c == '\n') {
			screen_cursor[kernel_screen] += VGA_WIDTH;
			screen_cursor[kernel_screen] -=
				screen_cursor[kernel_screen] % VGA_WIDTH;
		}
		else
			screen_buffer[kernel_screen][screen_cursor[kernel_screen]++] =
				vga_entry(c, terminal_color);
	}
}

void init_buffers(void) {
	char tmp[3] = {'\0', '\n', '\0'};
	for (uint8_t i = 0; i < 10; i++) {
		kernel_screen = i;
		terminal_setcolor(vga_entry_color(colors[kernel_screen][0],
										  colors[kernel_screen][1]));
		kvgaset(&screen_buffer[kernel_screen][0],
				vga_entry(' ', terminal_color), (VGA_HEIGHT * VGA_WIDTH));

		// clang-format off
        write_string_buffer("                  _   __                     _                      ___  _____ \n");
        write_string_buffer("                 | | / /                    | |                    /   |/ __  \\\n");
        write_string_buffer("  ___  _   _ _ __| |/ /  ___ _ __ _ __   ___| |                   / /| |`' / /'\n");
        write_string_buffer(" / _ \\| | | | '__|    \\ / _ \\ '__| '_ \\ / _ \\ |                  / /_| |  / /  \n");
        write_string_buffer("| (_) | |_| | |  | |\\  \\  __/ |  | | | |  __/ |                  \\___  |./ /___\n");
        write_string_buffer(" \\___/ \\__,_|_|  \\_| \\_/\\___|_|  |_| |_|\\___|_|                      |_/\\_____/\n");
		write_string_buffer("tty: ");
		tmp[0] = i + '0';
		write_string_buffer(tmp);
		write_string_buffer(PROMPT_STR);
	}
	kernel_screen = 0;
	terminal_setcolor(
		vga_entry_color(colors[kernel_screen][0], colors[kernel_screen][1]));
}



                                                                    
                                                                    
