#include "kernel.h"

#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);

void fb_move_cursor(uint16_t pos) {
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
	outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
	outb(FB_DATA_PORT, pos & 0x00FF);
}

bool is_caps = false;
bool is_altgr = false;
bool is_ctrl = false;
bool is_num = true;

typedef struct {
	bool is_accent;
	uint8_t code;
	uint8_t offset;
} accent_t;

accent_t accent = {0};

void toggle_altgr(void) {
	is_altgr = !is_altgr;
}

void toggle_caps(uint8_t code) {
	(void)code;
	is_caps = !is_caps;
}

void toggle_ctrl(uint8_t code) {
	(void)code;
	is_ctrl = !is_ctrl;
}

void toggle_num(uint8_t code) {
	(void)code;
	is_num = !is_num;
}

static uint32_t (*table)[256][3];

// code page 437
static uint32_t azerty[256][3] = {
	[0x02] = {'&',  '1',	0	 },
	[0x03] = {0x82, '2',	 '~' },
	[0x04] = {'"',  '3',	'#' },
	[0x05] = {'\'', '4',	 '{' },
	[0x06] = {'(',  '5',	'[' },
	[0x07] = {'-',  '6',	'|' },
	[0x08] = {0x8A, '7',	 '`' },
	[0x09] = {'_',  '8',	'\\'},
	[0x0A] = {0x87, '9',	 '^' },
	[0x0B] = {0x85, '0',	 '@' },
	[0x0C] = {')',  0xF8, ']' },
	[0x0D] = {'=',  '+',	'}' },
	[0x10] = {'a',  'A',	0	 },
	[0x11] = {'z',  'Z',	0	 },
	[0x12] = {'e',  'E',	0xFB},
	[0x13] = {'r',  'R',	0	 },
	[0x14] = {'t',  'T',	0	 },
	[0x15] = {'y',  'Y',	0	 },
	[0x16] = {'u',  'U',	0	 },
	[0x17] = {'i',  'I',	0	 },
	[0x18] = {'o',  'O',	0	 },
	[0x19] = {'p',  'P',	0	 },
	[0x1A] = {'^',  0xAB, 0	  },
	[0x1B] = {'$',  0xEC, 0	  },
	[0x1C] = {'\n', '\n', 0   },
	[0x1E] = {'q',  'Q',	0	 },
	[0x1F] = {'s',  'S',	0	 },
	[0x20] = {'d',  'D',	0	 },
	[0x21] = {'f',  'F',	0	 },
	[0x22] = {'g',  'G',	0	 },
	[0x23] = {'h',  'H',	0	 },
	[0x24] = {'j',  'J',	0	 },
	[0x25] = {'k',  'K',	0	 },
	[0x26] = {'l',  'L',	0	 },
	[0x27] = {'m',  'M',	0	 },
	[0x28] = {0x97, '%',	 0	  },
	[0x29] = {0xFD, 0,	   0	},
	[0x2B] = {'*',  0xE6, 0	  },
	[0x2C] = {'w',  'W',	0	 },
	[0x2D] = {'x',  'X',	0	 },
	[0x2E] = {'c',  'C',	0	 },
	[0x2F] = {'v',  'V',	0	 },
	[0x30] = {'b',  'B',	0	 },
	[0x31] = {'n',  'N',	0	 },
	[0x32] = {',',  '?',	0	 },
	[0x33] = {';',  '.',	0	 },
	[0x34] = {':',  '/',	0	 },
	[0x35] = {'!',  0,	  0   },
	[0x39] = {' ',  ' ',	0	 },
 // Pavé numérique
	[0x47] = {'7',  '7',	0	 },
	[0x48] = {'8',  '8',	0	 },
	[0x49] = {'9',  '9',	0	 },
	[0x4A] = {'-',  '-',	0	 },
	[0x4B] = {'4',  '4',	0	 },
	[0x4C] = {'5',  '5',	0	 },
	[0x4D] = {'6',  '6',	0	 },
	[0x4E] = {'+',  '+',	0	 },
	[0x4F] = {'1',  '1',	0	 },
	[0x50] = {'2',  '2',	0	 },
	[0x51] = {'3',  '3',	0	 },
	[0x52] = {'0',  '0',	0	 },
	[0x53] = {'.',  '.',	0	 },
	[0x37] = {'*',  '*',	0	 }
};

static uint32_t qwerty[256][3] = {
	[0x02] = {'1',  '!',	0},
	[0x03] = {'2',  '@',	0},
	[0x04] = {'3',  '#',	0},
	[0x05] = {'4',  '$',	0},
	[0x06] = {'5',  '%',	0},
	[0x07] = {'6',  '^',	0},
	[0x08] = {'7',  '&',	0},
	[0x09] = {'8',  '*',	0},
	[0x0A] = {'9',  '(',	0},
	[0x0B] = {'0',  ')',	0},
	[0x0C] = {'-',  '_',	0},
	[0x0D] = {'=',  '+',	0},
	[0x1A] = {'[',  '{',	0},
	[0x1B] = {']',  '}',	0},
	[0x27] = {';',  ':',	0},
	[0x28] = {'\'', '\"', 0},
	[0x29] = {'`',  '~',	0},
	[0x2B] = {'\\', '|',	 0},
	[0x33] = {',',  '<',	0},
	[0x34] = {'.',  '>',	0},
	[0x35] = {'/',  '?',	0},
	[0x39] = {' ',  ' ',	0},
	[0x1E] = {'a',  'A',	0},
	[0x30] = {'b',  'B',	0},
	[0x2E] = {'c',  'C',	0},
	[0x20] = {'d',  'D',	0},
	[0x12] = {'e',  'E',	0},
	[0x21] = {'f',  'F',	0},
	[0x22] = {'g',  'G',	0},
	[0x23] = {'h',  'H',	0},
	[0x17] = {'i',  'I',	0},
	[0x24] = {'j',  'J',	0},
	[0x25] = {'k',  'K',	0},
	[0x26] = {'l',  'L',	0},
	[0x32] = {'m',  'M',	0},
	[0x31] = {'n',  'N',	0},
	[0x18] = {'o',  'O',	0},
	[0x19] = {'p',  'P',	0},
	[0x10] = {'q',  'Q',	0},
	[0x13] = {'r',  'R',	0},
	[0x1F] = {'s',  'S',	0},
	[0x14] = {'t',  'T',	0},
	[0x16] = {'u',  'U',	0},
	[0x2F] = {'v',  'V',	0},
	[0x11] = {'w',  'W',	0},
	[0x2D] = {'x',  'X',	0},
	[0x15] = {'y',  'Y',	0},
	[0x2C] = {'z',  'Z',	0},
	[0x1C] = {'\n', '\n', 0},
 // Keypad
	[0x47] = {'7',  '7',	0},
	[0x48] = {'8',  '8',	0},
	[0x49] = {'9',  '9',	0},
	[0x4A] = {'-',  '-',	0},
	[0x4B] = {'4',  '4',	0},
	[0x4C] = {'5',  '5',	0},
	[0x4D] = {'6',  '6',	0},
	[0x4E] = {'+',  '+',	0},
	[0x4F] = {'1',  '1',	0},
	[0x50] = {'2',  '2',	0},
	[0x51] = {'3',  '3',	0},
	[0x52] = {'0',  '0',	0},
	[0x53] = {'.',  '.',	0},
	[0x37] = {'*',  '*',	0},
};

void switch_layout(uint8_t code) {
	(void)code;
	kprint("Layout switched to %s\n$>", &azerty == table ? "qwerty" : "azerty");
	table = (&azerty == table) ? &qwerty : &azerty;
}

void init_keyboard(void) {
	table = &qwerty;
}

bool is_accent(uint8_t c) {
	static uint8_t table[] = {'`', '^', 0xAB, 0x7E};
	for (uint8_t i = 0; i < 4; ++i)
		if (c == table[i]){
			accent.is_accent = true;
			accent.code = c;
			accent.offset = i;
			return true;
		}
	return false;
}

uint8_t get_corresponding_keycode(uint8_t c) {
	static uint8_t assoc[8][4] = {
		{0x85, 0x83, 0x84, 0x86}, // a
		{0x8A, 0x88, 0x89, 0x65}, // e
		{0x8D, 0x8C, 0x8B, 0x69}, // i
		{0x95, 0x93, 0x94, 0x96}, // o
		{0x97, 0xFB, 0x81, 0x75}, // u
		{0x79, 0x79, 0x98, 0x79}, // y
		{0x6E, 0x6E, 0x6E, 0xA4}, // n
		{0x8E, 0xA5, 0x99, 0x9A}  // Ä, Ñ, Ö, Ü
	};
	bool fall = true;
	switch (c) {
		case 'a':
			c = 0;
			break;
		case 'e':
			c = 1;
			break;
		case 'i':
			c = 2;
			break;
		case 'o':
			c = 3;
			break;
		case 'u':
			c = 4;
			break;
		case 'y':
			c = 5;
			break;
		case 'n':
			c = 6;
			break;
		case 'A': {
			if (accent.code != 0xAB)
				fall = false;
			accent.offset = 0;
			c = 7;
			break;
		}
		case 'O':{
			if (accent.code != 0xAB)
				fall = false;
			else 			
				c = 7;
			accent.offset = 2;
			break;
		}
		case 'U': {
			if (accent.code != 0xAB)
				fall = false;
			else
				c = 7;
			accent.offset = 3;
			break;
		}
		case 'N': {
			if (accent.code != 0x7E) 
				fall = false;
			else
				c = 7;
			accent.offset = 1;
			break;
		}

		default:
			fall = false;
			break;
	}
	
	if (fall)
		c = assoc[c][accent.offset];
	else
		prompt(accent.code);
	kmemset(&accent, 0, sizeof(accent_t));
	return c;
}

void handle_code(uint8_t code) {
	if (code >= 0x47 && code <= 0x53 && code != 0x4A && code != 0x4E && !is_num)
		return;
	uint8_t index = (is_altgr) ? 2 : is_caps;
	uint8_t c = (*table)[code][index];
	if (c == 0 || c == 255 || (table == &azerty && is_accent(c)))
		return;
	if (table == &azerty && accent.is_accent)
		c = get_corresponding_keycode(c);
	if (is_ctrl && c >= '0' && c <= '9')
		switch_screen((int)c - '0' - 1);
	else
		prompt(c);
}

uint8_t get_scan_code() {
	return inb(0x60);
}

void handle_extended(uint8_t code) {
	if (is_hlt)
		return;
	code = get_scan_code();
	switch (code) {
		case 0x4B:
			if (input_buffer.cursor > 0) {
				--screen_cursor[kernel_screen];
				move_input_cursor(-1);
			}
			break;
		case 0x4D:
			if (input_buffer.cursor < input_buffer.size) {
				++screen_cursor[kernel_screen];
				move_input_cursor(1);
			}
			break;
		case 0x53:
			delete_char(0x53);
			delete_char_buffer();
			break;
		case 0x47:
			if (input_buffer.size)
				screen_cursor[kernel_screen] -= input_buffer.size + (input_buffer.cursor % input_buffer.size);
			input_buffer.cursor = 0;
			break;
		case 0x48: {
			if (input_buffer.cursor)
				break;
			size_t i = 0;
			size_t start = screen_cursor[kernel_screen] -
						   (screen_cursor[kernel_screen] % VGA_WIDTH) +
						   (PROMPT_LEN);
			screen_cursor[kernel_screen] = start;
			kvgaset(&screen_buffer[kernel_screen][start],
					vga_entry(' ', terminal_color), VGA_WIDTH - PROMPT_LEN);
			kvgaset(&terminal_buffer[start], vga_entry(' ', terminal_color),
					VGA_WIDTH - PROMPT_LEN);

			if (kstrlen(last_cmd) == 1 && last_cmd[0] == ' ')
				break;

			while (last_cmd[i])
				prompt(last_cmd[i++]);
			break;
		}
		case 0x4f: {
			if (input_buffer.size)
				screen_cursor[kernel_screen] += input_buffer.size - (input_buffer.cursor % input_buffer.size);
			input_buffer.cursor = input_buffer.size;
			break;
		}
		case 0x1C:
			prompt('\n');
			break;
		case 0x35:
			prompt('/');
			break;
		case 0x38:
			toggle_altgr();
			break;
		case 0xB8:
			toggle_altgr();
			break;

		default:
			break;
	}
	fb_move_cursor(screen_cursor[kernel_screen]);
}
