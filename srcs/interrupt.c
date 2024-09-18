#include "interrupt.h"
#include "kernel.h"

void set_idt_entry(uint32_t id, uint32_t offset, uint16_t selector,
				   uint8_t attributes) {
	idt[id].offset_low = offset & 0xFFFF;
	idt[id].offset_high = (offset >> 16) & 0xFFFF;
	idt[id].attributes = attributes;
	idt[id].zero = 0;
	idt[id].selector = selector;
}

void tes() {
	kprint("KEYBOARD\n");
}

void zer() {
	kprint("ZEROOOO\n");
}

static bool vectors[256];
extern void *isr_stub_table[];

void init_idt() {
	idt_ptr.offset = &idt[0];
	idt_ptr.size = (sizeof(t_idt_entry) * IDT_ENTRIES) - 1;
	for (uint8_t vector = 0; vector < 32; vector++) {
		set_idt_entry(vector, isr_stub_table[vector], 0x08, 0x8E);
		vectors[vector] = true;
	}
	set_idt_entry(0, (uint32_t)zer, 0x08, 0x8E);

	// kmemset(idt, 0, sizeof(t_idt_entry) * IDT_ENTRIES);
	load_idt((uint32_t)&idt_ptr);
	__asm__ volatile("sti");
}
