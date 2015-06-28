#include "u.h"
#include "idt.h"
#include "cpufunc.h"

void default_exception_handler();
void default_interrupt_handler();
void (*exceptionhandlers[20])() = {};

typedef struct Idtentry {
  u16 offset0;
  u16 selector;
  u16 type;
  u16 offset1;
  u32 offset2;
  u32 reserved;
} __attribute__((packed)) Idtentry;

typedef struct Idtdesc {
  u16 limit;
  u64 base;
} __attribute__((packed)) Idtdesc;

void idtinit() {
  for (int i = 0; i < 20; ++i) {
    idtsethandler(i, InterruptGate, exceptionhandlers[i]);
  }
  for (int i = 20; i < 32; ++i) {
    idtsethandler(i, InterruptGate, default_exception_handler);
  }
  for (int i = 32; i < 256; ++i) {
    idtsethandler(i, TrapGate, default_interrupt_handler);
  }
  Idtdesc idtdesc = {.limit = 256 * sizeof(Idtentry) - 1, .base = 0x0};
  lidt(&idtdesc);
}

static inline void idtsetentry(u8 index, u64 base, u16 selector, u16 type) {
  Idtentry *entry = (Idtentry *)0x0 + index;
  entry->offset0 = (u16)base;
  entry->selector = selector;
  entry->type = type;
  entry->offset1 = (u16)(base >> 16);
  entry->offset2 = (u32)(base >> 32);
  entry->reserved = 0;
}

void idtsethandler(u8 index, InterruptType type, void(*handler)) {
  if (handler) {
    u16 selector = 0x8;
    idtsetentry(index, (u64)handler, selector, type);
  } else {
    idtsetentry(index, 0, 0, 0);
  }
}
