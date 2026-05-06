; Assemble using:
;    nasm -f bin -o hello_world hello_world.asm
[bits 64]

; Virtual address at which we request our ELF to be mapped into memory. This is
; somewhat arbitrary, but we don't want it to be 0, and it's probably good to
; keep it page-aligned.
file_load_va: equ 4096 * 40

; ELF header

; Signature
db 0x7f, 'E', 'L', 'F'
; "Class" = 2, 64-bit
db 2
; Endianness = 1, little
db 1
; ELF version = 1
db 1
; OS ABI, unused, should be 0
db 0
; Extended ABI byte + 7 bytes padding. Leave as 0, it's ignored
dq 0
; ELF file type. 2 = executable
dw 2
; Target architecture. 0x3e = x86_64
dw 0x3e
; Additional ELF version stuff. Leave as 1.
dd 1
; Entry point address.
dq entry_point + file_load_va
; Program header offset. We'll put it immediately after the ELF header.
dq program_headers_start
; Section header offset. We'll put it after the program headers.
dq section_headers_start
; More flags. Not used, as far as I know.
dd 0
; Size of this header, 64 bytes.
dw 64
; Size of a program header entry.
dw 0x38
; Number of program header entries.
dw 1
; Size of a section header entry.
dw 0x40
; Number of section header entries
dw 3
; Index of the section containing the string table with section names
dw 2


program_headers_start:
; First field: The program header type. 1 = loadable segment.
dd 1
; Program header flags. 5 = Not writable. (bits 0, 1, and 2 = executable,
; writable, and readable, respectively)
dd 5
; The offset of the loadable segment in the file. This will contain the entire
; file, so make it 0.
dq 0
; The VA to place the segment at.
dq file_load_va
; The "phyiscal address". Don't think it's used, set to same as VA.
dq file_load_va
; The size of the segment in the file. It ends at the string table.
dq string_table
; The size of the segment in memory.
dq string_table
; The alignment of the segment
dq 0x200000


; We're just going to have two sections: .text, and .shstrtab. However, the
; first section header must be the NULL section header.
section_headers_start:
; The section header at index 0 is the null section header, filled with zero.
times 0x40 db 0

; The offset of the name ".text" in the string table
dd text_section_name - string_table
; The type is a loadable "bits" section
dd 1
; The flags for the section. Bits 0, 1, and 2 mean "writable", "allocated", and
; "executable", respectively.
dq 6
; The "virtual address" of the section
dq file_load_va
; The offset in the file
dq 0
; The size of the section
dq file_end
; Linked section index.  Keep as 0.
dd 0
; Section "info". Keep as 0 (may need to do something with it?)
dd 0
; Alignment. Who cares.
dq 16
; Section entry size. 0.
dq 0

; Next, the string table section.
dd string_table_name - string_table
; String table section
dd 3
; Doesn't need to be loaded.
dq 0
; This section only contains the string table, not the whole file
dq file_load_va + string_table
dq string_table
dq string_table_end - string_table
dd 0
dd 0
dq 1
dq 0


; Now we're past all the program and section headers. The actual code goes here
entry_point:
  ; Syscall number 1: write
  mov rax, 1
  ; File descriptor number 1
  mov rdi, 1
  ; Buffer
  mov rsi, file_load_va + message
  ; Buffer length
  mov rdx, message_length
  syscall
  ; Syscall number 60: exit
  mov rax, 60
  ; exit code
  mov rdi, 0
  syscall

message: db `Hello, world!\n`, 0
message_length: equ $ - message

string_table:
; The first string in the table must be empty
db 0
text_section_name:
db ".text", 0
string_table_name:
db ".shstrtab", 0
string_table_end:

file_end:
