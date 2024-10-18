#include "kernel.h"

void kinit(struct multiboot_info *mbi) {
	for (size_t i = 0; i < 255; i++)
		func[i] = &handle_code;
	func[0x3a] = &toggle_caps;
	func[0x36] = &toggle_caps;
	func[0xB6] = &toggle_caps;
	func[0x2A] = &toggle_caps;
	func[0xAA] = &toggle_caps;
	func[0x1D] = &toggle_ctrl;
	func[0x9D] = &toggle_ctrl;
	func[0x45] = &toggle_num;
	func[0xE0] = &handle_extended;
	func[0x0E] = &delete_char;
	func[0x01] = &reboot;

	kmemset(screen_cursor, 0, 10 * sizeof(size_t));
	init_gdt();
	init_idt();
	vga_init();
	init_buffers();
	terminal_initialize();
	init_memory(mbi);
}

void kernel_main(struct multiboot_info *mbi, uint32_t magic) {
	kinit(mbi);

	print_multiboot(mbi);

	while (1) {
		halt();
		if (is_cmd) {
			is_cmd = false;
			exec();
		}
		// kbrk(1024);
	}
}
