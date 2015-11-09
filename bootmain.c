// Boot loader
#include "u.h"
#include "io.h"
#include "elf.h"

#define SECTSIZE  512

struct mbheader {
  u32 magic;
  u32 flags;
  u32 checksum;
  u32 header_addr;
  u32 load_addr;
  u32 load_end_addr;
  u32 bss_end_addr;
  u32 entry_addr;
};

void readsegment(u8*, u32, u32);

void bootclrscr(u16 color) {
  u16* buf = (u16*)(0xb8000);
  for (int i = 0; i < 80 * 25; ++i) {
      buf[i] = color << 12;
  }
}


void
bootmain(void)
{
  outw(0x3D4,0x200A); // disable cursor
  #if 0
  ElfHeader* eh = (ElfHeader*)0x10000;
  ElfProgramHeader* ph, *eph;
  readsegment((u8*)eh, SECTSIZE*8, 0);
  if (eh->magic != elfmagic) {
    bootclrscr(0xee);
    for (;;);
  }
  bootclrscr(0x00);
  ph = (ElfProgramHeader*)((u8*)eh + eh->phoff);
  eph = ph + eh->phnum;
  for (; ph < eph; ph++) {
    readsegment((u8*)ph->paddr, ph->memsz, ph->offset);
  }
  bootclrscr(0xaa);
  ((void (*)(void)) ((u32)(eh->entry)))();
  bootclrscr(0xff);
  #else
  struct mbheader *hdr;
  void (*entry)(void);
  u32 *x;
  u32 n;
  x = (u32*) 0x10000; // scratch space
  outw(0x3D4,0x200A);

  readsegment((u8*)x, 8192, 0);
  for (n = 0; n < 8192/4; n++)
    if (x[n] == 0x1BADB002)
      if ((x[n] + x[n+1] + x[n+2]) == 0)
        goto found_it;
  // failure
  return;

found_it:
  //bootclrscr(0x00);
  hdr = (struct mbheader *) (x + n);

  if (!(hdr->flags & 0x10000))
    return; // does not have load_* fields, cannot proceed
  //bootclrscr(0xaa);
  if (hdr->load_addr > hdr->header_addr)
    return; // invalid;
  if (hdr->load_end_addr < hdr->load_addr)
    return; // no idea how much to load
  readsegment((u8*) hdr->load_addr,
    (hdr->load_end_addr - hdr->load_addr),
    (n * 4) - (hdr->header_addr - hdr->load_addr));

  if (hdr->bss_end_addr > hdr->load_end_addr)
    stosb((void*) hdr->load_end_addr, 0,
      hdr->bss_end_addr - hdr->load_end_addr);

  // Call the entry point from the multiboot header.
  // Does not return!
  entry = (void(*)(void))(hdr->entry_addr);
  entry();
  #endif
}

inline void
waitdisk(void)
{
  // Wait for disk ready.
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
void
readsect(void *dst, u32 offset)
{
  // Issue command.
  waitdisk();
  outb(0x1F2, 1);   // count = 1
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  // Read data.
  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void
readsegment(u8* pa, u32 count, u32 offset)
{
  u8* epa;

  epa = pa + count;

  // Round down to sector boundary.
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  offset = (offset / SECTSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
