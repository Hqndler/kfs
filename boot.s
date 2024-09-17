bits 32  ; Specify the processor mode to be 32-bit

; Define constants for multiboot flags
ALIG                equ 1 << 0      ; Align loaded modules on page boundaries
MEMINFO             equ 1 << 1      ; Provide memory map
FLAGS               equ ALIG | MEMINFO ; Combine the flags for alignment and memory map

; Multiboot header constants
MAGIC_NUMBER        equ 0x1BADB002  ; The magic number required by the multiboot specification
CHECKSUM            equ -(MAGIC_NUMBER + FLAGS) ; Checksum must make the header's magic number + flags + checksum equal zero

; Kernel stack size
KERNEL_STACK_SIZE   equ 8192        ; Size of the stack in bytes (8 KB)

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
global _start

; Entry point for the bootloader
_start:
    mov esp, stack_top   ; Set the stack pointer (ESP) to the top of the stack
    extern kernel_main   ; Declare external reference to the kernel_main function
    call kernel_main     ; Call the kernel_main function
    cli                  ; Disable interrupts

.hang:                   ; Infinite loop to halt the CPU if kernel_main returns
    hlt                  ; Halt the CPU
    jmp .hang            ; Jump back to the halt instruction

.end:  ; Label for end of the _start function
