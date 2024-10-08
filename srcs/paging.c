#include "paging.h"

void init_paging() {
	for (size_t i = 0; i < NUM_PAGES; i++)
		page_table[i] = (i * 0x1000) | 3;
	*(&BootPageDirectory) = ((uint32_t)page_table) | 3;
}
