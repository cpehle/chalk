# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
    .set MEMINFO,  1<<1             # provide memory map
    .set ADDR, 0x10000
.set FLAGS,    ADDR | ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard. We put this into a special
.section .multiboot
    .align 4
multibootheader:
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
.global _start
.type _start, @function
_start:
    movl $stack_top, %esp

    pushl %ebx
    pushl %eax
    call main

    cli
    hlt
.Lhang:
    jmp .Lhang
.size _start, . - _start
