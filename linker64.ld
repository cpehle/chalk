OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH("i386:x86-64")
ENTRY(mboot_entry)

mboot_load_address = 1M;
/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */


/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
    . = 1M;
     PROVIDE(begin = .);
    .text : AT(mboot_load_address) {
        *(.multiboot)
        *(.text .rela.text .stub .text.* .gnu.linkonce.t.*)

    }
    PROVIDE(etext = .);
    /* Read-only data. */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }
    PROVIDE(data = .);

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }
    PROVIDE(edata = .);

    /* Read-write data (uninitialized) and stack */
    bss_start = .;
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
        bootstrap_stack_end = .;
    }
    PROVIDE(end = .);
    /DISCARD/ : {
        *(.eh_frame .rela.eh_frame .note.GNU-stack)
    }
}
mboot_load_end = mboot_load_address + (edata - begin);
mboot_bss_end = mboot_load_address + (end - begin);
mboot_entry_address = mboot_load_address + (mboot_entry - begin);
