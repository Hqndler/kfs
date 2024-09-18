#include "kernel.h"

void init_gdt(void);

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

void get_gdt_base_and_limit(uint32_t *base, uint16_t *limit) {
	gdt_ptr_t gdt_ptr;

	__asm__ volatile("sgdt %0" : "=m"(gdt_ptr));
	*base = gdt_ptr.base;
	*limit = gdt_ptr.limit;
}

void check_gdt_address() {
	uint32_t base;
	uint16_t limit;

	get_gdt_base_and_limit(&base, &limit);

	if (base == 0x00000800)
		kprint("GOOD GDT (I think)\n");
	else
		kprint("BAD\n");
}

void init_idt();

void kernel_main(void) {
	init_gdt();
	init_idt();

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

	for (size_t i = 0; i < 10; i++)
		kmemset(screen_buffer[i], ' ', VGA_HEIGHT * VGA_WIDTH);
	kmemset(screen_cursor, 0, 10 * sizeof(size_t));

	/* Initialize terminal interface */
	vga_init();
	init_buffers();
	terminal_initialize();
	// check_gdt_address();
	int test = 10;
	int test2 = 9;
	while (1) {
		kprint("%d\n", test2);
		test2 = test2 / --test;

		// unsigned char code = get_scan_code();
		// func[code](code);
	}
}
