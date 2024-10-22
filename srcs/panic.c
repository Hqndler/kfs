#include "panic.h"

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
	clean_registers();
	get_registers(&regs);

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

	size_t cursor = screen_cursor[kernel_screen] - 31;

	disable_cursor();
	terminal_initialize();

	uint32_t last = 0;

	ticks = 0;

	char tmp[8];
	kmemset(tmp, 0, 8);

	while (1) {
		halt();
		if (((ticks % 100) > 50) != last) {
			last = (ticks % 100) > 50;
			last ? ascii() :
				   kvgaset(&screen_buffer[kernel_screen][0],
						   vga_entry(' ', terminal_color), (6 * VGA_WIDTH));
			terminal_initialize();
		}
		if (!(ticks % 100)) {
			screen_cursor[kernel_screen] = cursor;
			kvgaset(&screen_buffer[kernel_screen][cursor],
					vga_entry(' ', terminal_color), (VGA_WIDTH));
			write_string_buffer("Kernel will reboot in ");
			write_string_buffer(
				kitoa(&tmp[0], 10 - (ticks / 100), sizeof(tmp)));
			write_string_buffer(" sec...");
		}

		if (ticks >= 100 * 10)
			reboot(0);
	}
}
