global outb
; outb - send a byte to an I/O port
; Parameters (via stack):
; [esp + 8] - data byte
; [esp + 4] - I/O port address
outb:
    mov al, [esp + 8]    ; Load the data byte into AL
    mov dx, [esp + 4]    ; Load the I/O port address into DX
    out dx, al           ; Send the byte to the I/O port
    ret                  ; Return to the calling function

global inb
; inb - read a byte from an I/O port
; Parameters (via stack):
; [esp + 4] - I/O port address
inb:
    mov dx, [esp + 4]    ; Load the I/O port address into DX
    in al, dx            ; Read a byte from the I/O port into AL
    ret                  ; Return to the calling function


global load_gdt
load_gdt:
   mov eax, [esp+4]  
   lgdt [eax]        

   mov ax, 0x10      
   mov ds, ax       
   mov es, ax
   mov fs, ax
   mov gs, ax
   mov ss, ax
   jmp 0x08:.flush   
.flush:
   ret