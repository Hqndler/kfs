#include "virtual_manager.h"

vm_manager_t *manager = NULL;

void init_vm_manager() {
	manager = alloc_early_boot_memory(sizeof(vm_manager_t));
	if (!manager)
		return; // TODO HANDLE ERROR
	manager->areas = alloc_early_boot_memory(NUM_AREA * sizeof(vm_area_t));
	if (!manager->areas)
		return; // TODO HANDLE ERROR
	kmemset(manager->areas, 0, NUM_AREA * sizeof(vm_area_t));
	manager->areas[0].start = KERNEL_HEAP_START;
	manager->areas[0].end = 0xF0000000;
	manager->num_area = 1;
	manager->next = NULL;
}

void merge_adjacent() {
	for (size_t i = 0; i < manager->num_area - 1; i++) {
		if (manager->areas[i].end == manager->areas[i + 1].start) {
			manager->areas[i].end = manager->areas[i + 1].end;
			kmemmove(&manager->areas[i + 1], &manager->areas[i + 2],
					 (manager->num_area - i - 2) * sizeof(vm_area_t));
			--manager->num_area;
		}
	}
}

void add_free(void *ptr, size_t size) {

	manager->areas[manager->num_area].start = (uint32_t)ptr;
	manager->areas[manager->num_area].end = (uint32_t)(ptr) + size;
	if (manager->num_area + 1 >= NUM_AREA) {
		// TODO HANDLE OVERFLOW (LINKED LIST ?)
	}
	++manager->num_area;
	merge_adjacent();
}

void print_area() {
	for (size_t i = 0; i < manager->num_area; i++) {
		kprint("Area [%d]: (%p -> %p)\n", i, manager->areas[i].start,
			   manager->areas[i].end);
	}
	kprint("\n");
}

void *find_free(size_t size) {
	if (!size)
		return NULL;
	void *ptr = NULL;
	for (size_t i = 0; i < manager->num_area; i++) {
		uint32_t index = manager->num_area - i - 1;
		uint32_t range =
			manager->areas[index].end - manager->areas[index].start;
		if (range >= size) {
			ptr = (void *)manager->areas[index].start;
			manager->areas[index].start += size;
			if (manager->areas[index].start == manager->areas[index].end &&
				manager->num_area >= 2)
			{
				kmemmove(&manager->areas[index], &manager->areas[index + 1],
						 (manager->num_area - index - 1) * sizeof(vm_area_t));
				--manager->num_area;
			}
			return ptr;
		}
	}
	return NULL;
}
