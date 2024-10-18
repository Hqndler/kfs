#include "gdt.h"

void set_gdt_entry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access,
				   uint8_t flags) {
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].flags = (limit >> 16) & 0x0F;
	gdt[num].flags |= flags & 0xF0;
	gdt[num].access = access;
}

void init_gdt(void) {
	gdt_ptr.limit = (sizeof(t_gdt_entry) * GTD_ENTRIES) - 1;
	gdt_ptr.base = 0x800;

	set_gdt_entry(0, 0, 0, 0, 0);
	set_gdt_entry(1, 0, 0xFFFFF, (uint8_t)(GDT_CODE_PL0), 0xCF);
	set_gdt_entry(2, 0, 0xFFFFF, (uint8_t)(GDT_DATA_PL0), 0xCF);
	// set_gdt_entry(3, 0, 0xFFFFF, (uint8_t)(GDT_STACK_PL0), 0xCF);
	set_gdt_entry(4, 0, 0xBFFFF, (uint8_t)(GDT_CODE_PL3), 0xCf);
	set_gdt_entry(5, 0, 0xBFFFF, (uint8_t)(GDT_DATA_PL3), 0xCF);
	// set_gdt_entry(6, 0, 0xBFFFF, (uint8_t)(GDT_STACK_PL3), 0xCF);
	kmemmove((void *)gdt_ptr.base, gdt, gdt_ptr.limit);

	load_gdt(((uint32_t)&gdt_ptr));
}
