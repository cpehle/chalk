#include "u.h"
#include "mem.h"
#include "cpu.h"
#include "cpufunc.h"
#include "dat.h"


typedef struct GDTEntry {
  u32 first;
  u32 second;
} GDTEntry;

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

inline static void __attribute__((section("init32"))) disablepaging() {
  u32 val = X86_CR0_PG;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr0, %0   \n"
                       "and  %1, %0      \n"
                       "mov  %0, %%cr0   \n"
                       : "=r"(tmp)
                       : "ri"(~val));
}

inline static void  __attribute__((section("init32"))) enablepaging() {
  u32 val = X86_CR0_PG | X86_CR0_WP | X86_CR0_PE;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr0, %0   \n"
                       "or   %1, %0      \n"
                       "mov  %0, %%cr0   \n"
                       : "=r"(tmp)
                       : "ri"(val));
}

inline static void __attribute__((section("init32"))) enablepaemode() {
  u32 val = X86_CR4_PAE;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr4, %0   \n"
                       "or   %1, %0      \n"
                       "mov  %0, %%cr4   \n"
                       : "=r"(tmp)
                       : "ri"(val));
}

inline static void  __attribute__((section("init32"))) enablelongmode() {
  u32 efer = rdmsr(0xc0000080);
  efer |= (1 << 8);
  wrmsr(0xc0000080, efer);
}

inline static u32 __attribute__((section("init32"))) getactivepagetable(void) {
   u32 pgm;
    __asm__ __volatile__ ("mov %%cr3, %0\n" :"=a" (pgm));
    return pgm;
}

inline static void __attribute__((section("init32"))) setactivepagetable(u32 root) {
   __asm__ __volatile__ ("mov %0, %%cr3 \n"
                        :
                        : "r"(root));
}

inline static int  __attribute__((section("init32"))) longmodeactive() {
    u32 efer = rdmsr(X86_MSR_EFER);
    return (efer & X86_MSR_EFER_LMA);
}

void __attribute__((section("init32"))) entry32() {
  // clear the memory from 0x1000 to 0x7000 we will store the
  // bootstrap page tables here. We will map the first 4 gigabytes,
  // this is rather careless if there isn't actually that much memory
  // in the machine.
  mset((void *)0x1000, 0, 6000);
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

  // that doesn't really work though...
  // We should check support for long mode via cpu id here!
  // This sequence to enable long mode is documented in the AMD Manual for example.
  disablepaging();
  enablepaemode();
  enablelongmode();
  setactivepagetable(0x1000);
  enablepaging();
  if (!longmodeactive()) {
          for(;;);
  }
  GDTEntry entries[3] = {{0, 0},                    // Null descriptor
                         {0x00000000, 0x00209800},  // Code, R/X, Nonconforming
                         {0x00000000, 0x00009000}}; // Data, R/W, Expand Down
  mmove((void *)0, entries, sizeof(entries));
  struct SegRegionDesc r = {0x00000, 3};
  lgdt(&r);
  return;
}
