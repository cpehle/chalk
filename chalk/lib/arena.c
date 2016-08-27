/// This file implements a memory arena, it is currently the only way
/// to allocate memory


#include "u.h"
#include "arena.h"

typedef struct TemporaryMemory
{
        Arena *arena;
        size_t used;
} TemporaryMemory;

/// \brief Initializes a new arena of a certain size
inline void arenainit(Arena *a, size_t size, void *base)
{
    a->size = size;
    a->base = (u8 *)base;
    a->used = 0;
    a->temp_count = 0;
}

///
inline size_t arenagetalignementoffset(Arena *a, size_t alignment)
{
    size_t alignment_offset = 0, result_ptr = (size_t)a->base + a->used, alignment_mask = alignment - 1;
    if(result_ptr & alignment_mask)
    {
        alignment_offset = alignment - (result_ptr & alignment_mask);
    }
    return alignment_offset;
}

inline size_t getarenasizeremaining(Arena *a, size_t alignment)
{
    size_t res = a->size - (a->used + arenagetalignementoffset(a, alignment));
    return res;
}

inline void* arenapushsize_(Arena *a, size_t sizeinit, size_t alignment)
{
    size_t size = sizeinit, alignmentoffset = arenagetalignementoffset(a, alignment);
    size += alignmentoffset;
    void *res = a->base + a->used + alignmentoffset;
    a->used += size;
    return(res);
}

inline TemporaryMemory begintemporarymemory(Arena *a)
{
    TemporaryMemory r;
    r.arena = a;
    r.used = a->used;
    ++a->temp_count;
    return r;
}

inline void endtemporarymemory(TemporaryMemory temporary_memory)
{
    Arena *Arena = temporary_memory.arena;
    Arena->used = temporary_memory.used;
    --Arena->temp_count;
}

inline void checkarena(Arena *arena) {
}

inline void subarena(Arena *res, Arena *a, size_t size, size_t alignment)
{
    res->size = size;
    res->base = (u8 *)arenapushsize_(a, size, alignment);
    res->used = 0;
    res->temp_count = 0;
}
