# define assert(x)                              \
do { \
        if (!(x)) {\
        static ConsoleDesc cd = {0, 0xf0, (unsigned short *)0xb8000};   \
        Console c = &cd;\
        cprint(c, "Assertion "#x" failed in ");\
        cprint(c, __FILE__), cputc(c,':');        \
        cprintint(c, __LINE__, 10, 0), cnl(c);     \
        }\
} while(0)
