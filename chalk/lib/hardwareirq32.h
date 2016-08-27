#define HARDWAREIRQ(n)
void hardwareirq##n();
__asm__(
        "   .section .text\n"
        "   .align 16"
        "   .global hardwareirq"#n"\n"
        "   .type hardwareirq"#n",@function"

        )
