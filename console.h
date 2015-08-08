typedef enum {
        Black,
        Blue,
        Green,
        Cyan,
        Red,
        Magenta,
        Brown,
        Grey,
        Bright = 0x08,
        Blinking = 0x80,
        Yellow = Bright|Brown,
        White  = Bright|Grey
} Color;
void cclear(Console c, short color);
void cputc(Console cons, int c);
void cnl(Console cons);
void cprint(Console c, const char * str);
void cprintint(Console c, int xx, int base, int sign);
