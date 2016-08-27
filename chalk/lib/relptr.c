#include "u.h"
#include "relptr.h"

void relwrite(RelativePointer p, u32 val) {
        *((volatile u32 *)(p.base) + p.offset) = val;
}

u32 relread(RelativePointer p) {
        return *((volatile u32 *)(p.base) + p.offset);
}
