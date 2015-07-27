static inline u8
inb(u16 port)
{
  u8 data;

  __asm__ volatile("in %1,%0" : "=a" (data) : "d" (port));
  return data;
}

static inline u32
inl(u16 port)
{
  u32 data;

  __asm__ volatile("in %1,%0" : "=a" (data) : "d" (port));
  return data;
}

static inline u16
inw(u16 port)
{
  u16 data;

  __asm__ volatile("in %1,%0" : "=a" (data) : "d" (port));
  return data;
}

static inline void
insl(int port, void *addr, int cnt)
{
  __asm__ volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

static inline void
outb(u16 port, u8 data)
{
  __asm__ volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outl(u16 port, unsigned int data)
{
  __asm__ volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outw(u16 port, u16 data)
{
  __asm__ volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outsl(int port, const void *addr, int cnt)
{
  __asm__ volatile("cld; rep outsl" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}

static inline void
stosb(void *addr, int data, int cnt)
{
  __asm__ volatile("cld; rep stosb" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}

static inline void
stosl(void *addr, int data, int cnt)
{
  __asm__ volatile("cld; rep stosl" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}



static inline void mmiowrite8(void *p, u8 data)
{
    *(volatile u8 *)(p) = data;
}

static inline u8 mmioread8(void *p)
{
    return *(volatile u8 *)(p);
}

static inline void mmiowrite16(void *p, u16 data)
{
    *(volatile u16 *)(p) = data;
}

static inline u16 mmioread16(void *p)
{
    return *(volatile u16 *)(p);
}

static inline void mmiowrite32(void *p, u32 data)
{
    *(volatile u32 *)(p) = data;
}

static inline u32 mmioread32(void *p)
{
    return *(volatile u32 *)(p);
}

static inline void mmiowrite64(void *p, u64 data)
{
    *(volatile u64 *)(p) = data;
}

static inline u64 mmioread64(void *p)
{
    return *(volatile u64 *)(p);
}
