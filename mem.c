static inline void stosb(void *addr, int data, int cnt) {
  __asm volatile("cld; rep stosb"
                 : "=D"(addr), "=c"(cnt)
                 : "0"(addr), "1"(cnt), "a"(data)
                 : "memory", "cc");
}

static inline void stosl(void *addr, int data, int cnt) {
  __asm volatile("cld; rep stosl"
                 : "=D"(addr), "=c"(cnt)
                 : "0"(addr), "1"(cnt), "a"(data)
                 : "memory", "cc");
}

void *mset(void *dst, int c, unsigned int n) {
  if ((long)dst % 4 == 0 && n % 4 == 0) {
    c &= 0xFF;
    stosl(dst, (c << 24) | (c << 16) | (c << 8) | c, n / 4);
  } else
    stosb(dst, c, n);
  return dst;
}

void *miset(void *dst, int c, unsigned int n) {
  if ((long)dst % 4 == 0 && n % 4 == 0) {
    stosl(dst, c, n / 4);
  }
  return dst;
}

void *mmove(void *dst, const void *src, unsigned int n) {
  const char *s;
  char *d;

  s = src;
  d = dst;
  if (s < d && s + n > d) {
    s += n;
    d += n;
    while (n-- > 0)
      *--d = *--s;
  } else
    while (n-- > 0)
      *d++ = *s++;

  return dst;
}
