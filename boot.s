bits 32  ; Specify the processor mode to be 32-bit

; Define constants for multiboot flags
MODULEALIGN         equ 1 << 0                ; Align loaded modules on page boundaries
MEMINFO             equ 1 << 1                ; Provide memory map
FLAGS               equ MODULEALIGN | MEMINFO ; Combine the flags for alignment and memory map

; Multiboot header constants
MAGIC_NUMBER        equ 0x1BADB002  ; The magic number required by the multiboot specification
CHECKSUM            equ -(MAGIC_NUMBER + FLAGS) ; Checksum must make the header's magic number + flags + checksum equal zero

; Kernel stack size
KERNEL_STACK_SIZE   equ 8192                        ; Size of the stack in bytes (8 KB)
KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GB
KERNEL_PAGE_NUMBER  equ (KERNEL_VIRTUAL_BASE >> 22)  ; Page directory index of kernel's 4MB PTE.


section .data
align 0x1000
BootPageDirectory:
    ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB.
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.
    ; This entry must be here -- otherwise the kernel will crash immediately after paging is
    ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0                 ; Pages before kernel space.
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0  ; Pages after the kernel image.

; Section for the multiboot header
section .multiboot
align 4
    dd MAGIC_NUMBER    ; Store the multiboot magic number
    dd FLAGS           ; Store the flags defined above
    dd CHECKSUM        ; Store the checksum

; Uninitialized data section (BSS)
section .bss
align 16

stack_bottom:
    resb KERNEL_STACK_SIZE ; Reserve space for the kernel stack
stack_top:

; Code section
section .text

loader equ (_start - 0xC0000000)
global loader

; Entry point for the bootloader
_start:
    mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx                                        ; Load Page Directory Base Register.

    mov ecx, cr4
    or ecx, 0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000                          ; Set PG bit in CR0 to enable paging.
    mov cr0, ecx

    lea ecx, [start_higher]
    jmp ecx  

start_higher:
    mov dword [BootPageDirectory], 0
    invlpg [0]
    mov esp, stack_top   ; Set the stack pointer (ESP) to the top of the stack
    extern kernel_main   ; Declare external reference to the kernel_main function
    call kernel_main     ; Call the kernel_main function
    cli                  ; Disable interrupts

.hang:                   ; Infinite loop to halt the CPU0x1BADB002           ;  if kernel_main returns
    hlt                  ; Halt the CPU
    jmp .hang            ; Jump back to the halt instruction

.end:  ; Label for end of the _start function
