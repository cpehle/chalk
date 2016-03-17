typedef struct Arena Arena;
typedef struct Arena
{
        u64 size;
        u8 *base;
        u64 used;
        s32 temp_count;
} Arena;

typedef struct TemporaryMemory TemporaryMemory;
#define arenapushstructalign(arena, type, alignment) (type *)arenapushsize_(arena, sizeof(type), 16)
#define arenapusharrayalign(arena, count, type, alignment) (type *)arenapushsize_(arena, (count)*sizeof(type), alignment)

#define arenapushstruct(arena, type) (type *)arenapushsize_(arena, sizeof(type), 16)
#define arenapusharray(arena, count, type) (type *)arenapushsize_(arena, (count)*sizeof(type), 16)
#define arenapushsize(arena, size, alignment) arenapushsize_(arena, size, alignment)
void* arenapushsize_(Arena *a, size_t sizeInit, size_t alignment);

void arenainit(Arena *arena, size_t size, void *base);
TemporaryMemory begin_temporary_memory(Arena *a);
void end_temporary_memory(TemporaryMemory temporary_memory);
void subarena(Arena *res, Arena *arena, size_t size, size_t alignment);
