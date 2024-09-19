#include "kernel.h"

void r(uint8_t code) {
	(void)code;
	reboot();
}

void kernel_main(void) {
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
	func[0x01] = &r;
	for (size_t i = 0; i < 10; i++)
		kmemset(screen_buffer[i], ' ', VGA_HEIGHT * VGA_WIDTH);
	kmemset(screen_cursor, 0, 10 * sizeof(size_t));

	init_gdt();
	init_idt();
	vga_init();

	init_buffers();
	terminal_initialize();

	// int fault = 1 / 0;

	while (1) {
		halt();
		if (is_cmd) {
			is_cmd = false;
			exec();
		}
	}
}
