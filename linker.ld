ENTRY (_start)

SECTIONS
{
    /* Base address starting at 1MB */
    . = 0x00100000;
    _EARLY_KERNEL_START = .;

    /* Special multiboot compliant header.
     * Used by GRUB for image detection.
     */
    .__mbHeader : { *(.__mbHeader) }
    .early_text ALIGN (0x1000) : { *(.early_text) }
    .early_data ALIGN (0x1000) : { *(.early_data) }
    .early_bss ALIGN (0x1000) :
    {
        *(.early_bss)
    }

    _EARLY_KERNEL_END = .;

    /* Higher half (3 GB) */
    . += 0xC0000000;

    /* Let the source know about the kernel start */
    _KERNEL_START = .;

    /* make sure the sections are page aligned */
    .text ALIGN (0x1000) : AT(ADDR(.text) - 0xC0000000) { *(.text) }
    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xC0000000) { *(.rodata*) }
    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000) { *(.data) }
    .bss ALIGN (0x1000) : AT(ADDR(.bss) - 0xC0000000)
    {
        *(.bss)
        *(COMMON)
    }
    .early_kmalloc ALIGN (0x1000) : AT(ADDR(.early_kmalloc) - 0xC0000000)
    {
        _EARLY_KMALLOC_START = .;
        . += 0x100000; /* 1MB of heap space */
        _EARLY_KMALLOC_END = .;
    }

    /* Let the source know about the kernel end */
    _KERNEL_END = .;
    _KERNEL_SIZE = _KERNEL_END - _KERNEL_START;
}
