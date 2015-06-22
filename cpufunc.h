/*	$OpenBSD: cpufunc.h,v 1.12 2015/03/21 20:42:38 kettenis Exp $	*/
/*	$NetBSD: cpufunc.h,v 1.3 2003/05/08 10:27:43 fvdl Exp $	*/

/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Functions to provide access to i386-specific instructions.
 */

#include "specialreg.h"

static __inline void invlpg(u32);
static __inline void lidt(void *);
static __inline void lldt(u16);
static __inline void ltr(u16);
static __inline void lcr0(u32);
static __inline u32 rcr0(void);
static __inline u32 rcr2(void);
static __inline void lcr3(u32);
static __inline u32 rcr3(void);
static __inline void lcr4(u32);
static __inline u32 rcr4(void);
static __inline void tlbflushg(void);
static __inline void disable_intr(void);
static __inline void enable_intr(void);
static __inline u32 read_eflags(void);
static __inline void write_eflags(u32);
static __inline void wbinvd(void);
static __inline void clflush(u32 addr);
static __inline void mfence(void);
static __inline void wrmsr(u32, u64);
static __inline u64 rdmsr(u32);
static __inline void breakpoint(void);

static __inline void
invlpg(u32 addr)
{
        __asm __volatile("invlpg (%0)" : : "r" (addr) : "memory");
}

static __inline void
lidt(void *p)
{
    __asm __volatile("lidt (%0)" : : "r" (p) : "memory");
}

static __inline void
lldt(u16 sel)
{
    __asm __volatile("lldt %0" : : "r" (sel));
}

static __inline void
ltr(u16 sel)
{
    __asm __volatile("ltr %0" : : "r" (sel));
}

static __inline void
lcr0(u32 val)
{
    __asm __volatile("movl %0,%%cr0" : : "r" (val));
}

static __inline u32
rcr0(void)
{
    u32 val;
    __asm __volatile("movl %%cr0,%0" : "=r" (val));
    return val;
}

static __inline u32
rcr2(void)
{
    u32 val;
    __asm __volatile("movl %%cr2,%0" : "=r" (val));
    return val;
}

static __inline void
lcr3(u32 val)
{
    __asm __volatile("movl %0,%%cr3" : : "r" (val));
}

static __inline u32
rcr3(void)
{
    u32 val;
    __asm __volatile("movl %%cr3,%0" : "=r" (val));
    return val;
}

static __inline void
lcr4(u32 val)
{
    __asm __volatile("movl %0,%%cr4" : : "r" (val));
}

static __inline u32
rcr4(void)
{
    u32 val;
    __asm __volatile("movl %%cr4,%0" : "=r" (val));
    return val;
}

static __inline void
tlbflush(void)
{
    u32 val;
    __asm __volatile("movl %%cr3,%0" : "=r" (val));
    __asm __volatile("movl %0,%%cr3" : : "r" (val));
}


#ifdef notyet
void	setidt(int idx, /*XXX*/caddr_t func, int typ, int dpl);
#endif


/* XXXX ought to be in psl.h with spl() functions */

static __inline void
disable_intr(void)
{
    __asm __volatile("cli");
}

static __inline void
enable_intr(void)
{
    __asm __volatile("sti");
}

static __inline u32
read_eflags(void)
{
    u32 ef;

    __asm __volatile("pushfl; popl %0" : "=r" (ef));
    return (ef);
}

static __inline void
write_eflags(u32 ef)
{
    __asm __volatile("pushl %0; popfl" : : "r" (ef));
}

static __inline void
wbinvd(void)
{
        __asm __volatile("wbinvd");
}

static __inline void
clflush(u32 addr)
{
    __asm __volatile("clflush %0" : "+m" (addr));
}

static __inline void
mfence(void)
{
    __asm __volatile("mfence" : : : "memory");
}

static __inline void
wrmsr(u32 msr, u64 newval)
{
        __asm __volatile("wrmsr" : : "A" (newval), "c" (msr));
}

static __inline u64
rdmsr(u32 msr)
{
        u64 rv;

        __asm __volatile("rdmsr" : "=A" (rv) : "c" (msr));
        return (rv);
}

/*
 * Some of the undocumented AMD64 MSRs need a 'passcode' to access.
 *
 * See LinuxBIOSv2: src/cpu/amd/model_fxx/model_fxx_init.c
 */

#define	OPTERON_MSR_PASSCODE	0x9c5a203a

static __inline u64
rdmsr_locked(u32 msr, u32 code)
{
    u64 rv;
    __asm volatile("rdmsr"
        : "=A" (rv)
        : "c" (msr), "D" (code));
    return (rv);
}

static __inline void
wrmsr_locked(u32 msr, u32 code, u64 newval)
{
    __asm volatile("wrmsr"
        :
        : "A" (newval), "c" (msr), "D" (code));
}

/* Break into DDB/KGDB. */
static __inline void
breakpoint(void)
{
    __asm __volatile("int $3");
}
