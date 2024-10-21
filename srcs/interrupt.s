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
        pusha             ; Sauvegarder tous les registres généraux
        cli               ; Désactiver les interruptions
        push %1           ; Pousser le numéro de l'ISR sur la pile
        call isr_handler  ; Appeler le gestionnaire d'ISR
        add esp, 4        ; Ajuster la pile (enlever l'ISR numéro)
        popa              ; Restaurer les registres généraux
        iretd             ; Retourner de l'interruption
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
