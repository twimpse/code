.section .data

    name: .string "/bin/bash"

.section .text

.globl _start

_start:
    # third argument of execve is envp, set to NULL
    xor %rdx, %rdx 

    # push NULL to the stack, argv terminator
    pushq %rdx 

    # first argument to execve is the file name
    leaq name, %rdi

    # also argv[0]
    push %rdi

    # second argument to execve is argv
    mov %rsp, %rsi

    #copy 59 to rax, defining syscall number for execve  
    movq $59, %rax 
    syscall
