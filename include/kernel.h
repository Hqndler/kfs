#pragma once
#include "gdt.h"
#include "heap.h"
#include "input_buffer.h"
#include "interrupt.h"
#include "multiboot.h"
#include "paging.h"
#include "panic.h"
#include "syscalls.h"
#include "virtual_manager.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PROMPT_STR "$>"
#define PROMPT_LEN sizeof(PROMPT_STR) - 1

typedef struct {
	uint32_t eax, ebx, ecx, edx;
	uint32_t esi, edi, ebp, esp;
	uint32_t eip, eflags;
	uint16_t cs, ds, es, fs, gs, ss;
} registers_t;

#define GET_EAX(x) asm volatile("mov %%eax, %0" : "=r"(x))
#define GET_EBX(x) asm volatile("mov %%ebx, %0" : "=r"(x))
#define GET_ECX(x) asm volatile("mov %%ecx, %0" : "=r"(x))
#define GET_EDX(x) asm volatile("mov %%edx, %0" : "=r"(x))
#define GET_ESI(x) asm volatile("mov %%esi, %0" : "=r"(x))
#define GET_EDI(x) asm volatile("mov %%edi, %0" : "=r"(x))
#define GET_EBP(x) asm volatile("mov %%ebp, %0" : "=r"(x))
#define GET_ESP(x) asm volatile("mov %%esp, %0" : "=r"(x))
#define GET_EIP(x) asm volatile("mov $., %0" : "=r"(x));
#define GET_EFLAGS(x)                                                          \
	asm volatile("pushf\n\t"                                                   \
				 "pop %0"                                                      \
				 : "=r"(x)::)

#define GET_CS(x) asm volatile("mov %%cs, %0" : "=r"(x))
#define GET_DS(x) asm volatile("mov %%ds, %0" : "=r"(x))
#define GET_ES(x) asm volatile("mov %%es, %0" : "=r"(x))
#define GET_FS(x) asm volatile("mov %%fs, %0" : "=r"(x))
#define GET_GS(x) asm volatile("mov %%gs, %0" : "=r"(x))
#define GET_SS(x) asm volatile("mov %%ss, %0" : "=r"(x))

#define GET_REGISTERS(regs)                                                    \
	do {                                                                       \
		GET_EAX((regs).eax);                                                   \
		GET_EBX((regs).ebx);                                                   \
		GET_ECX((regs).ecx);                                                   \
		GET_EDX((regs).edx);                                                   \
		GET_ESI((regs).esi);                                                   \
		GET_EDI((regs).edi);                                                   \
		GET_EBP((regs).ebp);                                                   \
		GET_ESP((regs).esp);                                                   \
		GET_EIP((regs).eip);                                                   \
		GET_EFLAGS((regs).eflags);                                             \
		GET_CS((regs).cs);                                                     \
		GET_DS((regs).ds);                                                     \
		GET_ES((regs).es);                                                     \
		GET_FS((regs).fs);                                                     \
		GET_GS((regs).gs);                                                     \
		GET_SS((regs).ss);                                                     \
	} while (0)

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

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* Globals */

extern size_t kernel_screen;
extern size_t screen_cursor[10];
extern uint16_t screen_buffer[10][2000];
extern uint8_t terminal_color;
extern uint16_t *terminal_buffer;

extern void (*func[255])(uint8_t code);

extern bool is_cmd;
extern bool is_hlt;

extern char *last_cmd;

extern uint32_t ticks;

// Simplified storage varables (see memory.c)
extern uint32_t KERNEL_START;
extern uint32_t KERNEL_END;
extern uint32_t EARLY_KMALLOC_START;
extern uint32_t EARLY_KMALLOC_END;
extern uintptr_t early_boot_next_free;

/* Keyboard */

void fb_move_cursor(uint16_t pos);
void toggle_caps(uint8_t code);
void toggle_ctrl(uint8_t code);
void toggle_num(uint8_t code);
void handle_code(uint8_t code);
void handle_extended(uint8_t code);
uint8_t get_scan_code();

void delete_char(uint8_t code);

/* VGA */

void print_stack(void);

uint16_t vga_entry(unsigned char uc, uint8_t color);
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
void terminal_setcolor(uint8_t color);
void write_string_buffer(char const *str);
void terminal_initialize(void);
void vga_init();
void switch_screen(int n);
void terminal_putchar(char c);
void terminal_writestring(char const *data);
void terminal_putnbr(uint32_t n);
void terminal_puthexa(uint32_t n);
void init_buffers(void);
void init_keyboard(void);
void switch_layout(uint8_t code);
void prompt(char c);
void exec(void);
void terminal_putprompt(void);

/* Utils */

void *kmemset(void *pointer, uint8_t value, size_t count);
void *kvgaset(void *pointer, uint16_t value, size_t count);
void *kmemcpy(void *destination, void const *source, size_t size);
void *kmemshift(void *source, const uint8_t byte, size_t pos, size_t size);
void *kvgashift(void *source, const uint16_t byte, size_t pos, size_t size);
void *kmemmove(void *destination, void const *source, size_t size);
size_t kstrlen(char const *str);
char *kitoa(char *buff, uint32_t n, size_t len);
char *kxitoa(char *buff, uint64_t n, size_t len, bool caps);
char *kbitoa(char *buff, uint32_t n, size_t len);
int katoi(char const *nptr);
uint32_t kaxtoi(char const *nptr);
int kstrncmp(char const *s1, char const *s2, size_t n);
char *kstrchr(char const *string, int c);
void kprint(char const *fmt, ...);
void halt(void);
void reboot(uint8_t code);
int kmemcmp(void const *p1, void const *p2, size_t size);
int kstrcmp(char const *s1, char const *s2);
void print_multiboot(struct multiboot_info *mbi);
void kpanic(char const *error);
void disable_cursor();

char *kstrdup(char const *str);
char *get_line(char const *msg);

extern void trigger_interrupt(uint8_t number);
