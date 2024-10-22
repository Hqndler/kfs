#include "heap.h"

void init_bitmaps(struct multiboot_info *mbi) {
	uint32_t total_memory = (mbi->mem_lower + mbi->mem_upper) * 1024;
	uint32_t total_pages = total_memory / PAGE_SIZE;
	uint32_t size_bitmap = (total_pages + 31) / 32;

	physical_bitmap.data =
		alloc_early_boot_memory(size_bitmap * sizeof(uint32_t));
	physical_bitmap.size = size_bitmap;

	kmemset(physical_bitmap.data, 0xFF,
			physical_bitmap.size * sizeof(uint32_t));

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
	multiboot_memory_map_t *mmap_end =
		(multiboot_memory_map_t *)(mbi->mmap_addr + mbi->mmap_length);

	while (mmap < mmap_end) {
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE && mmap->addr != 0) {
			uint32_t start_page = (uint32_t)(mmap->addr / PAGE_SIZE);
			uint32_t end_page =
				(uint32_t)((mmap->addr + mmap->len) / PAGE_SIZE);

			for (uint32_t page = start_page; page < end_page; page++)
				BITMAP_CLEAR(&physical_bitmap, page);
		}

		mmap = (multiboot_memory_map_t *)((uint32_t)mmap + mmap->size +
										  sizeof(mmap->size));
	}

	uint32_t start_page = (KERNEL_PHYSICAL_BASE) / PAGE_SIZE;
	uint32_t end_page = ((KERNEL_END - KERNEL_VIRTUAL_BASE)) / PAGE_SIZE;

	for (uint32_t page = start_page; page < end_page; page++)
		BITMAP_SET(&physical_bitmap, page);
}

void *alloc_early_boot_memory(size_t size) {
	size = ALIGN(size, 32);
	if (early_boot_next_free + size >= EARLY_KMALLOC_END)
		return NULL;
	void *allocated = (char *)early_boot_next_free;
	early_boot_next_free += size;
	return allocated;
}

static slab_cache_t caches[NUM_CACHES];
static void *brk = NULL;

big_object_t *big_list = NULL;

void init_slab_allocator() {
	size_t sizes[] = {32, 64, 128, 256, 512, 1024, 2048};
	for (int i = 0; i < NUM_CACHES; i++) {
		caches[i].size = sizes[i];
		caches[i].max_obj = (PAGE_SIZE - sizeof(slab_t)) / sizes[i];
		caches[i].slabs = NULL;
	}
}

static void *allocate_slab(slab_cache_t *cache) {
	slab_t *new_slab = get_pages(1);
	if (!new_slab)
		return NULL;

	new_slab->next = cache->slabs;
	cache->slabs = new_slab;

	uint8_t *slab_mem = (uint8_t *)new_slab + sizeof(slab_t);
	new_slab->free_list = slab_mem;

	for (size_t i = 0; i < cache->max_obj - 1; i++) {
		void **obj = (void **)(slab_mem + i * cache->size);
		*obj = slab_mem + (i + 1) * cache->size;
	}
	void **last_obj = (void **)(slab_mem + (cache->max_obj - 1) * cache->size);
	*last_obj = NULL;

	return new_slab;
}

void *slab_alloc(size_t size, int contig);

static void *handle_big_alloc(size_t size, int contig) {
	void *ptr = contig ? get_cpages((size / PAGE_SIZE) + 1) :
						 get_pages((size / PAGE_SIZE) + 1);

	if (!ptr)
		return NULL;

	if (!big_list) {
		big_list = alloc_early_boot_memory(sizeof(big_object_t));
		big_list->size = size;
		big_list->ptr = ptr;
		big_list->next = NULL;
	}

	else {
		big_object_t *new = alloc_early_boot_memory(sizeof(big_object_t));
		if (new) {
			new->next = big_list;
			new->size = size;
			new->ptr = ptr;
			big_list = new;
		}
	}
	return ptr;
}

void *slab_alloc(size_t size, int contig) {

	size += sizeof(slab_cache_t);

	slab_cache_t *cache = NULL;
	size_t cache_index = LOG2(UPPER_2_POWER(size)) - 5;
	if (cache_index < NUM_CACHES)
		cache = &caches[cache_index];

	if (!cache)
		return handle_big_alloc(size, contig);

	if (!cache->slabs || !cache->slabs->free_list) {
		ASSERT_PANIC(allocate_slab(cache), "Out Of Memory");
	}

	slab_object_t *object = cache->slabs->free_list;
	cache->slabs->free_list = *(void **)object;
	object->cache = cache;
	object->size = size;
	return object->data;
}

static void free_big_object(void *ptr, size_t size) {
	big_object_t *cur = big_list;
	if (cur->ptr == ptr) {
		big_list = NULL;
		for (size_t i = 0; i < (size / PAGE_SIZE) + 1; i++)
			free_page(ptr + (i * PAGE_SIZE));
		return;
	}
	while (cur && cur->next) {
		if (cur->next == ptr) {
			cur->next = cur->next->next;
			for (size_t i = 0; i < (size / PAGE_SIZE) + 1; i++)
				free_page(ptr + (i * PAGE_SIZE));
			return;
		}
		cur = cur->next;
	}
}

static void slab_free(void *ptr) {
	if (!ptr)
		return;

	size_t size = ksize(ptr);
	if (size > caches[NUM_CACHES - 1].size) {
		free_big_object(ptr, size);
		return;
	}

	slab_object_t *object =
		(slab_object_t *)((uint8_t *)ptr - offsetof(slab_object_t, data));
	slab_t *slab = (slab_t *)((uintptr_t)object & ~(PAGE_SIZE - 1));
	*(void **)object = slab->free_list;
	slab->free_list = object;
}

void *kmalloc(size_t size) {
	return slab_alloc(size, 1);
}

void kfree(void *ptr) {
	slab_free(ptr);
}

void *vmalloc(size_t size) {
	return slab_alloc(size, 0);
}

void init_brk() {
	brk = get_pages(1);
	ASSERT_PANIC(brk, "Out Of Memory");
}

void *kbrk(int32_t increment) {
	static uint32_t allocated = 0;
	if (increment <= 0)
		return brk;
	if (allocated + increment >= PAGE_SIZE) {
		allocated = 0;
		size_t pages = (increment / PAGE_SIZE) + 1;
		void *ptr = get_cpages(pages);
		ASSERT_PANIC(ptr, "Out Of Memory");
		brk = ptr + increment;
		return ptr;
	}
	allocated += increment;
	void *ptr = brk;
	brk += increment;
	return ptr;
}

void *vbrk(int32_t increment) {
	static uint32_t allocated = 0;
	if (increment <= 0)
		return brk;
	if (allocated + increment >= PAGE_SIZE) {
		allocated = 0;
		size_t pages = (increment / PAGE_SIZE) + 1;
		void *ptr = get_pages(pages);
		ASSERT_PANIC(ptr, "Out Of Memory");
		brk = ptr + increment;
		return ptr;
	}
	allocated += increment;
	void *ptr = brk;
	brk += increment;
	return ptr;
}

size_t get_size(void const *ptr, int is_virtual) {
	if (!ptr)
		return 0;

	big_object_t *cur = big_list;
	while (cur) {
		if (cur->ptr == ptr)
			return cur->size - (is_virtual * sizeof(slab_cache_t));
		cur = cur->next;
	}

	slab_object_t const *object =
		(slab_object_t const *)((uint8_t const *)ptr -
								offsetof(slab_object_t, data));
	if (!object || !object->cache)
		return 0;
	return object->size - (is_virtual * sizeof(slab_cache_t));
}

size_t ksize(void const *ptr) {
	return get_size(ptr, 0);
}

size_t vsize(void const *ptr) {
	return get_size(ptr, 1);
}

void print_big_list() {

	big_object_t *cur = big_list;
	while (cur) {
		kprint("(size: %d, ptr: %p) -> ", cur->size, cur->ptr);
		cur = cur->next;
	}
	kprint("NULL\n");
}

void test_malloc() {
	kprint("===============Test Malloc===============\n");
	kprint("");
	char const *msg = "Ceci est un test !";
	size_t size = kstrlen(msg) + 1;
	char *str1 = kmalloc(size * sizeof(char));
	if (!str1)
		return kprint("NULL DETECTED !\n");
	kmemset(str1, 0, size);
	kmemcpy(str1, msg, size);
	kprint("%s\n", str1);
	uint16_t arr1[8192];
	uint16_t *arr2 = kmalloc(8192 * sizeof(uint16_t));
	uint16_t *arr3 = vmalloc(8192 * sizeof(uint16_t));
	if (!arr2 || !arr3)
		return kprint("NULL DETECTED !\n");
	kmemset(&arr1[0], 42, 8192 * sizeof(uint16_t));
	kmemset(arr2, 42, 8192 * sizeof(uint16_t));
	kmemset(arr3, 42, 8192 * sizeof(uint16_t));

	kprint("arr1 == arr2: %s\n",
		   kmemcmp(arr1, arr2, 8192 * sizeof(uint16_t)) ? "False" : "True");
	kprint("arr1 == arr3: %s\n",
		   kmemcmp(arr1, arr3, 8192 * sizeof(uint16_t)) ? "False" : "True");
	kprint("arr2 == arr3: %s\n",
		   kmemcmp(arr2, arr3, 8192 * sizeof(uint16_t)) ? "False" : "True");

	kprint("arr2 ksize: %d, vsize: %d\n", ksize(arr2), vsize(arr2));
	kprint("arr3 ksize: %d, vsize: %d\n", ksize(arr3), vsize(arr3));

	// char *invalid = kmalloc(PAGE_SIZE - 100);
	// if (!invalid)
	// 	return kprint("NULL DETECTED !\n");
	// invalid[PAGE_SIZE] = 't';

	void *ptr = kmalloc(42);
	kmemset(ptr, 21, 42);
	kprint("ptr : %p\n", ptr);
	kfree(ptr);

	void *ptr2 = kmalloc(42);
	kmemset(ptr2, 21, 42);
	kprint("ptr2: %p\n", ptr2);

	void *ptr3 = kbrk(32);
	kmemset(ptr3, 31, 32);
	kprint("ptr3: %p\n", ptr3);

	void *ptr4 = vbrk(32);
	kmemset(ptr4, 31, 32);
	kprint("ptr4: %p\n", ptr4);

	kfree(arr2);
	kfree(arr3);
	kfree(ptr);
	kfree(ptr2);

	kprint("=========================================\n");
}
