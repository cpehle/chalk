#include "u.h"
#include "io.h"
#include "mem.h"
#include "dat.h"
#include "console.h"

static char digits[] = "0123456789abcdef";

void cclear(Console c, short color) {
  for (int i = 0; i < 80 * 25; ++i) {
    c->buf[i] = color << 12;
  }
}

static void movecursor(Console c) {
  outb(0x3d4, 0x0e);
  outb(0x3d5, c->pos >> 8);
  outb(0x3d4, 0x0f);
  outb(0x3d5, c->pos);
  c->buf[c->pos + 1] = ' ' | (c->color << 8);
}

void cnl(Console cons) {
  cputc(cons, '\n');
}

void cputc(Console cons, int c) {
  if (c == '\n') {
    cons->pos += 80 - cons->pos % 80;
  } else {
    cons->buf[cons->pos++] = c | (cons->color << 8);
  }
  if ((cons->pos / 80) >= 24) {
    mmove(cons->buf, cons->buf + 80, sizeof(cons->buf[0]) * 23 * 80);
    cons->pos -= 80;
    miset(cons->buf + cons->pos, (cons->color << 24 | cons->color << 8),
          sizeof(cons->buf[0]) * (24 * 80 - cons->pos));
  }
  movecursor(cons);
}

void cprint(Console c, const char *str) {
  char ch;
  while ((ch = *str++)) {
    cputc(c, ch);
  }
}

void cprintpairs(Console c, Pair* p, int count) {
  for (int i = 0; i < count; ++i) {
    cprint(c, p[i].key), cprint(c, ": "), cprintint(c, p[i].value, 16, 0),  cprint(c, ", ");
  }
}

void cprintint(Console c, int xx, int base, int sign) {
  char buf[16];
  int i;
  unsigned int x;
  if (base > 16) {
    base = 16;
  }
  if (sign && (sign = xx < 0)) {
    x = -xx;
  } else {
    x = xx;
  }
  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);
  if (sign) {
    buf[i++] = '-';
  }
  while (--i >= 0) {
    cputc(c, buf[i]);
  }
}
