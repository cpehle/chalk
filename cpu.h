struct SegRegionDesc {
        u16 rd_limit;
        u64 rd_base;
} __packed;
typedef struct SegRegionDesc* SegRegion;

void	lgdt(SegRegion r) {
        __asm __volatile ("lgdt %0\n" : : "m"(r));
}
