; Multiboot Kernel Bootloader
BITS 32

; Multiboot flags
MODULEALIGN        equ 1 << 0         ; Align loaded modules on page boundaries
MEMINFO            equ 1 << 1         ; Provide memory map (BIOS e820)
FLAGS              equ MODULEALIGN | MEMINFO

; Multiboot header magic number and checksum
MAGIC              equ 0x1BADB002     ; Magic number for multiboot
CHECKSUM           equ -(MAGIC + FLAGS)

; Kernel constants
KERNEL_VIRTUAL_BASE        equ 0xC0000000     ; Virtual base of the kernel (3GB)
PAGE_SIZE          equ 4096           ; Page size (4KB)
PAGE_PERM          equ 0x3            ; Page permissions: present, read/write
STACK_SIZE         equ 4 * PAGE_SIZE  ; Kernel stack size (16KB)


extern _KERNEL_START
extern _KERNEL_END

extern _EARLY_KERNEL_START
extern _EARLY_KERNEL_END
extern kernel_main

; Multiboot header section
section .__mbHeader
align 4
  dd MAGIC
  dd FLAGS
  dd CHECKSUM

; Early boot code section
section .early_text progbits
global _start

_start:
  ; Save multiboot information from GRUB
  mov [multiboot_magic], eax
  mov [multiboot_info], ebx

  ; Identity map low memory (physical address = virtual address)
  ; first 1M
  mov eax, lowmem_page_table
  mov [page_directory], eax
  or dword [page_directory], PAGE_PERM

  xor eax, eax   ; Start at physical address 0

.lowmem:
    mov ecx, eax
    shr ecx, 12
    and ecx, 1023  ; Index for page table entry

    ; Map the first MB 
    mov [lowmem_page_table + ecx * 4], eax
    or dword [lowmem_page_table + ecx * 4], PAGE_PERM
    add eax, PAGE_SIZE
    cmp eax, _EARLY_KERNEL_END
    jl .lowmem

    ; Map the kernel in the higher half (starting at KERNEL_BASE)
    mov ecx, (KERNEL_VIRTUAL_BASE >> 22)
    mov eax, kernel_page_table
    mov [page_directory + ecx * 4], eax
    or dword [page_directory + ecx * 4], PAGE_PERM

    mov eax, _KERNEL_START

.higher:
    mov ecx, eax
    shr ecx, 12
    and ecx, 1023 ; Index for page table entry

    mov ebx, eax
    sub ebx, KERNEL_VIRTUAL_BASE   ; Virtual to physical

    mov [kernel_page_table + ecx * 4], ebx
    or dword [kernel_page_table + ecx * 4], PAGE_PERM

    add eax, PAGE_SIZE
    cmp eax, _KERNEL_END
    jl .higher

    ; Load page directory and enable paging
    mov eax, page_directory
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax  ; Enable paging

    ; Adjust the stack and call the kernel
    mov esp, stack + STACK_SIZE
    push dword [multiboot_magic]
    push dword [multiboot_info]
    call kernel_main  ; Jump to kernel main function

.loop:
    hlt
    jmp short .loop

; Page directories and page tables
section .early_bss nobits
alignb 4096
page_directory:
  resd 1024        ; Page directory (1024 entries)

lowmem_page_table:
  resd 1024        ; Page table for low memory

kernel_page_table:
  resd 1024        ; Page table for kernel

section .early_data
multiboot_magic:
  dd 0
multiboot_info:
  dd 0

; Stack section
section .bss
align 4
stack:
  resb STACK_SIZE  ; Kernel stack (16KB)
