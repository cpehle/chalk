# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set ADDR, 0x10000
.set FLAGS,    ADDR | ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

    # Declare a header as in the Multiboot Standard. We put this into a speci

    .code32
    .global mboot_header
    .global mboot_entry
.section .multiboot
    .align 4
mboot_header:
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long mboot_load_address
    .long mboot_load_address
    .long mboot_load_end
    .long mboot_bss_end
    .long mboot_entry_address

.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.type mboot_entry, @function
mboot_entry:
    movl $stack_top, %esp

    pushl %ebx
    pushl %eax
    call kernelmain
    jmp $0x0008,$entry64
hang:
    jmp hang



.align 16
    .code64
    .global entry64
entry64:
    cli
    hlt
hang64:
    jmp hang64

.size mboot_entry, . - mboot_entry
