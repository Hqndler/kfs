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
	kprint(KERN_WARN "Division by 0!\n");

	OLD_REG();
	asm volatile("mov 4(%%ebp), %%ebx\n"
				 "add $12, %%ebx\n"
				 "mov %%ebx, 4(%%ebp)\n"
				 :
				 :
				 : "ebx");
	asm("sti");
	asm("hlt");
}

void isr8() {
	kprint(KERN_CRIT "DOUBLE FAULT!\n");
	kprint("System halted press any key to reboot\n");
	asm("sti");
	asm("hlt");
	asm("hlt");
	reboot();
}

void isr14() {
	asm("pushal");
	uint32_t ptr;
	asm volatile("mov %%cr2, %0" : "=r"(ptr));
	kprint(KERN_CRIT "PAGE FAULT! at 0x%x\n", ptr);

	asm volatile("mov 4(%%ebp), %0" : "=r"(ptr));
	ptr & 0x1 ? kprint("Page-protection violation ") :
				kprint("Page not present ");
	ptr & 0x2 ? kprint("caused by write access") :
				kprint("caused by read access");
	ptr & 0x4 ? kprint(" in user mode ") : kprint(" in kernel mode ");
	kprint("[%b]\n", ptr);
	asm("sti");
	asm("hlt");
	asm("popal");
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
	set_idt_entry(0, (uint32_t)isr0, 0x08, 0x8E);
	set_idt_entry(8, (uint32_t)isr8, 0x08, 0x8E);
	set_idt_entry(14, (uint32_t)isr14, 0x08, 0x8E);
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
