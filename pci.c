#include "u.h"
#include "dat.h"
#include "io.h"
#include "pci.h"
#include "console.h"

// Configuration method one.
// won't support method two for now, as it is deprecated.
static const int pci_address_ioport = 0x0cf8;
static const int pci_data_ioport = 0x0cfc;

//  address dd 10000000000000000000000000000000b
//        E    Res    Bus    Dev  F  Reg   0
// Bits
// 31		Enable bit = set to 1
// 30 - 24	Reserved = set to 0
// 23 - 16	Bus number = 256 options
// 15 - 11	Device/Slot number = 32 options
// 10 - 8	Function number = will leave at 0 (8 options)
// 7 - 2    Register number = will leave at 0 (64 options) 64 x 4 bytes =
// 256 bytes worth of accessible registers
// 1 - 0	Set to 0
void write_pci(u32 bus, u32 slot, u32 function, u32 reg, u32 data) {
  u32 address = (1 << 31) | (bus << 16) | (slot << 11) | (function << 8) | reg;
  outl(pci_address_ioport, address);
  outl(pci_data_ioport, data);
}

u32 read_pci(u32 bus, u32 slot, u32 function, u32 reg) {
  u32 address = 0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | reg;
  outl(pci_address_ioport, address);
  return inl(pci_data_ioport);
}

static void pcibar0(u32 bus, u32 slot, u32 func, u32 index, u32 *address,
                    u32 *mask) {
  u32 reg = 0x10 + index * sizeof(u32);
  *address = read_pci(bus, slot, func, reg);
  write_pci(bus, slot, func, reg, 0xffffffff);
  *mask = read_pci(bus, slot, func, reg);
  write_pci(bus, slot, func, reg, *address);
}

static PciBar pcibar(u32 bus, u32 slot, u32 func, u32 index) {
  u32 al, ml;
  u64 a;
  u64 size;
  u8 flags;
  PciBar res = {{}, .size = 0, .flags = 0, .tag = PciBarInvalid};
  pcibar0(bus, slot, func, index, &al, &ml);
  // the memory space bar layout is
  // 31 - 4                       | 3            | 2 - 1 | 0
  // 16-byte aligned base address | prefetchable | Type  | 0
  // the i/o space bar layout is
  // 31 - 2                      | 1        | 0
  // 4-byte aligned base address | reserved | 1
  if (al & 0x1) { // i/o space
    res.port = (u16)(al & ~0x3);
    res.size = (u16)(~(ml & ~0x3) + 1);
    res.flags = al & 0x3;
    res.tag = PciBarIO;
  } else {
    if (al & 0x4) { // 64-bit
      u32 ah, mh;
      pcibar0(bus, slot, func, index, &ah, &mh);
      res.address = (void *)((((u64)ah) << 32) | (al & ~0xf));
      res.size = ~(((u64)mh << 32) | (ml & ~0xf)) + 1;
      res.flags = al & 0xf;
      res.tag = PciBarM64;
    } else if (~(al & 0x6)) { // 32-bit
      res.address = (void *)(al & ~0xf);
      res.size = ~(ml & ~0xf) + 1;
      res.flags = al & 0xf;
      res.tag = PciBarM32;
    } else { // 16-bit
    }
  }
  return res;
}

PciConf pciconfread(u8 bus, u8 slot) {
  PciConf conf = {};
  u8 func = 0;
  u32 val = read_pci(bus, slot, func, 0x00);
  conf.vendor_id = val & 0xffff;
  conf.device_id = val >> 16;
  if (conf.vendor_id == 0xffff) {
    return conf;
  }

  val = read_pci(bus, slot, func, 0x04);
  conf.command_reg = val & 0xffff;
  conf.status_reg = val >> 16;
  val = read_pci(bus, slot, func, 0x08);
  conf.revision_id = val & 0xff;
  conf.prog_if = (val >> 8) & 0xff;
  conf.subclass = (val >> 16) & 0xff;
  conf.class_code = val >> 24;
  val = read_pci(bus, slot, func, 0x0C);
  conf.cache_line_size = val & 0xff;
  conf.latency_timer = (val >> 8) & 0xff;
  conf.header_type = (val >> 16) & 0xff;
  conf.bist_register = val >> 24;

  switch (conf.header_type) {
  case 0x00:
    // standard header type:
    {
      for (int i = 0; i < 6; i++) {
        conf.dev.base_address_register[i] = pcibar(bus, slot, func, i);
      }
      conf.dev.cardbus_cis_ptr = read_pci(bus, slot, func, 0x28);
      val = read_pci(bus, slot, func, 0x2c);
      conf.dev.subsystem_vendor = val & 0xffff;
      conf.dev.subsystem = val >> 16;
      conf.dev.expansion_rom_base_address = read_pci(bus, slot, func, 0x30);
      conf.dev.capability_ptr = read_pci(bus, slot, func, 0x34) & 0xff;
      val = read_pci(bus, slot, func, 0x30);
      conf.dev.interrupt_line = val & 0xff;
      conf.dev.interrupt_pin = (val >> 8) & 0xff;
      conf.dev.min_grant = (val >> 16) & 0xff;
      conf.dev.max_latency = val >> 24;
    };
    break;
  case 0x01: {
    /* PCI-PCI bridge */

  }; break;
  case 0x08: {
    /* multifunction device */
    for (func = 1; func < 8; func++) {
    }
  } break;
  }
  return conf;
}

void pciscan(Console c) {
  for (u8 bus = 0; bus < 255; bus++) {
    for (u8 dev = 0; dev < 32; dev++) {
      PciConf conf = pciconfread(bus, dev);
      if ((conf.device_id == 0xffff) || (conf.vendor_id == 0xffff)) {
        continue;
      }
      cprint(c, "pci(bus,dev): ("), cprintint(c, bus, 16, 0), cputc(c, ','),
        cprintint(c, dev, 16, 0), cputc(c, ')'), cnl(c);
      cprint(c, "device id: "), cprintint(c, conf.device_id, 16, 0),
          cnl(c);
      cprint(c, "vendor id: "), cprintint(c, conf.vendor_id, 16, 0),
        cnl(c);
      for (int i = 0; i < 6; ++i) {
        u64 size = conf.dev.base_address_register[i].size;
        if (size) {
          switch (conf.dev.base_address_register[i].tag) {
          case PciBarInvalid: {
            cprint(c, " Invalid bar:");
            break;
          }
          case PciBarM16: {
            cprint(c, " M16 Bar:");
            break;
          }
          case PciBarM32: {
            cprint(c, " M32 Bar:");
            break;
          }
          case PciBarM64: {
            cprint(c, " M64 Bar:");
            break;
          }
          case PciBarIO: {
            cprint(c, " IO Bar:");
            break;
          }
          }
          cprintint(c, conf.dev.base_address_register[i].size, 16, 0);
        }
      }
      cnl(c);
    }
  }
}
