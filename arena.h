typedef struct Arena Arena;
typedef struct Arena
{
        u64 size;
        u8 *base;
        u64 used;
        s32 temp_count;
} Arena;

typedef struct TemporaryMemory TemporaryMemory;

// TODO(casey): Optional "clear" parameter!!!!
#define arenapushstruct(Arena, type) (type *)arenapushsize_(Arena, sizeof(type), 16)
#define arenapusharray(Arena, Count, type) (type *)arenapushsize_(Arena, (Count)*sizeof(type), 16)
#define arenapushsize(Arena, size, alignment) arenapushsize_(Arena, size, alignment)
void* arenapushsize_(Arena *a, size_t sizeInit, size_t alignment);

void arenainit(Arena *a, size_t size, void *base);
TemporaryMemory begintemporarymemory(Arena *a);
void endtemporarymemory(TemporaryMemory TempMem);
void subarena(Arena *res, Arena *a, size_t size, size_t alignment);
