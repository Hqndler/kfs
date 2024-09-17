#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void *kmemset(void *pointer, uint8_t value, size_t count) {
	uint8_t *str;

	str = pointer;
	while (count--) {
		*str = (uint8_t)value;
		str++;
	}
	return (pointer);
}

void *kvgaset(void *pointer, uint16_t value, size_t count) {
	uint16_t *str;

	str = pointer;
	while (count--) {
		*str = (uint16_t)value;
		str++;
	}
	return (pointer);
}

void *kmemcpy(void *destination, void const *source, size_t size) {
	uint8_t *d;
	uint8_t *s;
	size_t i;

	d = (uint8_t *)destination;
	s = (uint8_t *)source;
	i = -1;
	while (++i < size)
		d[i] = s[i];
	return (destination);
}

void *kvgashift(void *source, const uint16_t byte, size_t pos, size_t size) {
	if (pos >= size)
		return source;

	for (size_t i = size; i > pos; --i)
		((uint16_t *)source)[i] = ((uint16_t *)source)[i - 1];
	((uint16_t *)source)[pos] = byte;
	return source;
}

void *kmemshift(void *source, const uint8_t byte, size_t pos, size_t size) {
	if (pos >= size)
		return source;

	for (size_t i = size; i > pos; --i)
		((uint8_t *)source)[i] = ((uint8_t *)source)[i - 1];
	((uint8_t *)source)[pos] = byte;
	return source;
}

void *kmemmove(void *destination, void const *source, size_t size) {
	size_t i;
	unsigned char *d;
	unsigned char *s;

	d = (unsigned char *)destination;
	s = (unsigned char *)source;
	i = -1;
	if (d > s)
		while (size-- > 0)
			d[size] = s[size];
	else
		while (++i < size)
			d[i] = s[i];
	return (destination);
}

size_t kstrlen(char const *str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

char *kitoa(char *buff, unsigned long n, size_t len) {
	size_t pos = len;
	bool is_neg = false;
	if ((long)n < 0) {
		n = -n;
		is_neg = true;
	}

	buff[--pos] = '\0';
	while (n >= 10) {
		buff[--pos] = n % 10 + '0';
		n /= 10;
	}
	buff[--pos] = n + '0';
	if (is_neg)
		buff[--pos] = '-';
	return &buff[pos];
}

char *kxitoa(char *buff, unsigned long n, size_t len, bool caps) {
	size_t pos = len;
	static char const table[2][16] = {
		{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
		 'e', 'f'},
		{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
		 'E', 'F'}
	  };
	buff[--pos] = '\0';
	do {
		unsigned int digit = (n % 16);
		n /= 16;
		buff[--pos] = table[caps][digit];
	} while (n);
	return &buff[pos];
}
