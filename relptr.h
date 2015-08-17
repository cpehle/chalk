typedef struct {
        void * base;
        u32 offset;
} RelativePointer;

void relwrite(RelativePointer p, u32 val);

u32 relread(RelativePointer p);
