#include "panic.h"

void get_registers(registers_t *regs) {
	asm volatile("movl %%eax, %0\n"
				 "movl %%ebx, %1\n"
				 "movl %%ecx, %2\n"
				 "movl %%edx, %3\n"
				 "movl %%esi, %4\n"
				 "movl %%edi, %5\n"
				 "movl %%ebp, %6\n"
				 "movl %%esp, %7\n"
				 "pushfl\n"
				 "popl %8\n"
				 "movw %%cs, %9\n"
				 "movw %%ds, %10\n"
				 "movw %%es, %11\n"
				 "movw %%fs, %12\n"
				 "movw %%gs, %13\n"
				 "movw %%ss, %14\n"
				 : "=m"(regs->eax), "=m"(regs->ebx), "=m"(regs->ecx),
				   "=m"(regs->edx), "=m"(regs->esi), "=m"(regs->edi),
				   "=m"(regs->ebp), "=m"(regs->esp), "=m"(regs->eflags),
				   "=m"(regs->cs), "=m"(regs->ds), "=m"(regs->es),
				   "=m"(regs->fs), "=m"(regs->gs), "=m"(regs->ss)
				 :
				 : "memory");
}

void clean_registers() {
	asm volatile("xorl %%eax, %%eax\n"
				 "xorl %%ebx, %%ebx\n"
				 "xorl %%ecx, %%ecx\n"
				 "xorl %%edx, %%edx\n"
				 "xorl %%esi, %%esi\n"
				 "xorl %%edi, %%edi\n"
				 "mov %%ax, %%ds\n"
				 "mov %%ax, %%es\n"
				 "mov %%ax, %%fs\n"
				 "mov %%ax, %%gs\n"
				 :
				 :
				 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
}

void print_registers(registers_t *regs) {
	kprint("Registers Dump:\n");
	kprint("EAX: 0x%08x  EBX: 0x%08x  ECX: 0x%08x  EDX: 0x%08x\n", regs->eax,
		   regs->ebx, regs->ecx, regs->edx);
	kprint("ESI: 0x%08x  EDI: 0x%08x  EBP: 0x%08x  ESP: 0x%08x\n", regs->esi,
		   regs->edi, regs->ebp, regs->esp);
	kprint("EIP: 0x%08x  EFLAGS: 0x%08x\n", regs->eip, regs->eflags);
	kprint("CS: 0x%04x  DS: 0x%04x  ES: 0x%04x\n", regs->cs, regs->ds,
		   regs->es);
	kprint("FS: 0x%04x  GS: 0x%04x  SS: 0x%04x\n", regs->fs, regs->gs,
		   regs->ss);
}

static uint8_t stack_save[PAGE_SIZE];

static void ascii() {
	// clang-format off
    screen_cursor[kernel_screen] = 0;
    write_string_buffer("           _   __                     _______           _        _ \n");
    write_string_buffer("          | | / /                    | | ___ \\         (_)      | |\n");
    write_string_buffer("          | |/ /  ___ _ __ _ __   ___| | |_/ /_ _ _ __  _  ___  | |\n");
    write_string_buffer("          |    \\ / _ \\ '__| '_ \\ / _ \\ |  __/ _` | '_ \\| |/ __| | |\n");
    write_string_buffer("          | |\\  \\  __/ |  | | | |  __/ | | | (_| | | | | | (__  |_|\n");
    write_string_buffer("          \\_| \\_/\\___|_|  |_| |_|\\___|_\\_|  \\__,_|_| |_|_|\\___| (_)\n\n");
	// clang-format on
}

void kpanic(char const *error) {
	registers_t regs;
	get_registers(&regs);
	clean_registers();

	uint32_t stack_start = regs.esp;
	uint32_t copy_size = PAGE_SIZE;
	kmemcpy(&stack_save[0], (void *)stack_start, copy_size);

	is_hlt = true;
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_RED));
	kvgaset(&screen_buffer[kernel_screen][0], vga_entry(' ', terminal_color),
			(VGA_HEIGHT * VGA_WIDTH));

	ascii();
	write_string_buffer("Kernel has encountered a fatal error: ");
	write_string_buffer(error);
	write_string_buffer("\n");
	print_registers(&regs);

	kprint("\nStack Dump:\n");
	for (size_t i = 0; i < 112; i += 16) {

		kprint("%p: ", regs.esp + i);
		for (size_t o = 0; o < 16; o++) {
			kprint("%02X", stack_save[i + o]);
			if (o == 7)
				kprint(" ");
		}
		kprint("\n");
	}

	write_string_buffer("\nKernel will reboot in 10 sec...");

	disable_cursor();
	terminal_initialize();

	uint32_t last = 0;

	ticks = 0;
	while (1) {
		halt();
		if (((ticks % 100) > 50) != last) {
			last = (ticks % 100) > 50;
			last ? ascii() :
				   kvgaset(&screen_buffer[kernel_screen][0],
						   vga_entry(' ', terminal_color), (6 * VGA_WIDTH));
			terminal_initialize();
		}

		if (ticks >= 100 * 10)
			reboot(0);
	}
}
