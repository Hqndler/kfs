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

extern void skip_instruction(void);

void isr0() {
	kprint(KERN_WARN "Division by 0!\n");
	skip_instruction();
}

void isr1() {
	kprint(KERN_WARN "Debug\n");
	// skip_instruction();
}

void isr2() {
	kprint(KERN_WARN "Non-maskable Interrupt\n");
	// skip_instruction();
}

void isr3() {
	kprint(KERN_WARN "Breakpoint!\n");
	// skip_instruction();
}

void isr4() {
	kprint(KERN_WARN "Overflow!\n");
	// skip_instruction();
}

void isr5(){
	kprint(KERN_WARN "Bound Range Exceeded\n");
	skip_instruction();
}

void isr6(){
	kprint(KERN_WARN "Invalid Opcode!\n");
	skip_instruction();
}

void isr7(){
	kprint(KERN_WARN "Device Not Available\n");
	skip_instruction();
}

void isr8(){
	kprint(KERN_CRIT "Double Fault!\n");
	asm("hlt");
}

void isr10(){
	kprint(KERN_WARN "Invalid TSS!\n");
	skip_instruction();
}

void isr11(){
	kprint(KERN_WARN "Segment not present\n");
	skip_instruction();
}

void isr12(){
	kprint(KERN_WARN "Stack Segment Fault!\n");
	skip_instruction();
}

void isr13(){
	kprint(KERN_WARN "General Protection Fault!\n");
	skip_instruction();
}

void isr14(){
	kprint(KERN_WARN "Page Fault!\n");
	skip_instruction();
}

void isr16(){
	kprint(KERN_WARN "x87 Floating-Point Exception\n");
	skip_instruction();
}

void isr17(){
	kprint(KERN_WARN "Alignment Check\n");
	skip_instruction();
}

void isr18(){
	kprint(KERN_WARN "Machine Check\n");
	asm("hlt");
}

void isr19(){
	kprint(KERN_WARN "SIMD Floating-Point Exception\n");
	skip_instruction();
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
	// set_idt_entry(0x1, (uint32_t)isr2, 0x08, 0x8E);
	// set_idt_entry(0x2, (uint32_t)isr3, 0x08, 0x8E);
	// set_idt_entry(0x3, (uint32_t)isr4, 0x08, 0x8E);
	// set_idt_entry(0x4, (uint32_t)isr5, 0x08, 0x8E);
	// set_idt_entry(0x5, (uint32_t)isr5, 0x08, 0x8E);
	// set_idt_entry(0x6, (uint32_t)isr6, 0x08, 0x8E);
	// set_idt_entry(0x8, (uint32_t)isr8, 0x08, 0x8E);
	// set_idt_entry(0xA, (uint32_t)isr10, 0x08, 0x8E);
	// set_idt_entry(0xB, (uint32_t)isr11, 0x08, 0x8E);
	// set_idt_entry(0xC, (uint32_t)isr12, 0x08, 0x8E);
	// set_idt_entry(0xD, (uint32_t)isr13, 0x08, 0x8E);
	// set_idt_entry(0xE, (uint32_t)isr14, 0x08, 0x8E);
	// set_idt_entry(0x10, (uint32_t)isr16, 0x08, 0x8E);
	// set_idt_entry(0x11, (uint32_t)isr17, 0x08, 0x8E);
	// set_idt_entry(0x12, (uint32_t)isr18, 0x08, 0x8E);
	// set_idt_entry(0x13, (uint32_t)isr19, 0x08, 0x8E);
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
