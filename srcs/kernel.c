#include "kernel.h"

void r(uint8_t code) {
	(void)code;
	reboot();
}

#define BITMAP_NO_FREE_BIT UINT32_MAX

uint32_t bmap_find(bitmap_t *bitmap) {
	for (uint32_t i = 0; i < bitmap->size; ++i) {
		if (bitmap->data[i] != 0xFFFFFFFF) {
			for (uint8_t b = 0; b < 32; ++b) {
				if (!(bitmap->data[i] & (1U << b)))
					return i * 32 + b;
			}
		}
	}
	return BITMAP_NO_FREE_BIT;
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

#define RECURSIVE_PAGE_DIR_BASE 0xFFFFF000
#define RECURSIVE_PAGE_TABLE_BASE 0xFFC00000

void print_paging_state(void *addr) {
	uint32_t pd_index = (uint32_t)addr >> 22;
	uint32_t pt_index = ((uint32_t)addr >> 12) & 0x3FF;

	uint32_t *pde = (uint32_t *)(RECURSIVE_PAGE_DIR_BASE + (pd_index * 4));
	kprint("PDE for 0x%x: 0x%x\n", addr, *pde);

	if (*pde & PAGE_PRESENT) {
		uint32_t *pte = (uint32_t *)(RECURSIVE_PAGE_TABLE_BASE +
									 (pd_index * 0x1000) + (pt_index * 4));
		kprint("PTE for 0x%x: 0x%x\n", addr, *pte);
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
	init_bitmaps(mbi);
	init_vm_manager();
	init_slab_allocator();

	print_multiboot(mbi);

	char *ptr = 1; // tqt

	size_t size = 1;

	void *last = NULL;

	while (ptr) {
		ptr = kmalloc(size);
		kmemset(ptr, 42, size);
		kprint("%d bytes: %p -> %p\n", size, ptr, virtual_to_physical(ptr));
		last = ptr;
		size += 1;
		// kfree(ptr);
	}

	while (1) {
		halt();
		if (is_cmd) {
			is_cmd = false;
			exec();
		}
	}
}
