#include "gdt.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t kernel_screen = 0;
size_t screen_cursor[10];
uint8_t screen_buffer[10][2000];
uint8_t terminal_color;
uint16_t *terminal_buffer;

t_gdt_entry gdt[GTD_ENTRIES];
t_gdt_ptr gdt_ptr;

void (*func[255])(unsigned char code);
