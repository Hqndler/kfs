#pragma once

#include "kernel.h"
#include <stddef.h>

typedef struct {
	char *buffer;
	size_t size;
	size_t capacity;
	size_t cursor;
} input_buffer_t;

extern int line_ready;
extern input_buffer_t input_buffer;

void init_input_buffer();
void insert_buff_char(char c);
void backspace();
void delete_char_buffer();
void move_input_cursor(int32_t increment);
