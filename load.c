#include "u.h"
#include "io.h"
#include "dat.h"
#include "mem.h"
#include "console.h"
#include "cpu.h"
#include "detect.h"
#include "cpufunc.h"
#include "pci.h"
#include "arena.h"
#include "ahci.h"
#include "idt.h"
#include "e1000.h"
#include "vesa.h"
#include "vec.h"
#include "acpi.h"
#include "graphics.h"

#define X86_MSR_EFER                0xC0000080
#define X86_MSR_EFER_LMA			(1 << 10)

#define X86_CR0_PE	(1 <<  0)	/* enable protected mode	*/
#define X86_CR0_EM	(1 <<  2)	/* disable fpu			*/
#define X86_CR0_TS	(1 <<  3)	/* task switched		*/
#define X86_CR0_WP	(1 << 16)	/* force write protection on user read only pages for kernel	*/
#define X86_CR0_NW	(1 << 29)	/*				*/
#define X86_CR0_CD	(1 << 30)	/*				*/
#define X86_CR0_PG	(1 << 31)	/* enable paging		*/
#define X86_CR4_PVI	(1 <<  1)	/* enable protected mode */
#define X86_CR4_PAE (1 <<  5)  // enable physical address extension

#define INIT32_CS		0x08

typedef struct GDTEntry {
  u32 first;
  u32 second;
} GDTEntry;

typedef u64 Elf64_Addr;
typedef u64 Elf64_Off;

#define EI_NIDENT 16

typedef struct {
  unsigned char e_ident[EI_NIDENT];
  u16 e_type;
  u16 e_machine;
  u32 e_version;
  Elf64_Addr e_entry;
  Elf64_Off e_phoff;
  Elf64_Off e_shoff;
  u32 e_flags;
  u16 e_ehsize;
  u16 e_phentsize;
  u16 e_phnum;
  u16 e_shentsize;
  u16 e_shnum;
  u16 e_shstrndx;
} Elf64_Ehdr;

typedef struct {
  u32 p_type;
  u32 p_flags;
  Elf64_Off p_offset;
  Elf64_Addr p_vaddr;
  Elf64_Addr p_paddr;
  u64 p_filesz;
  u64 p_memsz;
  u64 p_align;
} Elf64_Phdr;

typedef struct {
  u32 sh_name;
  u32 sh_type;
  u64 sh_flags;
  Elf64_Addr sh_addr;
  Elf64_Off  sh_offset;
  u64 sh_size;
  u32 sh_link;
  u32 sh_info;
  u64 sh_addralign;
  u64 sh_entsize;
} Elf64_Shdr;

typedef struct {
    u32 flags;			//required
    u32 memLower;		//if bit 0 in flags are set
    u32 memUpper;		//if bit 0 in flags are set
    u32 bootDevice;		//if bit 1 in flags are set
    u32 commandLine;		//if bit 2 in flags are set
    u32 moduleCount;		//if bit 3 in flags are set
    u32 moduleAddress;		//if bit 3 in flags are set
    u32 syms[4];		//if bits 4 or 5 in flags are set
    u32 memMapLength;		//if bit 6 in flags is set
    u32 memMapAddress;		//if bit 6 in flags is set
    u32 drivesLength;		//if bit 7 in flags is set
    u32 drivesAddress;		//if bit 7 in flags is set
    u32 configTable;		//if bit 8 in flags is set
    u32 apmTable;		//if bit 9 in flags is set
    u32 vbeControlInfo;	//if bit 10 in flags is set
    u32 vbeModeInfo;		//if bit 11 in flags is set
    u32 vbeMode;		// all vbe_* set if bit 12 in flags are set
    u32 vbeInterfaceSeg;
    u32 vbeInterfaceOff;
    u32 vbeInterfaceLength;
} __attribute__((packed)) Multibootinfo;

typedef struct {
  u32 modstart;
  u32 modend;
  u32 string;
  u32 _r0;
} __attribute__((packed)) Multibootmodule;

static char* bootmsg = "Chalk.\n";

void disablepaging() {
  u32 val = X86_CR0_PG;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr0, %0   \n"
                       "and  %1, %0      \n"
                       "mov  %0, %%cr0   \n"
                       : "=r"(tmp)
                       : "ri"(~val));
}

void enablepaging() {
  u32 val = X86_CR0_PG | X86_CR0_WP | X86_CR0_PE;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr0, %0   \n"
                       "or   %1, %0      \n"
                       "mov  %0, %%cr0   \n"
                       : "=r"(tmp)
                       : "ri"(val));
}

void enablepaemode() {
  u32 val = X86_CR4_PAE;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr4, %0   \n"
                       "or   %1, %0      \n"
                       "mov  %0, %%cr4   \n"
                       : "=r"(tmp)
                       : "ri"(val));
}

void enablelongmode() {
  u32 efer = rdmsr(0xc0000080);
  efer |= (1 << 8);
  wrmsr(0xc0000080, efer);
}

u32 getactivepagetable(void) {
   u32 pgm;
    __asm__ __volatile__ ("mov %%cr3, %0\n" :"=a" (pgm));
    return pgm;
}

void setactivepagetable(u32 root) {
   __asm__ __volatile__ ("mov %0, %%cr3 \n"
                        :
                        : "r"(root));
}

int longmodeactive() {
    u32 efer = rdmsr(X86_MSR_EFER);
    return (efer & X86_MSR_EFER_LMA);
}

extern char* bootstrap_stack_end;
extern u8* entry64;

void main(u32 magic, u32 mbootinfoaddr) {
  ConsoleDesc cd = {0, 0xf0, (unsigned short *)0xb8000};
  Console c = &cd;
  cclear(c, 0xff);
  cprint(c, bootmsg);

#if 0
  if (magic != 0x2BADB002) {
    cprint(c, "boot: invalid boot magic.\n");
    for (;;);
  }

  Multibootinfo *const mbi = (Multibootinfo *) mbootinfoaddr;
  if (!(mbi->flags & (1 << 3))) {
    for (;;) {}
  }

  Multibootmodule *const mbm = (Multibootmodule *)mbi->moduleAddress;
#endif

  // clear the memory from 0x1000 to 0x7000 we will store the
  // bootstrap page tables here. We will map the first 4 gigabytes,
  // this is rather careless if there isn't actually that much memory
  // in the machine.
  // mset((void *)0x1000, 0, 6000);
  u64 *p4ml = (u64 *)(0x1000);
  u64 *pdpt = (u64 *)(0x2000);
  u64 *pd[4] = {(u64 *)(0x3000), (u64 *)0x4000, (u64 *)0x5000, (u64 *)0x6000};
  p4ml[0] = 0x2000 | 3;
  pdpt[0] = 0x3000 | 3;
  pdpt[1] = 0x4000 | 3;
  pdpt[2] = 0x5000 | 3;
  pdpt[3] = 0x6000 | 3;
  u64 pde = 0x83;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 512; ++j) {
      pd[i][j] = pde;
      pde += 0x200000;
    }
  }


#if 0
  {
    Elf64_Ehdr *hdr = 0;
    Elf64_Phdr *phdr = 0;
    Elf64_Shdr *shdr = 0;
    u64 srcstart = 0;
    u64 srcend = 0;
    u64 dststart = 0;
    u64 dstend = 0;
    hdr  = (Elf64_Ehdr *)(mbm->modstart);
    if (hdr->e_ident[0] != 0x7f || hdr->e_ident[1] != 'E' ||
        hdr->e_ident[2] != 'L' || hdr->e_ident[3] != 'F') {
      cprint(c, "Invalid ELF Magic!\n");
    }

    phdr = (Elf64_Phdr *)((u8 *)hdr + hdr->e_phoff);
    for (int i = 0; i < hdr->e_phnum; ++i) {
      if (phdr[i].p_type != 1) {
        continue;
      }
      srcstart = mbm->modstart + phdr[i].p_offset;
      srcend = srcstart + phdr[i].p_memsz;
      dststart = phdr[i].p_paddr;
      dstend = dststart + phdr[i].p_memsz;

      mmove(dststart, srcstart, phdr[i].p_filesz);
      mset(dststart + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);

      cprintint(c, phdr[i].p_paddr, 16, 0), cputc(c,'\n');
      cprintint(c, phdr[i].p_align, 16, 0), cputc(c,'\n');
      cprintint(c, phdr[i].p_filesz, 16, 0), cputc(c,'\n');
      cprintint(c, phdr[i].p_offset, 16, 0), cputc(c,'\n');
      cprintint(c, phdr[i].p_memsz, 16, 0), cputc(c,'\n');
      cprintint(c, phdr[i].p_type, 16, 0), cputc(c,'\n');
      cprintint(c, phdr[i].p_vaddr, 16, 0), cputc(c,'\n');

      // readseg(pa, phdr->p_filesz, phdr->p_offset);
    }

    for (int i = 0; i < 4; ++i) {
      cputc(c, hdr->e_ident[i]);
    }
    cputc(c,'\n');
    if (!(hdr->e_machine == 0x3E)) {
      cprint(c, "elf: expected x86_64-ELF.\n");
      return 0;
    }
    if (hdr->e_type == 2) {
      cprint(c, "elf: found executable.\n");
    }
  }
#endif



  // This sequence to enable long mode is documented in the AMD Manual
  // for example.
  disablepaging();
  enablepaemode();
  enablelongmode();
  setactivepagetable(0x1000);
  enablepaging();
  if (!longmodeactive()) {
    cprint(c, "Failed to enable long mode.");
    for(;;);
  }
  GDTEntry entries[3] = {{0, 0},                       // Null descriptor
                         {0x00000000, 0x00209800},   // Code, R/X, Nonconforming
                         {0x00000000, 0x00009000}};  // Data, R/W, Expand Down
  mmove((void *)0, entries, sizeof(entries));
  struct SegRegionDesc r = {0x00000, 3};
  lgdt(&r);


  u8 *startup64;
  __asm__(
      "	call 1f			\n\t" /* retrieve ip of next instruction */
      "1:	popl %0 		\n\t" /* save in addr  */
      : "=r"(startup64));
  load(c);
}



void load(Console c) {
  #if 1

  {
    outb(0x21, 0xff); // Mask interrupts on PIC1 and PIC2
    outb(0xa1, 0xff);
  }



  pciscan(c); // This is to test pci traversal.

  // Ethernet
  {
    PciConf eth = pciconfread(0, 3);
    e1000init(0, &eth, c);
  }

  // Instead of a proper memory allocator we just allocate permanently
  // from one large Arena.
  Arena* a;
  arenainit(a, 4000000, (void *)0x1000000);
  int *x = arenapusharray(a, 1000, int);
  int *y = arenapusharray(a, 1000, int);
  for (int i = 0; i < 1000; ++i) {
    x[i] = y[i] = 1;
  }
  for (int i = 0; i < 1000; ++i) {
    x[i] += y[i];
  }
  cprintint(c, x[400], 16, 0), cputc(c, '\n');

  AcpiDesc acpi = {};
  acpiinit(&acpi, c);

  // Ahci code doesn't really work yet.
  ahcipciinit(0, c, 0, 4);
  // Eventually we want to enable all CPU features found by the scan.
  cpudetect(c);

  u64 t = rdtsc();
  // random vector code, need to write proper tests.
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
  cprintint(c, rdtsc()-t, 16, 0);



  for (;;) {}
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
    for (int i = 0; i < 1920; ++i) {
      for (int j = 0; j<1200; ++j) {
      offset = (j * 1920 + i);
      // Format is 0x00rrggbb, can use first byte for alpha blending / z-buffering
      framebufferaddr[offset] = 0x00eeeeee;
      }
    }
    for (int i = 0; i < 1000000; i += 1920 * 4) {
      copy(framebufferaddr, (u32*)i, 0, 0, 1920, 1200);
    }

    // draw a "line"
    for (int i = 0; i < 1920; ++i) {
      offset = (50 * 1920 + i);
      // Format is 0x00rrggbb, can use first byte for alpha blending
      framebufferaddr[offset] = 0x00ff0000;
    }

    float t = 0;
    BezierData b = {4,{{50,68,0,0},{70,220,0,0},{80,40,0,0},{90,100,0,0}}};

    while (t < 1) {
      V4 v = evcubicbezier(b, t);
      int offset = (u32)(v.v[1]) * 1920 + (u32)(v.v[0]);
      framebufferaddr[offset] = 0x000000ff;
      t += 0.00001;
    }
    u32* buf = renderglyphs(a);
    copyrect(framebufferaddr, buf, 0, 20, 98);
    rendertext(framebufferaddr, buf, "Hello World.", 0, 50);
    rendertext(framebufferaddr, buf, "Hello World.", 0, 66);
    rendertext(framebufferaddr, buf, "This is a test.", 0, 96);
    rendertext(framebufferaddr, buf, "Hello World.", 8*20, 82);
    rendertext(framebufferaddr, buf, "12 23 45 332", 8*20, 60*16);

  }
  for (;;) {}
  #endif
}
