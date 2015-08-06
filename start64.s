    .global _start
    .code64
    .section .text
    .align 16
.type _start, @function
_start:

    xor %rax, %rax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov 0x10000, %rax
    mov %rax, %rsp

    jmp main
