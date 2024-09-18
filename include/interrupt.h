#pragma once
#include <stdint.h>

#define IDT_ENTRIES 256

typedef struct __attribute__((packed)) {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t attributes;
	uint16_t offset_high;
} t_idt_entry;

typedef struct __attribute((packed)) {
	uint16_t size;
	uint32_t offset;
} t_idt_ptr;

extern t_idt_entry idt[IDT_ENTRIES];
extern t_idt_ptr idt_ptr;

void load_idt(uint32_t idt_ptr);
void init_idt(void);

extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);
extern void keyboard_handler();
