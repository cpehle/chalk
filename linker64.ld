OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH("i386:x86-64")
ENTRY(start)

mboot_load_address = 1M;

SECTIONS
{
    . = 1M;
    .init32 : {
        start64.o(.init32)
        *(.init32 .init32.*)
    }

    .text : {
        *(.text .rela.text .stub .text.* .gnu.linkonce.t.*)

    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
        bootstrap_stack_end = .;
    }


    /DISCARD/ : {
        *(.eh_frame .rela.eh_frame .note.GNU-stack)
    }
}
