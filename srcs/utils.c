#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

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

	for (size_t i = size - 1; i > pos; --i)
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

char *kitoa(char *buff, uint32_t n, size_t len) {
	size_t pos = len;
	bool is_neg = false;
	if ((int32_t)n < 0) {
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

char *kxitoa(char *buff, uint64_t n, size_t len, bool caps) {
	size_t pos = len;
	static char const table[2][16] = {
		{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
		 'e', 'f'},
		{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
		 'E', 'F'}
	  };
	buff[--pos] = '\0';
	do {
		uint32_t digit = (n % 16);
		n /= 16;
		buff[--pos] = table[caps][digit];
	} while (n);
	return &buff[pos];
}

char *kbitoa(char *buff, uint32_t n, size_t len) {
	size_t pos = len;
	static char const table[2] = {'0', '1'};
	buff[--pos] = '\0';
	do {
		uint32_t digit = (n % 2);
		n /= 2;
		buff[--pos] = table[digit];
	} while (n);
	return &buff[pos];
}

int	katoi(const char *nptr) {
	int		nbr, sign, i;
	char	*str;

	i = 0;
	str = (char *)nptr;
	while ((str[i] >= 9 && str[i] <= 13) || str[i] == 32)
		i++;
	sign = 1;
	if (str[i] == '+' || str[i] == '-')
		if (str[i++] == '-')
			sign = -sign;
	nbr = 0;
	while (str[i] >= '0' && str[i] <= '9')
		nbr = nbr * 10 + (str[i++] - 48);
	return (nbr * sign);
}

int	kstrncmp(const char *s1, const char *s2, size_t n) {
	unsigned int	i;

	if (n == 0)
		return (-1);
	i = 0;
	while ((s1[i] && s2[i]) && i < n - 1)
	{
		if (s1[i] != s2[i])
			break ;
		i++;
	}
	return (s1[i] - s2[i]);
}

char	*kstrchr(const char *string, int c) {
	int	i = -1;

	while (string[++i])
		if (string[i] == (unsigned char)c)
			return ((char *)&string[i]);
	if (string[i] == (unsigned char)c)
		return ((char *)&string[i]);
	return (NULL);
}

void halt() {
	asm("hlt");
}

void reboot() {
	uint8_t good = 0x02;
	while (good & 0x02)
		good = inb(0x64);
	outb(0x64, 0xFE);
}

int kmemcmp(void const *p1, void const *p2, size_t size) {
	uint8_t const *s1 = (uint8_t const *)p1;
	uint8_t const *s2 = (uint8_t const *)p2;
	while (size-- > 0) {
		if (*s1++ != *s2++)
			return *--s1 < *s2 ? -1 : 1;
	}
	return 0;
}

int kstrcmp(char const *s1, char const *s2) {
	size_t i;

	if (!s1 || !s2)
		return (0);
	i = 0;
	while (s1[i] == s2[i]) {
		if (!s1[i] && !s2[i])
			return (0);
		i++;
	}
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
