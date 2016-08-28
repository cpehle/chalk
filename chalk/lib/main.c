#include "u.h"
#include "io.h"
#include "dat.h"
#include "mem.h"
#include "console.h"
#include "cpu.h"
#include "pci.h"
#include "arena.h"
#include "ahci.h"
#include "e1000.h"
#include "vesa.h"
#include "vec.h"
#include "acpi.h"
#include "nvme.h"
#include "graphics.h"
#include "assert.h"


int kernel_main() {
  ConsoleDesc cd = {0, 0xf0, (unsigned short *)0xb8000};
  Console c = &cd;
  cclear(c,0xff);

  {
    const u16 com1 = 0x3f8;
    outb(com1 + 1, 0x00);    // Disable all interrupts
    outb(com1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(com1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(com1 + 1, 0x00);    //                  (hi byte)
    outb(com1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(com1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(com1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    if (0) {
    for (;;) {
      if(inb(com1 + 5) & 1) {
        char ch = inb(com1);
        cputc(c, ch);
      }
    }
    }

  }

  pciscan(c); // This is to test pci traversal.
  // Ethernet
  if (0) {
    PciConf eth = pciconfread(0, 3);
    cprintpciconf(c, eth);
    e1000init(0, &eth, c);
  }
  // for (;;);

  // Instead of a proper memory allocator we just allocate permanently
  // from one large Arena, this is only a temporary solution.
  // Eventually we want to guarantee that processes and tasks get
  // different arenas with potential overlap in a single address
  // space.
  Arena* a;
  arenainit(a, 1000*1000*1000*sizeof(u8), (void *)0x100000);
  // Functions that need to allocate memory need to be passed a memory
  // arena as part of their execution context.

  // initialize all known hardware, so far there is no dynamic device
  // discovery, instead we hardcode the locations of the devices on
  // the pci bus

  // AcpiDesc acpi = {};
  // acpiinit(&acpi, c);

  // nvme
  if (0) {
    PciConf conf = pciconfread(0,3);
    cprintpciconf(c, conf);
    nvmepciinit(a, c, conf);
  }

  // Ahci code doesn't work yet.
  // ahcipciinit(a, c, 0, 4);

  // Eventually we want to enable all CPU features found during detection.
  cpudetect(c);

  {
    M44 m = (M44){1,1,0,0,
                  0,1,0,0,
                  0,0,1,0,
                  0,0,0,1};
    V4 v = {1,
            0,
            0,
            0};
    V4 w = {0,
            1,
            0,
            0};
    V4 u = v4avv(v, w);
    cprintv4(c, u);
    cprintv4(c, v4msv(2, u));
    cprintv4(c, v4mmv(m44i(), u));
    cprintm44(c, m44i());
    cprintm44(c, v4msm(2, m44i()));
    cprintm44(c, v4mmm(v4msm(2, m44i()), m44i()));
    cprintm44(c, v4mmm(m, m));
  }

  for (;;);
  // Graphics only work with bochs emulator
  {
    u32 *framebufferaddr;
    {
       // 0x1234:0x1111
      PciConf vesa = pciconfread(0, 2);
      framebufferaddr = (u32 *)vesa.dev.base_address_register[0].address;
    }
    vbeset(1920, 1200, 32);
    int offset = 0;
// clear the screen to light grey
    for (int i = 0; i < 1920; ++i) {
      for (int j = 0; j<1200; ++j) {
      offset = (j * 1920 + i);
      // Format is 0x00rrggbb, can use first byte for alpha blending / z-buffering
      framebufferaddr[offset] = 0x00eeeeee;
      }
    }
    // draw a "line"
    for (int i = 0; i < 1920; ++i) {
      offset = (50 * 1920 + i);
      // Format is 0x00rrggbb, can use first byte for alpha blending
      framebufferaddr[offset] = 0x00ff0000;
    }


    {
      float t = 0;
      BezierData b = {
          4,
          {{50, 68, 0, 0}, {70, 220, 0, 0}, {80, 40, 0, 0}, {90, 100, 0, 0}}};
      while (t < 1) {
        V4 v = evcubicbezier(b, t);
        int offset = (u32)(v.v[1]) * 1920 + (u32)(v.v[0]);
        framebufferaddr[offset] = 0x000000ff;
        t += 0.00001;
      }
    }
    {
      float t = 0;
      BezierData b = {
        4,
        {{10, 20, 0, 0}, {70, 10, 0, 0}, {80, 40, 0, 0}, {90, 100, 0, 0}}};
      while (t < 1) {
        V4 v = evcubicbezier(b, t);
        int offset = (u32)(v.v[1]) * 1920 + (u32)(v.v[0]);
        framebufferaddr[offset] = 0x0000ff00;
        t += 0.00001;
      }
    }
    for (;;);
    u32* buf = renderglyphs(a);
    copyrect(framebufferaddr, buf, 0, 20, 98);
    rendertext(framebufferaddr, buf, "Hello World.", 0, 50);
    rendertext(framebufferaddr, buf, "Hello World.", 0, 66);
    rendertext(framebufferaddr, buf, "This is a test.", 0, 96);
    rendertext(framebufferaddr, buf, "Hello World.", 8*20, 82);
    rendertext(framebufferaddr, buf, "12 23 45 332", 8*20, 60*16);
  }
  for (;;) {}
}
