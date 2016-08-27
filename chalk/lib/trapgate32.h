
typedef union {
    struct {
        u32 reason;
        u32 es;
        u32 ds;
        u32 edi;
        u32 esi;
        u32 ebp;
        u32 __esp;
        u32 ebx;
        u32 edx;
        u32 ecx;
        u32 eax;
        u32 error;
        u32 eip;
        u32 cs;
        u32 eflags;
        u32 esp;
        u32 ss;
    };
    u32 regs[17];
} x86Exceptionframe;


// X86_EXCWITH_ERRORCODE: allows C implementation of
// exception handlers and trap/interrupt gates with error
// code.
// Usage: X86_EXCWITH_ERRORCODE(exc_gp)
#define X86_EXCWITH_ERRORCODE(name, reason)			\
void name (void);					\
static void name##handler(x86_exceptionframe_t * frame);	\
void name##_wrapper()						\
{								\
    u32 *handler = (u32*)name##handler;       \
    __asm__ (							\
    ".global "#name "		\n"			\
    "\t.type "#name",@function	\n"			\
    #name":				\n"			\
    "pusha				\n"			\
    "push	%%ds			\n"			\
    "push	%%es			\n"			\
    "push	%1			\n"			\
    "push	%%esp			\n"			\
    "call	%0			\n"			\
    set_dbgctl_lbr ()					\
    "addl	$8, %%esp		\n"			\
    "popl	%%es			\n"			\
    "popl	%%ds			\n"			\
    "popa				\n"			\
    "addl	$4, %%esp		\n"			\
    "iret				\n"			\
    :							\
    : "m"(*handler), "i"(reason)                            \
    );							\
}								\
static void name##handler(x86Exceptionframe * frame)



#define X86_EXCNO_ERRORCODE(name, reason)			\
void name (void);					\
static void name##handler(x86_exceptionframe_t * frame);	\
void name##_wrapper()						\
{								\
    u32 *handler = (u32*)name##handler;                     \
    __asm__ (							\
        ".global "#name "		\n"			\
    "\t.type "#name",@function	\n"			\
    #name":				\n"			\
    kdb_check_stack()					\
    "subl	$4, %%esp		\n"			\
    "pusha				\n"			\
    "push	%%ds			\n"			\
    "push	%%es			\n"			\
    "push	%1			\n"			\
    "push	%%esp			\n"			\
    "call	%0			\n"			\
    "addl	$8, %%esp		\n"			\
    "popl	%%es			\n"			\
    "popl	%%ds			\n"			\
    "popa				\n"			\
    "addl	$4, %%esp		\n"			\
    "iret				\n"			\
    :							\
    : "m"(*handler), "i"(reason)                            \
    );							\
}								\
static void name##handler(x86Exceptionframe * frame)
