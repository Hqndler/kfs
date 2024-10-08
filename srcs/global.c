#include "gdt.h"
#include "interrupt.h"
#include "paging.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t kernel_screen = 0;
size_t screen_cursor[10];
uint16_t screen_buffer[10][2000];
uint8_t terminal_color;
uint16_t *terminal_buffer;

t_gdt_entry gdt[GTD_ENTRIES];
t_gdt_ptr gdt_ptr;

t_idt_entry idt[IDT_ENTRIES];
t_idt_ptr idt_ptr;

uint8_t input_buffer[VGA_WIDTH + 1];
uint8_t input_cursor;

bool is_cmd = false;
bool is_hlt = false;

char last_cmd[VGA_WIDTH];

void (*func[255])(uint8_t code);

uint32_t page_table[NUM_PAGES] __attribute__((aligned(0x1000)));
