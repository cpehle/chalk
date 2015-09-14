
    .code16
    .globl start

start:
    cli
     # Set up the important data segment registers (DS, ES, SS).
    xorw %ax,%ax   # Segment number zero
    movw %ax,%ds # -> Data Segment
    movw %ax,%es # -> Extra Segment
    movw %ax,%ss # -> Stack Segment

seta20.1: 	# This is to fix some silly compatibility stuff
    inb     $0x64,%al   # Wait for not busy
    testb   $0x2,%al
    jnz     seta20.1
    movb    $0xd1,%al # 0xd1 -> port 0x64
    outb    %al,$0x64

seta20.2:
    inb     $0x64,%al  # Wait for not busy
    testb   $0x2,%al
    jnz     seta20.2

    movb    $0xdf,%al  # 0xdf -> port 0x60
    outb    %al,$0x60

    lgdt    gdtdesc
    movl    %cr0, %eax
    orl     $0x1, %eax
    movl    %eax, %cr0

    ljmp    $(1<<3), $start32

.code32
start32:
  movw    $(2<<3), %ax    # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    $0, %ax                 # Zero segments not ready for use
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS

  movl    $start, %esp
  call    bootmain

  # If bootmain returns (it shouldn't), trigger a Bochs
  # breakpoint if running under Bochs, then loop.
  movw    $0x8a00, %ax
  movw    %ax, %dx
  outw    %ax, %dx
  movw    $0x8ae0, %ax
  outw    %ax, %dx
spin:
  jmp     spin

.p2align 2
gdt:
    .word 0,0
    .byte 0,0,0,0
    .word 0xffff, 0x0
    .byte 0x0, 0x9a, 0xcf, 0x0
    .word 0xffff, 0x0
    .byte 0x0, 0x92, 0xcf, 0x0

gdtdesc:
  .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
  .long   gdt
