#include "u.h"
#include "arena.h"

typedef u32 size_t;

typedef struct TemporaryMemory
{
        Arena *arena;
        size_t used;
} TemporaryMemory;


inline void
arena_init(Arena *a, size_t size, void *base)
{
    a->size = size;
    a->base = (u8 *)base;
    a->used = 0;
    a->temp_count = 0;
}

inline size_t
arena_get_alignment_offset(Arena *a, size_t alignment)
{
    size_t alignment_offset = 0;

    size_t result_ptr = (size_t)a->base + a->used;
    size_t alignment_mask = alignment - 1;
    if(result_ptr & alignment_mask)
    {
        alignment_offset = alignment - (result_ptr & alignment_mask);
    }

    return alignment_offset;
}

inline size_t
GetArenasizeRemaining(Arena *a, size_t Alignment = 4)
{
    size_t res = a->size - (a->used + arena_get_alignment_offset(a, Alignment));

    return(res);
}

inline void *
arena_push_size_(Arena *a, size_t sizeInit, size_t Alignment = 4)
{
    size_t size = sizeInit;

    size_t AlignmentOffset = arena_get_alignment_offset(a, Alignment);
    size += AlignmentOffset;

    Assert((a->used + size) <= a->size);
    void *res = a->base + a->used + AlignmentOffset;
    a->used += size;

    Assert(size >= sizeInit);

    return(res);
}

inline TemporaryMemory
BeginTemporaryMemory(Arena *a)
{
    TemporaryMemory r;

    r.arena = a;
    r.used = a->used;

    ++a->temp_count;

    return(res);
}

inline void
EndTemporaryMemory(TemporaryMemory TempMem)
{
    Arena *Arena = TempMem.Arena;
    Assert(Arena->used >= TempMem.used);
    Arena->used = TempMem.used;
    Assert(Arena->temp_count > 0);
    --Arena->temp_count;
}

inline void
CheckArena(Arena *Arena)
{
    Assert(Arena->temp_count == 0);
}

inline void
SubArena(Arena *res, Arena *Arena, size_t size, size_t Alignment = 16)
{
    res->size = size;
    res->base = (uint8 *)Pushsize_(Arena, size, Alignment);
    res->used = 0;
    res->temp_count = 0;
}
