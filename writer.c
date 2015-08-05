#include "u.h"

typedef u64 Font;
typedef u32 Color;

typedef struct Buffer {

} Buffer;

typedef struct {
        Buffer buffer;
        Font font;
        Color color;
        u16 verticaloffset;
} Writer;

void write(Writer w);
void writeln(Writer w);
void writeint(Writer w);
void writestring(Writer w);
void writehex(Writer w);
void writereal(Writer w);
void writedate(Writer w);
