typedef enum {
        InterruptGate = 0x8e00,
        TrapGate = 0x8f00
} InterruptType;

void idtinit();
void idtsethandler(u8 index, InterruptType t, void (*handler));
