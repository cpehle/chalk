#include "u.h"
#include "dat.h"
#include "mem.h"
#include "console.h"
#include "cpu.h"
#include "cpufunc.h"
#include "pci.h"

#define X86_MSR_EFER                		0xC0000080

#define X86_MSR_EFER_LMA			(1 << 10)

#define X86_CR0_PE	(1 <<  0)	/* enable protected mode	*/
#define X86_CR0_EM	(1 <<  2)	/* disable fpu			*/
#define X86_CR0_TS	(1 <<  3)	/* task switched		*/
#define X86_CR0_WP	(1 << 16)	/* force write protection on user
                       read only pages for kernel	*/
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



static char* bootmsg = "Chalk.\n";

void disable_paging() {
  u32 val = X86_CR0_PG;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr0, %0   \n"
                       "and  %1, %0      \n"
                       "mov  %0, %%cr0   \n"
                       : "=r"(tmp)
                       : "ri"(~val));
}

void enable_paging() {
  u32 val = X86_CR0_PG | X86_CR0_WP | X86_CR0_PE;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr0, %0   \n"
                       "or   %1, %0      \n"
                       "mov  %0, %%cr0   \n"
                       : "=r"(tmp)
                       : "ri"(val));
}

void enable_pae_mode() {
  u32 val = X86_CR4_PAE;
  u32 tmp;
  __asm__ __volatile__("mov  %%cr4, %0   \n"
                       "or   %1, %0      \n"
                       "mov  %0, %%cr4   \n"
                       : "=r"(tmp)
                       : "ri"(val));
}

void enable_long_mode() {
  u32 efer = rdmsr(0xc0000080);
  efer |= (1 << 8);
  wrmsr(0xc0000080, efer);
}

u32 get_active_pagetable(void) {
   u32 pgm;
    __asm__ __volatile__ ("mov %%cr3, %0\n" :"=a" (pgm));
    return pgm;
}

void set_active_pagetable(u32 root) {
   __asm__ __volatile__ ("mov %0, %%cr3 \n"
                        :
                        : "r"(root));
}

int long_mode_active() {
    u32 efer = rdmsr(X86_MSR_EFER);
    return (efer & X86_MSR_EFER_LMA);
}

extern char* bootstrap_stack_end;

int main() {
  ConsoleDesc cd = {0, 0xf0, (unsigned short *)0xb8000};
  Console c = &cd;
  cclear(c, 0xff);
  cprint(c, bootmsg);
  // clear the memory from 0x1000 to 0x4000
  // we will store the bootstrap page tables
  // here.
  mset((void *)0x1000, 0, 4000);
  u64 *p4ml = (u64 *)(0x1000);
  u64 *pdpt = (u64 *)(0x2000);
  u64 *pd = (u64 *)(0x3000);
  p4ml[0] = 0x2000 | 3;
  pdpt[0] = 0x3000 | 3;
  u64 pde = 0x83;
  for (int i = 0; i < 512; ++i) {
    pd[i] = pde;
    pde += 0x200000;
  }

  {
    // We arranged the linker to put the 64bit elf file at bootrstrap_stack_end
    // and managed to convince GRUB to load it for us....
    // whether or not that was such a great idea is another question.
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)(0x150000);
    if (ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
      cprint(c, "Invalid ELF Magic!\n");
      for (;;)
        ;
    }
    for (int i = 0; i < 4; ++i) {
      cputc(c, ehdr->e_ident[i]);
    }
    if (!(ehdr->e_machine == 0x3E)) {
      cprint(c, "Expected x86_64-ELF.\n");
    }
    cprint(c, "\nProgram header offset:"), cprintint(c, ehdr->e_phoff, 16, 0);
    cprint(c, "\nNumber of entries in program section "),
        cprintint(c, ehdr->e_phnum, 16, 0);
    Elf64_Phdr *phdr = (Elf64_Phdr *)((u8 *)ehdr + ehdr->e_phoff);
    cprint(c, "\nType of entry in Program Header: "),
        cprintint(c, phdr->p_type, 16, 0);
    cprint(c, "\nIt is at offset: "), cprintint(c, phdr->p_offset, 16, 0);
    cprint(c, "\nExpects to be loaded at virtual address: "),
        cprintint(c, phdr->p_vaddr, 16, 0);
    cprint(c, "\nExpects to be loaded at physical address: "),
        cprintint(c, phdr->p_paddr, 16, 0);
    cprint(c, "\nSize in file: "), cprintint(c, phdr->p_filesz, 16, 0);
    cprint(c, "\nSize in memory: "), cprintint(c, phdr->p_memsz, 16, 0);
    cprint(c, "\nSize of entries in program section "),
        cprintint(c, ehdr->e_phentsize, 16, 0);
    cputc(c, '\n');
    // entry is where this code expects to be loaded.
    u64 entry = ehdr->e_entry;
    // that doesn't really work though...
  }

  // We should check support for long mode via cpu id here
  // This sequence to enable long mode is documented in the AMD Manual for example.
  disable_paging();
  enable_pae_mode();
  enable_long_mode();
  set_active_pagetable(0x1000);
  enable_paging();
  if (!long_mode_active()) {
    cprint(c, "Failed to enable long mode.");
    for(;;);
  }
  cprint(c, "We have enabled long mode!\n");
  // Actually this is not quite true, in fact we need to
  // do a long jump at some point for it to actually work.
  // pci_scan(c);
  // need to find ahci in order to load kernel from disk.

  // We now need to figure out where the 64bit code is and jump to it.
  GDTEntry entries[3] = {{0, 0},                    // Null descriptor
                         {0x00000000, 0xffffffff},  // Code, R/X, Nonconforming
                         {0x00000000, 0xffffffff}}; // Data, R/W, Expand Down
  mmove((void *)0, entries, sizeof(entries));
  struct SegRegionDesc r = {0x00000, 3};
  lgdt(&r);
  u8* startup64;

  for(;;);
  __asm__  __volatile__ ("	mov  %0, %%ds		\n\t"
      "	pushw %1		\n\t" /* push segment selector, used by ljmp */
      "	lea %2, %%eax   	\n\t" /* load startup_system */
      "	pushl %%eax		\n\t" /* load startup_system */
      "     movl %3, %%edi          \n\t" /* pass AP info to startup_system */
      "	ljmp *(%%esp)		\n\t" /* jump to startup_system and load new CS
                                         */
      :                               /* No Output */
      : "r"(INIT32_CS), "i"(INIT32_CS), "m"(*startup64), "r"(0));
  for (;;);
}
