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
	char tmp[1024] = {0};
	kmemcpy(tmp, "ABORT DUE TO ", 13);
	kmemcpy(&tmp[13], msg, kstrlen(msg));
	asm volatile("sti");
	kpanic(tmp);
}

void fault(char *msg) {
	kprint(KERN_WARN "%s\nSystem halted\n", msg);
	asm volatile("hlt");
}

void print_selector_error_code(int error_code) {
	kprint("\nError Code: 0x%x 0b%b\n", error_code, error_code);

	int index = (error_code >> 3) & 0x1FFF;
	kprint("Segment Index: %d\n", index);

	int table_indicator = (error_code >> 1) & 0x3;
	switch (table_indicator) {
		case 0:
			kprint("GDT (Global Descriptor Table).\n");
			break;
		case 1:
			kprint("LDT (Local Descriptor Table).\n");
			break;
		case 2:
			kprint("IDT (Interrupt Descriptor Table).\n");
			break;
		default:
			kprint("Unknown table.\n");
			break;
	}

	int external = error_code & 0x1;
	if (external == 0) {
		kprint("Internal error : Processor error.\n");
	}
	else {
		kprint("Extern error : User instruction.\n");
	}
}

void fault_code(char *msg) {
	uint32_t code;
	asm volatile("mov 4(%%ebp), %0" : "=r"(code)); // not even sure it still works
	kprint(msg);
	print_selector_error_code(code);
	asm volatile("hlt");
}

static char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device not Available",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack-Segment fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Fault",
    "Machine Check", 
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exceptio",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
};

void page_fault(void) {
	uint32_t ptr;
	asm volatile("mov %%cr2, %0" : "=r"(ptr));
	kprint(KERN_CRIT "PAGE FAULT! at 0x%x\n", ptr);
	asm volatile("mov 8(%%ebp), %0" : "=r"(ptr));
	ptr & 0x1 ? kprint("Page-protection violation ") :
				kprint("Page not present ");
	ptr & 0x2 ? kprint("caused by write access") :
				kprint("caused by read access");
	ptr & 0x4 ? kprint(" in user mode ") : kprint(" in kernel mode ");
	kprint("[%b]\n", (uint8_t)ptr);
	asm volatile("hlt");
}

void isr_handler(uint32_t code) {

	if (code == 14)
		page_fault();

	if (code == 8 || code == 18)
		abort(exception_messages[code]);
	if ((code >= 10 && code <= 14) || code == 17 || code == 29 || code == 30)
		fault_code(exception_messages[code]);
	fault(exception_messages[code]);
}

void handle_keyboard_interrupt() {
	outb(0x20, 0x20);
	if (!(inb(0x64) & 0x1))
		return;
	uint8_t code = inb(0x60);
	func[code](code);
}

void init_timer() {
	ticks = 0;

	uint32_t divisor = 1193180 / 100;

	outb(0x43, 0x36);
	outb(0x40, (uint8_t)(divisor & 0xFF));
	outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void tick() {
	outb(0x20, 0x20);
	++ticks;
}

void init_idt() {
	// http://www.brokenthorn.com/Resources/OSDevPic.html
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
	set_idt_entry(29, (uint32_t)isr29, 0x08, 0x8E);
	set_idt_entry(30, (uint32_t)isr30, 0x08, 0x8E);
	set_idt_entry(0x20, (uint32_t)timer_handler, 0x08, 0x8E);
	set_idt_entry(0x21, (uint32_t)keyboard_handler, 0x08, 0x8E);
	init_timer();

	{
		outb(0x20, 0x11);
		outb(0xA0, 0x11);

		outb(0x21, 0x20);
		outb(0xA1, 0x28);

		outb(0x21, 0x4);
		outb(0xA1, 0x2);

		outb(0x21, 0x1);
		outb(0xA1, 0x1);

		outb(0x21, 0x0);
		outb(0xA1, 0x0);

		outb(0x21, 0xFC);
	}

	load_idt((uint32_t)&idt_ptr);
}
