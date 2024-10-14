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

void abort(char *msg) {
	kprint(msg);
	kprint("System halted press any key to reboot\n");
	asm("sti");
	asm("hlt");
	asm("hlt");
	reboot();
}

void fault(char *msg) {
	asm volatile("pushal");
	kprint(msg);
	kprint("System halted\n");
	asm volatile("hlt");
	asm volatile("popal");
}

void fault_code(char *msg) {
	asm volatile("pushal");
	uint32_t code;
	asm volatile("mov 4(%%ebp), %0" : "=r" (code));
	kprint(msg);
	kprint(" [%b]\n", code);
	asm volatile("hlt");
	asm volatile("popal");
}

void trap(char *msg) {
	asm volatile("pushal");
	kprint(msg);
	// kprint("System halted\n");
	// asm volatile("htl");
	asm volatile("popal");
}

void isr0() {
	asm volatile("pushal");
	kprint(KERN_WARN "Division by 0!\n");

	OLD_REG();
	asm volatile("mov 4(%%ebp), %%ebx\n"
				 "add $12, %%ebx\n"
				 "mov %%ebx, 4(%%ebp)\n"
				 :
				 :
				 : "ebx");
	asm volatile("sti");
	asm volatile("hlt");
	asm volatile("pushal");
}

void isr1(void) {
	fault(KERN_WARN "Debug\n");
}

void isr2(void) {
	fault(KERN_WARN "Non-maskable Interrupt\n");
}

void isr3(void) {
	fault(KERN_WARN "Breakpoint\n");
}

void isr4(void) {
	fault(KERN_WARN "Overflow\n");
}

void isr5(void) {
	fault(KERN_WARN "Bound Range Exceed!\n");
}

void isr6(void) {
	fault(KERN_WARN "Invalid Opcode\n");
}

void isr7(void) {
	fault(KERN_WARN "Device Not Available\n");
}

void isr8() {
	abort(KERN_CRIT "DOUBLE FAULT!\n");
}

void isr9(void) {
	fault(KERN_WARN "Coprocessor Segment Overrun\n");
}

void isr10(void) {
	fault_code(KERN_WARN "Invalid TSS");
	// old eip can have a pointer to the next instruction (possible to skip)
}

void isr11(void) {
	fault_code(KERN_WARN "Segment not present");
}

void isr12(void) {
	fault_code(KERN_WARN "Stack-Segment Fault");
	// old eip can sometimes point to the next instruction
}

void isr13(void) {
	fault_code(KERN_WARN "General Protection Fault");
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

void isr15(void) {
	// RESERVED
}

void isr16(void) {
	fault(KERN_WARN "x87 Floating-Point Exception\n");
}

void isr17(void) {
	fault(KERN_WARN "Aligment check\n");
	// alignement check is not enabled
}

void isr18(void) {
	abort(KERN_CRIT "Machine check\n");
}

void isr19(void) {
	fault(KERN_WARN "SIMD Floating-Point Exception\n");
}

void isr20(void) {
	fault(KERN_WARN "Virtualization Exception\n");
}

void isr21(void) {
	fault(KERN_WARN "Control Protection Exception\n");
	// no documentation
}

void isr22(void) {
	// RESERVED
}
void isr23(void) {
	// RESERVED
}
void isr24(void) {
	// RESERVED
}
void isr25(void) {
	// RESERVED
}
void isr26(void) {
	// RESERVED
}
void isr27(void) {
	// RESERVED
}

void isr28(void) {
	fault(KERN_WARN "Hypervisor Injection Exception\n");
}

void isr29(void) {
	fault(KERN_WARN "VMM Communication Exception\n");
	// no documentation
}

void isr30(void) {
	fault(KERN_WARN "Security Exception\n");
	// no documentation
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
	set_idt_entry(1, (uint32_t)isr1, 0x08, 0x8E);
	set_idt_entry(2, (uint32_t)isr2, 0x08, 0x8E);
	set_idt_entry(3, (uint32_t)isr3, 0x08, 0x8E);
	set_idt_entry(4, (uint32_t)isr4, 0x08, 0x8E);
	set_idt_entry(5, (uint32_t)isr5, 0x08, 0x8E);
	set_idt_entry(6, (uint32_t)isr6, 0x08, 0x8E);
	set_idt_entry(7, (uint32_t)isr7, 0x08, 0x8E);
	set_idt_entry(8, (uint32_t)isr8, 0x08, 0x8E);
	set_idt_entry(9, (uint32_t)isr9, 0x08, 0x8E);
	set_idt_entry(10, (uint32_t)isr10, 0x08, 0x8E);
	set_idt_entry(11, (uint32_t)isr11, 0x08, 0x8E);
	set_idt_entry(12, (uint32_t)isr12, 0x08, 0x8E);
	set_idt_entry(13, (uint32_t)isr13, 0x08, 0x8E);
	set_idt_entry(14, (uint32_t)isr14, 0x08, 0x8E);
	set_idt_entry(15, (uint32_t)isr15, 0x08, 0x8E);
	set_idt_entry(16, (uint32_t)isr16, 0x08, 0x8E);
	set_idt_entry(17, (uint32_t)isr17, 0x08, 0x8E);
	set_idt_entry(18, (uint32_t)isr18, 0x08, 0x8E);
	set_idt_entry(19, (uint32_t)isr19, 0x08, 0x8E);
	set_idt_entry(20, (uint32_t)isr20, 0x08, 0x8E);
	set_idt_entry(21, (uint32_t)isr21, 0x08, 0x8E);
	set_idt_entry(22, (uint32_t)isr22, 0x08, 0x8E);
	set_idt_entry(23, (uint32_t)isr23, 0x08, 0x8E);
	set_idt_entry(24, (uint32_t)isr24, 0x08, 0x8E);
	set_idt_entry(25, (uint32_t)isr25, 0x08, 0x8E);
	set_idt_entry(26, (uint32_t)isr26, 0x08, 0x8E);
	set_idt_entry(27, (uint32_t)isr27, 0x08, 0x8E);
	set_idt_entry(28, (uint32_t)isr28, 0x08, 0x8E);
	set_idt_entry(28, (uint32_t)isr29, 0x08, 0x8E);
	set_idt_entry(30, (uint32_t)isr30, 0x08, 0x8E);
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
