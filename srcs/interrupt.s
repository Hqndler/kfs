global load_idt
global keyboard_handler

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti
	ret

extern handle_keyboard_interrupt
keyboard_handler:
	pushad
	call handle_keyboard_interrupt
	popad
	iretd