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

typedef struct __attribute__((packed)) {
	uint32_t cr2;
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;
} t_interrupt_register;

typedef struct __attribute((packed)) {
	uint16_t size;
	uint32_t offset;
} t_idt_ptr;

extern t_idt_entry idt[IDT_ENTRIES];
extern t_idt_ptr idt_ptr;

void load_idt(uint32_t idt_ptr);
void init_idt(void);

void isr_handler(t_interrupt_register *reg);
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);
extern void keyboard_handler();
extern void skip_instruction(void);

