ENTRY(kernel_entry)
OUTPUT_FORMAT(elf32-i386)

kernel_virt_pos = 0xC0000000;

SECTIONS
{
    . = 0x00100000;

    .multiboot : {
        *(.multiboot)
    }

    . += kernel_virt_pos;

    .text ALIGN(4K) : AT(ADDR(.text) - kernel_virt_pos) {
        *(.text*)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - kernel_virt_pos) {
        *(.rodata*)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - kernel_virt_pos) {
        *(.data*)
    }

    /* BSS */
    .bss ALIGN(4K) : AT(ADDR(.bss) - kernel_virt_pos) {
        *(COMMON)
        *(.bss)
    }
}