#include "kernel.h"
#include <stdarg.h>

static uint8_t log_prefix(char c) {
	if (!(c >= '1' && c <= '9'))
		return 0;
	uint8_t color = terminal_color;
	static char const *msg[] = {
		"EMERGENCY: ", "ALERT: ",  "CRITICAL: ", "ERROR: ",
		"WARNING: ",   "NOTICE: ", "LOG Info: ", "LOG Debug: ",
	};
	static uint8_t const colors[8] = {
		VGA_COLOR_RED,	   VGA_COLOR_LIGHT_RED,	  VGA_COLOR_LIGHT_MAGENTA,
		VGA_COLOR_MAGENTA, VGA_COLOR_LIGHT_BROWN, VGA_COLOR_LIGHT_GREEN,
		VGA_COLOR_GREEN,   VGA_COLOR_WHITE};
	c -= 48;
	terminal_setcolor(vga_entry_color(colors[(uint8_t)c], terminal_color >> 4));
	terminal_writestring(msg[(uint8_t)c]);
	terminal_setcolor(color);
	return 1;
}

void kprint(char const *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char c;
	char *s;
	char num_buff[32];
	int32_t padding;

	fmt += log_prefix(*fmt);
	while (1) {
		while ((c = *fmt++)) {
			if (c == '%')
				break;
			terminal_putchar(c);
		}
		if (!c)
			break;

		padding = 0;
		if (*fmt == '0') {
			fmt++;
			while (*fmt >= '0' && *fmt <= '9') {
				padding = padding * 10 + (*fmt - '0');
				fmt++;
			}
		}

		c = *fmt++;
		switch (c) {
			case '\0':
				break;
			case '%':
				terminal_putchar('%');
				break;
			case 's':
				s = va_arg(ap, char *);
				if (!s)
					s = "NULL";
				terminal_writestring(s);
				break;
			case 'd':
			case 'i': {
				char *num_str =
					kitoa(num_buff, va_arg(ap, int), sizeof(num_buff));
				int32_t len = kstrlen(num_str);
				while (padding > len) {
					terminal_putchar('0');
					padding--;
				}
				terminal_writestring(num_str);
				break;
			}
			case 'p':
				terminal_writestring("0x");
				/* fallthrough */
			case 'X':
			case 'x': {
				char *num_str = kxitoa(num_buff, va_arg(ap, uint32_t),
									   sizeof(num_buff), c == 'X');
				int32_t len = kstrlen(num_str);
				while (padding > len) {
					terminal_putchar('0');
					padding--;
				}
				terminal_writestring(num_str);
				break;
			}
			case 'b':
				char *num_str =
					kbitoa(num_buff, va_arg(ap, uint32_t), sizeof(num_buff));
				int32_t len = kstrlen(num_str);
				while (padding > len) {
					terminal_putchar('0');
					padding--;
				}
				terminal_writestring(num_str);
				break;
			case 'c':
				terminal_putchar(va_arg(ap, int));
				break;
			default:
				terminal_putchar('%');
				terminal_putchar(c);
				break;
		}
	}
	va_end(ap);
}
