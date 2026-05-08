; true for PPC32
ORG 0x10000000

ehdr:
	db 0x7F, "ELF", 1, 2, 1, 0
	times 8 db 0
	dw 2
	dw 0x14
	dd 1
	dd _start
	dd phdr - $$
	dd 0, 0
	dw 0x34, 0x20
	dw 1
	dw 0, 0, 0

phdr:
	dd 1
	dd 0
	dd $$
	dd $$
	dd filesize
	dd filesize
	dd 5
	dd 0x1000

_start:
	li r0, 1
	li r3, 0
	sc

filesize equ $ - $$
