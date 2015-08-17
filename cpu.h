struct SegRegionDesc {
        u16 rd_limit;
        u64 rd_base;
} __packed;
typedef struct SegRegionDesc* SegRegion;

void	lgdt(SegRegion r) {
        __asm __volatile ("lgdt %0\n" : : "m"(r));
}


static __inline__ u64 rdtsc(void)
{
        u64 x;
        __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
        return x;
}
