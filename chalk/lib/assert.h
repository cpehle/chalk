# define assert(x)                              \
do { \
        if (!(x)) {\
                static ConsoleDesc cd = {0, 0xf0, (unsigned short *)0xb8000}; \
                Console c = &cd;                                        \
                cprint(c, __FILE__), cputc(c,':');                      \
                cprintint(c, __LINE__, 10, 0),                          \
                cprint(c, ":0: assertion "#x" failed"), cnl(c);    \
        }\
} while(0)
