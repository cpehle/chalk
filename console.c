#include "u.h"
#include "mem.h"
#include "dat.h"

static char digits[] = "0123456789abcdef";
void cclear(Console c, short color) {
  for (int i = 0; i < 80 * 25; ++i) {
    c->buf[i] = color << 12;
  }
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
    mset(cons->buf + cons->pos, 0,
           sizeof(cons->buf[0]) * (24 * 80 - cons->pos));
  }
}

void cprint(Console c, char * str) {
  char ch;
  while((ch = *str++)) {
    cputc(c, ch);
  }
}

void cprintint(Console c, int xx, int base, int sign)
{
  char buf[16];
  int i;
  unsigned int x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    cputc(c, buf[i]);
}
