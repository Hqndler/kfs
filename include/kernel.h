#pragma once
#include "gdt.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Hardware text mode color constants. */
enum vga_color
{
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

#define KERN_EMERG "0"
#define KERN_ALERT "1"
#define KERN_CRIT "2"
#define KERN_ERR "3"
#define KERN_WARN "4"
#define KERN_NOTICE "5"
#define KERN_INFO "6"
#define KERN_DEBUG "7"
#define KERN_DEFAULT ""

/* Globals */

extern size_t kernel_screen;
extern size_t screen_cursor[10];
extern uint8_t screen_buffer[10][2000];
extern uint8_t terminal_color;
extern uint16_t *terminal_buffer;

extern void (*func[255])(unsigned char code);

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* Keyboard */

void fb_move_cursor(unsigned short pos);
void toggle_caps(unsigned char code);
void toggle_ctrl(unsigned char code);
void toggle_num(unsigned char code);
void handle_code(unsigned char code);
void handle_extended(unsigned char code);
unsigned char get_scan_code();

void delete_char(unsigned char code);

/* VGA */

void print_stack(void);

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
void terminal_setcolor(uint8_t color);
void write_string_buffer(char *str);
void terminal_initialize(void);
void vga_init();
void switch_screen(int n);
void terminal_putchar(char c);
void terminal_writestring(char const *data);
void terminal_putnbr(unsigned int n);
void terminal_puthexa(unsigned long n);
void init_buffers(void);

/* Utils */

void *kmemset(void *pointer, uint8_t value, size_t count);
void *kvgaset(void *pointer, uint16_t value, size_t count);
void *kmemcpy(void *destination, void const *source, size_t size);
void *kmemshift(void *source, const uint8_t byte, size_t pos, size_t size);
void *kvgashift(void *source, const uint16_t byte, size_t pos, size_t size);
void *kmemmove(void *destination, void const *source, size_t size);
size_t kstrlen(char const *str);
char *kitoa(char *buff, unsigned long long n, size_t len);
char *kxitoa(char *buff, unsigned long long n, size_t len, bool caps);
void kprint(char const *fmt, ...);
