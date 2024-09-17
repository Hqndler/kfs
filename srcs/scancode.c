#include "kernel.h"

#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

void fb_move_cursor(unsigned short pos) {
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
	outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
	outb(FB_DATA_PORT, pos & 0x00FF);
}

bool is_caps = false;
bool is_ctrl = false;
bool is_num = true;

void toggle_caps(unsigned char code) {
	(void)code;
	is_caps = !is_caps;
}

void toggle_ctrl(unsigned char code) {
	(void)code;
	is_ctrl = !is_ctrl;
}

void toggle_num(unsigned char code) {
	(void)code;
	is_num = !is_num;
}

static unsigned char const table[256][2] = {
	[0x02] = {'1',  '!' },
	[0x03] = {'2',  '@' },
	[0x04] = {'3',  '#' },
	[0x05] = {'4',  '$' },
	[0x06] = {'5',  '%' },
	[0x07] = {'6',  '^' },
	[0x08] = {'7',  '&' },
	[0x09] = {'8',  '*' },
	[0x0A] = {'9',  '(' },
	[0x0B] = {'0',  ')' },
	[0x0C] = {'-',  '_' },
	[0x0D] = {'=',  '+' },
	[0x1A] = {'[',  '{' },
	[0x1B] = {']',  '}' },
	[0x27] = {';',  ':' },
	[0x28] = {'\'', '\"'},
	[0x29] = {'`',  '~' },
	[0x2B] = {'\\', '|' },
	[0x33] = {',',  '<' },
	[0x34] = {'.',  '>' },
	[0x35] = {'/',  '?' },
	[0x39] = {' ',  ' ' },
	[0x1E] = {'a',  'A' },
	[0x30] = {'b',  'B' },
	[0x2E] = {'c',  'C' },
	[0x20] = {'d',  'D' },
	[0x12] = {'e',  'E' },
	[0x21] = {'f',  'F' },
	[0x22] = {'g',  'G' },
	[0x23] = {'h',  'H' },
	[0x17] = {'i',  'I' },
	[0x24] = {'j',  'J' },
	[0x25] = {'k',  'K' },
	[0x26] = {'l',  'L' },
	[0x32] = {'m',  'M' },
	[0x31] = {'n',  'N' },
	[0x18] = {'o',  'O' },
	[0x19] = {'p',  'P' },
	[0x10] = {'q',  'Q' },
	[0x13] = {'r',  'R' },
	[0x1F] = {'s',  'S' },
	[0x14] = {'t',  'T' },
	[0x16] = {'u',  'U' },
	[0x2F] = {'v',  'V' },
	[0x11] = {'w',  'W' },
	[0x2D] = {'x',  'X' },
	[0x15] = {'y',  'Y' },
	[0x2C] = {'z',  'Z' },
	[0x1C] = {'\n', '\n'},
 // Keypad
	[0x47] = {'7',  '7' },
	[0x48] = {'8',  '8' },
	[0x49] = {'9',  '9' },
	[0x4A] = {'-',  '-' },
	[0x4B] = {'4',  '4' },
	[0x4C] = {'5',  '5' },
	[0x4D] = {'6',  '6' },
	[0x4E] = {'+',  '+' },
	[0x4F] = {'1',  '1' },
	[0x50] = {'2',  '2' },
	[0x51] = {'3',  '3' },
	[0x52] = {'0',  '0' },
	[0x53] = {'.',  '.' },
	[0x37] = {'*',  '*' },
};

void handle_code(unsigned char code) {
	if (code >= 0x47 && code <= 0x53 && code != 0x4A && code != 0x4E && !is_num)
		return;
	char c = table[code][is_caps];
	if (!((c >= ' ' && c <= '~') || c == '\n'))
		return;
	if (is_ctrl && c >= '0' && c <= '9') {
		int t = c;
		switch_screen(t - '0' - 1);
	}
	else
		terminal_putchar(c);
}

unsigned char get_scan_code() {
	while (!(inb(0x64) & 0x1))
		;
	return inb(0x60);
}

void handle_extended(unsigned char code) {
	code = get_scan_code();
	switch (code) {
		case 0x4B:
			if (screen_cursor[kernel_screen] != VGA_WIDTH * 6)
				--screen_cursor[kernel_screen];
			break;
		case 0x4D:
			if (screen_cursor[kernel_screen] != (VGA_WIDTH * VGA_HEIGHT) - 1)
				++screen_cursor[kernel_screen];
			break;
		case 0x53:
			delete_char(0x53);
			break;
		case 0x47:
			if (is_ctrl)
				screen_cursor[kernel_screen] = VGA_WIDTH * 6;
			else
				screen_cursor[kernel_screen] -=
					screen_cursor[kernel_screen] % VGA_WIDTH;
			break;
		case 0x4f:
			if (is_ctrl)
				screen_cursor[kernel_screen] = (VGA_HEIGHT * VGA_WIDTH) - 1;
			else
				screen_cursor[kernel_screen] +=
					(VGA_WIDTH - (screen_cursor[kernel_screen] % VGA_WIDTH)) -
					1;
			break;

		case 0x1C:
			terminal_putchar('\n');
			break;
		case 0x35:
			terminal_putchar('/');
			break;

		default:
			break;
	}
	fb_move_cursor(screen_cursor[kernel_screen]);
}
