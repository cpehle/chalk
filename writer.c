typedef struct {
        Buffer buffer;
        Font font;
        Color color;
        u16 verticaloffset;
} Writer;


void write();
void writeln();
void writeint();
void writestring();
void writehex();
void writereal();
void writedate();
