#include "u.h"
#include "arena.h"


typedef struct TemporaryMemory
{
        Arena *arena;
        size_t used;
} TemporaryMemory;


inline void
arenainit(Arena *a, size_t size, void *base)
{
    a->size = size;
    a->base = (u8 *)base;
    a->used = 0;
    a->temp_count = 0;
}

inline size_t
arenagetalignementoffset(Arena *a, size_t alignment)
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
getarenasizeremaining(Arena *a, size_t alignment)
{
    size_t res = a->size - (a->used + arenagetalignementoffset(a, alignment));
    return(res);
}


inline void* arenapushsize_(Arena *a, size_t sizeInit, size_t alignment)
{
    size_t size = sizeInit;

    size_t AlignmentOffset = arenagetalignementoffset(a, alignment);
    size += AlignmentOffset;

    // Assert((a->used + size) <= a->size);
    void *res = a->base + a->used + AlignmentOffset;
    a->used += size;

    // Assert(size >= sizeInit);

    return(res);
}

inline TemporaryMemory
begintemporarymemory(Arena *a)
{
    TemporaryMemory r;

    r.arena = a;
    r.used = a->used;

    ++a->temp_count;

    return r;
}

inline void
endtemporarymemory(TemporaryMemory TempMem)
{
    Arena *Arena = TempMem.arena;
    // Assert(Arena->used >= TempMem.used);
    Arena->used = TempMem.used;
    // Assert(Arena->temp_count > 0);
    --Arena->temp_count;
}

inline void
checkarena(Arena *Arena)
{
    // Assert(Arena->temp_count == 0);
}

inline void
subarena(Arena *res, Arena *a, size_t size, size_t Alignment)
{
    res->size = size;
    res->base = (u8 *)arenapushsize_(a, size, Alignment);
    res->used = 0;
    res->temp_count = 0;
}
