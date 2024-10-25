#include "input_buffer.h"

input_buffer_t input_buffer;

void init_input_buffer() {
	input_buffer.buffer = kcalloc(VGA_WIDTH);
	ASSERT_PANIC(input_buffer.buffer, "Out Of Memory");
	input_buffer.capacity = VGA_WIDTH;
	input_buffer.size = 0;
	input_buffer.cursor = 0;
}

void insert_buff_char(char c) {
	if (input_buffer.size == input_buffer.capacity) {
		input_buffer.buffer =
			krealloc(input_buffer.buffer, input_buffer.capacity * 2);
		ASSERT_PANIC(input_buffer.buffer, "Out Of Memory");
		input_buffer.capacity *= 2;
	}
	kmemmove(&input_buffer.buffer[input_buffer.cursor + 1],
			 &input_buffer.buffer[input_buffer.cursor],
			 input_buffer.size - input_buffer.cursor);

	input_buffer.buffer[input_buffer.cursor] = c;
	input_buffer.cursor++;
	input_buffer.size++;
	input_buffer.buffer[input_buffer.size] = '\0';
}

void backspace() {
	if (input_buffer.cursor > 0) {
		kmemmove(&input_buffer.buffer[input_buffer.cursor - 1],
				 &input_buffer.buffer[input_buffer.cursor],
				 input_buffer.size - input_buffer.cursor + 1);

		input_buffer.cursor--;
		input_buffer.size--;
	}
}

void delete_char_buffer() {
	if (input_buffer.cursor < input_buffer.size) {
		kmemmove(&input_buffer.buffer[input_buffer.cursor],
				 &input_buffer.buffer[input_buffer.cursor + 1],
				 input_buffer.size - input_buffer.cursor);

		input_buffer.size--;
	}
}

void move_input_cursor(int32_t increment) {
	int new = input_buffer.cursor + increment;
	if (new >= 0 && new <= (int32_t)input_buffer.size)
		input_buffer.cursor = new;
}
