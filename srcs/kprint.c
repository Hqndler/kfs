#include "kernel.h"
#include <stdarg.h>

void kprint(char const *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char c;
	char *s;
	char num_buff[32];
	while (1) {
		while ((c = *fmt++)) {
			if (c == '%')
				break;
			terminal_putchar(c);
		}
		if (!c)
			break;
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
			case 'i':
				terminal_writestring(
					kitoa(num_buff, va_arg(ap, int), sizeof(num_buff)));
				break;

			case 'p':
				terminal_writestring("0x");
				/* fallthrough */

			case 'X':
			case 'x':
				terminal_writestring(kxitoa(num_buff, va_arg(ap, unsigned long),
											sizeof(num_buff), c == 'X'));
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
