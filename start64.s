    .code32
    .section .bootstrap_stack, "aw", @nobits
stack_bottom:
    .skip 16384 # 16 KiB
stack_top:


    .section .init32
    .global start
    .type start, @function
start:
    movl $stack_top, %esp
    pushl %ebx
    pushl %eax
    call entry32
    ljmp $0x08:entry64
hang:
    jmp hang


    .align 16
    .code64
    .global _start:
_start:
    cli
    hlt
hang64:
    hlt
    jmp hang64
