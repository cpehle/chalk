typedef struct Arena Arena;
typedef struct Arena
{
        u64 size;
        u8 *base;
        u64 used;
        s32 temp_count;
} Arena;

// TODO(casey): Optional "clear" parameter!!!!
#define arena_push_struct(Arena, type) (type *)arena_push_size_(Arena, sizeof(type))
#define arena_push_array(Arena, Count, type) (type *)arena_push_size_(Arena, (Count)*sizeof(type))
#define arena_push_size(Arena, size) arena_push_size_(Arena, size)
inline void* arena_push_size_(Arena *a, unsigned int sizeInit);
