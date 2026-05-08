; true - always exits with code 0
BITS 32
ORG 0x08048000				; Standard Linux ELF load address

; ELF Header (52 bytes)
ehdr:
	db 0x7F, "ELF", 1, 1, 1, 0	; Magic Number + 32-bit little-endian
	times 8 db 0			; Padding
	dw 2				; Type: ET_EXEC
	dw 3				; Machine: x86
	dd 1				; Version
	dd _start			; Entry Point
	dd phdr - $$			; Program Header Offset
	dd 0, 0				; No section headers
	dw 0x34, 0x20			; ELF Header Size, Program Header Size
	dw 1				; Number of Program Headers
	dw 0, 0, 0			; No section headers

; Program Header (32 bytes)
phdr:
	dd 1				; Type: PT_LOAD
	dd 0				; Offset
    	dd $$				; Virtual Address
	dd $$				; Physical Address
	dd filesize			; File size
	dd filesize			; Memory size
	dd 5				; Flags: R + X
	dd 0x1000			; Alignment

; Code (3 bytes)
_start:
	; assume that eax is already 0 at program start, increment to 1
	inc eax				; eax = 1 (sys_exit)
	; assume that ebx is already 0 at program start, keep 0 for exit code
	int 0x80			; syscall

filesize equ $ - $$
