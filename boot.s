    .code16
    .globl start
start:
    # disable interrupts
    cli
    # Set data segment registers DS,ES and SS to zero.
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    #
seta20.1:
    inb     $0x64,%al               # Wait for not busy
    testb   $0x2,%al
    jnz     seta20.1

    movb    $0xd1,%al               # 0xd1 -> port 0x64
    outb    %al,$0x64

seta20.2:
    inb     $0x64,%al               # Wait for not busy
    testb   $0x2,%al
    jnz     seta20.2

    movb    $0xdf,%al               # 0xdf -> port 0x60
    outb    %al,$0x60

    lgdt gdtdesc
    movl %cr0, %eax
    orl , %eax
    movl %eax, %cr0

    ljmp , $start32

    movl $start, %esp
    call bootmain


gdt:


gdtdesc:
    .word (gdtdesc - gdt - 1)
    .long  gdt
