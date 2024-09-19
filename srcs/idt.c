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

void isr0() {
	kprint(KERN_CRIT "Division by 0!\n");
	while (1)
		;
}

void handle_keyboard_interrupt() {
	outb(0x20, 0x20);
	if (!(inb(0x64) & 0x1))
		return;
	uint8_t code = inb(0x60);
	func[code](code);
}

void init_idt() {
	idt_ptr.offset = (uint32_t)&idt;
	idt_ptr.size = (sizeof(t_idt_entry) * IDT_ENTRIES) - 1;
	// for (uint8_t vector = 0; vector < 32; vector++) {
	// 	set_idt_entry(vector, isr_stub_table[vector], 0x08, 0x8E);
	// 	vectors[vector] = true;
	// }
	set_idt_entry(0, (uint32_t)isr0, 0x08, 0x8E);
	set_idt_entry(0x21, (uint32_t)keyboard_handler, 0x08, 0x8E);

	outb(0x20, 0x11);
	outb(0xA0, 0x11);

	outb(0x21, 0x20);
	outb(0xA1, 0x28);

	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	outb(0x21, 0x1);
	outb(0xA1, 0x1);

	outb(0x21, 0xff);
	outb(0xA1, 0xff);

	outb(0x21, 0xFD);
	load_idt((uint32_t)&idt_ptr);
}
