global load_idt
global keyboard_handler

load_idt:
	mov edx, [esp + 4]  ; move idt ptr to edx
	lidt [edx]          ; load idt
	sti                 ; enable interruptions
	ret

extern handle_keyboard_interrupt
keyboard_handler:
	pushad                              ; save all general registers
	call handle_keyboard_interrupt      ; call keyboard handler
	popad                               ; restore all general registers
	iretd                               ; return from interuption

extern tick

global timer_handler
timer_handler:
	pushad
	call tick
	popad
	iretd

extern isr_handler

%macro ISR_EXCEPTION 1
    global isr%1
    isr%1:
        pusha             
        cli               
        push %1           
        call isr_handler  
        add esp, 4        
        popa              
        iretd             
%endmacro



ISR_EXCEPTION 0
ISR_EXCEPTION 1
ISR_EXCEPTION 2
ISR_EXCEPTION 3
ISR_EXCEPTION 4
ISR_EXCEPTION 5
ISR_EXCEPTION 6
ISR_EXCEPTION 7

ISR_EXCEPTION 8
ISR_EXCEPTION 9 
ISR_EXCEPTION 10
ISR_EXCEPTION 11
ISR_EXCEPTION 12
ISR_EXCEPTION 13
ISR_EXCEPTION 14
ISR_EXCEPTION 15
ISR_EXCEPTION 16
ISR_EXCEPTION 17
ISR_EXCEPTION 18
ISR_EXCEPTION 19
ISR_EXCEPTION 20
ISR_EXCEPTION 21
ISR_EXCEPTION 22
ISR_EXCEPTION 23
ISR_EXCEPTION 24
ISR_EXCEPTION 25
ISR_EXCEPTION 26
ISR_EXCEPTION 27
ISR_EXCEPTION 28
ISR_EXCEPTION 29
ISR_EXCEPTION 30

global get_registers
get_registers:
    mov eax, [esp+4]     
    mov [eax], eax       
    mov [eax+4], ebx     
    mov [eax+8], ecx     
    mov [eax+12], edx    
    mov [eax+16], esi    
    mov [eax+20], edi    
    mov [eax+24], ebp    
    mov [eax+28], esp    

    pushfd                
    pop dword [eax+32]    

    ; Save segment registers
    mov ax, cs
    mov [eax+36], ax      
    mov ax, ds
    mov [eax+38], ax      
    mov ax, es
    mov [eax+40], ax      
    mov ax, fs
    mov [eax+42], ax      
    mov ax, gs
    mov [eax+44], ax      
    mov ax, ss
    mov [eax+46], ax      

    ret


global clean_registers

clean_registers:
    xor eax, eax        
    xor ebx, ebx        
    xor ecx, ecx        
    xor edx, edx        
    xor esi, esi        
    xor edi, edi        

    mov ax, 0           
    mov ds, ax          
    mov es, ax          
    mov fs, ax          
    mov gs, ax          
    ret
