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

void handle_keyboard_interrupt() {
	outb(0x20, 0x20);
	if (!(inb(0x64) & 0x1))
		return;
	uint8_t code = inb(0x60);
	func[code](code);
}

static char *exception_messages[] = {
	"Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Fault",
    "Machine Check", 
};

void isr_handler(t_interrupt_register *reg) {
	if (reg->int_no < 32) {
		uint32_t old_eip, old_cs, old_eflags;
		asm volatile(
			"movl 4(%%ebp), %0\n"  // EIP
			"movl 8(%%ebp), %1\n"  // CS
			"movl 12(%%ebp), %2\n" // EFLAGS
			: "=r" (old_eip), "=r" (old_cs), "=r" (old_eflags)
			:
			:
    	);
		kprint(KERN_WARN "%s\n", exception_messages[reg->int_no]);
		kprint("Old EIP: 0x%x, CS: 0x%x, EFLAGS: 0x%x\n", old_eip, old_cs, old_eflags);
		asm volatile("sti");
		asm volatile("hlt");
	}
}

void init_idt() {
	idt_ptr.offset = (uint32_t)&idt;
	idt_ptr.size = (sizeof(t_idt_entry) * IDT_ENTRIES) - 1;
	set_idt_entry(0x0, (uint32_t)isr0, 0x08, 0x8E);  // Division par zéro
	set_idt_entry(0x1, (uint32_t)isr1, 0x08, 0x8E);  // Debug
	set_idt_entry(0x2, (uint32_t)isr2, 0x08, 0x8E);  // Non Maskable Interrupt
	set_idt_entry(0x3, (uint32_t)isr3, 0x08, 0x8E);  // Breakpoint
	set_idt_entry(0x4, (uint32_t)isr4, 0x08, 0x8E);  // Into Detected Overflow
	set_idt_entry(0x5, (uint32_t)isr5, 0x08, 0x8E);  // Out of Bounds
	set_idt_entry(0x6, (uint32_t)isr6, 0x08, 0x8E);  // Invalid Opcode
	set_idt_entry(0x7, (uint32_t)isr7, 0x08, 0x8E);  // No Coprocessor
	set_idt_entry(0x8, (uint32_t)isr8, 0x08, 0x8E);  // Double Fault
	set_idt_entry(0xA, (uint32_t)isr10, 0x08, 0x8E);
	set_idt_entry(0xB, (uint32_t)isr11, 0x08, 0x8E);
	set_idt_entry(0xC, (uint32_t)isr12, 0x08, 0x8E);
	set_idt_entry(0xD, (uint32_t)isr13, 0x08, 0x8E);
	set_idt_entry(0xE, (uint32_t)isr14, 0x08, 0x8E);
	set_idt_entry(0x10, (uint32_t)isr16, 0x08, 0x8E);
	set_idt_entry(0x11, (uint32_t)isr17, 0x08, 0x8E);
	set_idt_entry(0x12, (uint32_t)isr18, 0x08, 0x8E);
	set_idt_entry(0x13, (uint32_t)isr19, 0x08, 0x8E);
	set_idt_entry(0x14, (uint32_t)isr20, 0x08, 0x8E);
    set_idt_entry(0x15, (uint32_t)isr21, 0x08, 0x8E);
    set_idt_entry(0x16, (uint32_t)isr22, 0x08, 0x8E);
    set_idt_entry(0x17, (uint32_t)isr23, 0x08, 0x8E);
    set_idt_entry(0x18, (uint32_t)isr24, 0x08, 0x8E);
    set_idt_entry(0x19, (uint32_t)isr25, 0x08, 0x8E);
    set_idt_entry(0x1A, (uint32_t)isr26, 0x08, 0x8E);
    set_idt_entry(0x1B, (uint32_t)isr27, 0x08, 0x8E);
    set_idt_entry(0x1C, (uint32_t)isr28, 0x08, 0x8E);
    set_idt_entry(0x1D, (uint32_t)isr29, 0x08, 0x8E);
    set_idt_entry(0x1E, (uint32_t)isr30, 0x08, 0x8E);
    set_idt_entry(0x1F, (uint32_t)isr31, 0x08, 0x8E);

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
