#include "kernel.h"

void r(uint8_t code) {
	(void)code;
	reboot();
}

uint32_t placement_address = (uint32_t)&kernel_end;

uint32_t _kmalloc(uint32_t size, int align) {
	if (align && (placement_address & 0xFFFFF000)) {
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
	uint32_t mem = placement_address;
	placement_address += size;
	return mem;
}

uint32_t kmalloc(uint32_t size) {
	return _kmalloc(size, 1);
}

uint32_t kmalloc_page() {
	return _kmalloc(0x1000, 1);
}

void kernel_main(multiboot_info_t mbt) {
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

	kmemset(screen_cursor, 0, 10 * sizeof(size_t));

	init_gdt();
	init_idt();
	init_paging();
	vga_init();

	init_buffers();
	terminal_initialize();

	kprint("%p\n", ((multiboot_info_t *)(&mbt))->mmap_addr);
	kprint("%x\n", *(&BootPageDirectory + 768));
	*(&BootPageDirectory + 42) = 0x00000083;
	kprint("%x\n", *(&BootPageDirectory + 42));
	kprint("%p\n", &kernel_start);
	kprint("%p\n", &kernel_end);
	kprint("%d\n", &kernel_end - &kernel_start);

	void *ptr = (void *)kmalloc_page();
	kprint("%p\n", ptr);

	ptr = (void *)kmalloc_page();
	kprint("%p\n", ptr);

	ptr = (void *)kmalloc_page();
	kprint("%p\n", ptr);

	*(uint32_t *)ptr = 42;

	kprint("%d\n", *(uint32_t *)ptr);

	char *str = kmalloc(11);
	kmemcpy(str, "0123456789", 11);
	kprint("%s\n", str);

	uint32_t *bitmap = (uint32_t *)kmalloc(0x1000);

	kprint("%p\n", bitmap);

	while (1) {
		halt();
		if (is_cmd) {
			is_cmd = false;
			exec();
		}
	}
}
