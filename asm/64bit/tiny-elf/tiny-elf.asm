bits 64
            org 0x08048000

ehdr:                                           ; Elf64_Ehdr
            db  0x7F, "ELF", 2, 1,              ;   e_ident
_start:
            mov di,42        ; only the low byte of the exit code is kept,
                            ; so we can use di instead of the full edi/rdi
            xor eax,eax
            mov al,60        ; shorter than mov eax,60
            syscall          ; perform the syscall
            dw  2                               ;   e_type
            dw  62                              ;   e_machine
            dd  1                               ;   e_version
            dq  _start                          ;   e_entry
            dq  phdr - $$                       ;   e_phoff
            dq  0                               ;   e_shoff
            dd  0                               ;   e_flags
            dw  ehdrsize                        ;   e_ehsize
            dw  phdrsize                        ;   e_phentsize
phdr:                                           ; Elf64_Phdr
            dw  1                               ;   e_phnum         p_type
            dw  0                               ;   e_shentsize
            dw  5                               ;   e_shnum         p_flags
            dw  0                               ;   e_shstrndx
ehdrsize    equ $ - ehdr
            dq  0                               ;   p_offset
            dq  $$                              ;   p_vaddr
            dq  $$                              ;   p_paddr
            dq  filesize                        ;   p_filesz
            dq  filesize                        ;   p_memsz
            dq  0x1000                          ;   p_align

phdrsize    equ     $ - phdr
filesize    equ     $ - $$
