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


global skip_instruction
skip_instruction:
	mov edx, [esp + 16]
	add edx, 12
	mov [esp + 16], edx
	sti
