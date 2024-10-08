bits 32

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

KERNEL_VIRT     equ 0xC0000000
KERNEL_PHY      equ 0x00000000

; Section for the multiboot header
section .multiboot
align 4
    dd MAGIC_NUMBER    ; Store the multiboot magic number
    dd FLAGS           ; Store the flags defined above
    dd CHECKSUM        ; Store the checksum

section .text
global kernel_entry
kernel_entry:
    ; Fill Page directory
    mov ecx, 1024
    mov esp, BootPageDirectory
    sub esp, KERNEL_VIRT
    fill_dir:
        mov DWORD [esp], 0
        add esp, 4
    loop fill_dir

    mov esp, BootPageDirectory
    sub esp, KERNEL_VIRT

    ; The kernel is identity mapped because enabling paging does
    ; not change the next instruction, which continues to be physical.
    ; The CPU would instead page fault if there was no identity mapping.
    mov DWORD [esp], KERNEL_PHY + 131
    mov DWORD [esp + 4 * (KERNEL_VIRT >> 22)], KERNEL_PHY + 131

    ; Enable paging
    mov ecx, BootPageDirectory
    sub ecx, KERNEL_VIRT
    mov cr3, ecx            ; Load Page Directory Base Register.

    mov esp, cr4
    or esp, 0x00000010       ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, esp

    mov esp, cr0
    or esp, 0x80000001
    mov cr0, esp            ; Set PG bit in CR0 to enable paging.

    lea ecx, [high_entry]
    jmp ecx

section .text
extern kernel_main                ; Symbol defined in sources
high_entry:
    mov dword [BootPageDirectory], 0
    invlpg [0]
    mov esp, stack_top
    push ebx
    call kernel_main
    cli

.hang:                   ; Infinite loop to halt the CPU0x1BADB002           ;  if kernel_main returns
    hlt                  ; Halt the CPU
    jmp .hang            ; Jump back to the halt instruction

.end:  ; Label for end of the _start function

section .bss
align 4096
global BootPageDirectory         ; Page Directory Defined
BootPageDirectory:
    resb 4096

section .bss
global stack_bottom
global stack_top

align 4096
stack_bottom:
    resb 0x4000            ; Reserve 16kb Stack Memory
    align 4096
stack_top: