/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */

ENTRY(mboot_entry)

mboot_load_address = 1M;

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
    /* Begin putting sections at 1 MiB, a conventional place for kernels to be
       loaded at by the bootloader. */
    . = 1M;
    PROVIDE(begin = .);
    /* First put the multiboot header, as it is required to be put very early
       early in the image or the bootloader won't recognize the file format.
       Next we'll put the .text section. */
    .text : AT(mboot_load_address) {
        *(.multiboot)
        *(.text .rela.text .stub .text.* .gnu.linkonce.t.*)
        . = ALIGN(1K);
        *(.init.init32) FILL(0x90909090);
        . = . + 1K;
        . = ALIGN(1K);
        *(.init.init64);
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

    /* The compiler may produce other sections, by default it will put them in
       a segment with the same name. Simply add stuff here as needed. */
}
mboot_load_end = mboot_load_address + (edata - begin);
mboot_bss_end = mboot_load_address + (end - begin);
mboot_entry_address = mboot_load_address + (mboot_entry - begin);
