#include "kernel.h"

void r(uint8_t code) {
	(void)code;
	reboot();
}

uint32_t bmap_find(bitmap_t *bitmap) {
	for (uint32_t i = 0; i < bitmap->size; ++i) {
		if (bitmap->data[i] == 0xFFFFFFFF)
			continue;
		for (uint8_t b = 31; b >= 0; --b) {
			if (!(bitmap->data[i] & (1 << b)))
				return (((i * 32) + (32 - b)) - 1);
		}
	}
	return 0;
}

void print_multiboot(struct multiboot_info *mbi) {
	if (mbi == NULL) {
		kprint("No multiboot information!\n");
		return;
	}

	if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
		unsigned int total_mem = mbi->mem_lower + mbi->mem_upper;
		kprint("Total Memory %d KB, mem_lower = %d KB, mem_upper = %d KB\n",
			   total_mem, mbi->mem_lower, mbi->mem_upper);
	}

	if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
		unsigned int *mem_info_ptr = (unsigned int *)mbi->mmap_addr;

		while (mem_info_ptr < (unsigned int *)mbi->mmap_addr + mbi->mmap_length)
		{
			multiboot_memory_map_t *cur =
				(multiboot_memory_map_t *)mem_info_ptr;

			if (cur->len > 0)
				kprint("  [%p-%p] -> %s\n", (uint32_t)cur->addr,
					   (uint32_t)(cur->addr + cur->len),
					   cur->type == MULTIBOOT_MEMORY_AVAILABLE ? "Available" :
																 "Reserved");

			mem_info_ptr += cur->size + sizeof(cur->size);
		}
		kprint("  [%p-%p] -> Kernel\n", KERNEL_START, KERNEL_END);
	}
}

void kernel_main(struct multiboot_info *mbi, uint32_t magic) {
	for (size_t i = 0; i < 255; i++)
		func[i] = &handle_code;
	{
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
	}
	kmemset(screen_cursor, 0, 10 * sizeof(size_t));

	init_gdt();
	init_idt();
	vga_init();

	init_buffers();
	terminal_initialize();

	init_paging();

	print_multiboot(mbi);

	init_bitmaps(mbi);
	virtual_bitmap.data[1] = 0xFFFFFFF0;
	kprint("%d\n", bmap_find(&virtual_bitmap));

	void *ptr = kernel_allocate_pages(2);

	kmemset(ptr, 'A', PAGE_SIZE);

	uint32_t total_memory = (mbi->mem_lower + mbi->mem_upper) * 1024;

	uint32_t total_pages = total_memory / PAGE_SIZE;

	while (1) {
		halt();
		if (is_cmd) {
			is_cmd = false;
			exec();
		}
	}
}
