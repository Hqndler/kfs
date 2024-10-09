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
	kprint(KERN_CRIT "Division by 0!\n");
	skip_instruction();
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
	set_idt_entry(0, (uint32_t)isr0, 0x08, 0x8E);
	set_idt_entry(8, (uint32_t)isr0, 0x08, 0x8E);
	set_idt_entry(14, (uint32_t)isr0, 0x08, 0x8E);
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
