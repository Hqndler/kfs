#include "gdt.h"
#include "heap.h"
#include "interrupt.h"
#include "paging.h"
#include "virtual_manager.h"
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

uint32_t page_directory[PAGE_ENTRIES] __attribute__((aligned(4096)));
uint32_t page_table[PAGE_ENTRIES] __attribute__((aligned(4096)));
uintptr_t next_free_page = 0;
size_t allocated_pages = 0;
uintptr_t early_boot_next_free = 0;

extern uint32_t _KERNEL_START;
extern uint32_t _KERNEL_END;
extern uint32_t _EARLY_KMALLOC_START;
extern uint32_t _EARLY_KMALLOC_END;

uint32_t KERNEL_START = (uint32_t)&_KERNEL_START;
uint32_t KERNEL_END = (uint32_t)&_KERNEL_END;
uint32_t EARLY_KMALLOC_START = (uint32_t)&_EARLY_KMALLOC_START;
uint32_t EARLY_KMALLOC_END = (uint32_t)&_EARLY_KMALLOC_END;

bitmap_t physical_bitmap = {0, 0};

extern vm_manager_t *manager;
