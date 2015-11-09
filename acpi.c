#include "u.h"
#include "dat.h"
#include "mem.h"
#include "console.h"

// * ACPI
// We implement a simple recursive parser of the various in
// memory structures presented by ACPI but do not attempt to interpret
// the bytecode that is also part of the specification.

typedef struct AcpiHeader {
  u32 signature;
  u32 length;
  u8 revision;
  u8 checksum;
  u8 oem[6];
  u8 oemTableId[8];
  u32 oemRevision;
  u32 creatorId;
  u32 creatorRevision;
} __attribute__((packed)) AcpiHeader;

typedef struct AcpiFadt {
  AcpiHeader header;
  u32 firmwareControl;
  u32 dsdt;
  u8 reserved;
  u8 preferredPMProfile;
  u16 sciInterrupt;
  u32 smiCommandPort;
  u8 acpiEnable;
  u8 acpiDisable;
  // TODO - fill in rest of data
} __attribute__((packed)) AcpiFadt;

typedef struct AcpiMadt {
  AcpiHeader header;
  u32 localApicAddr;
  u32 flags;
} __attribute__((packed)) AcpiMadt;

typedef struct ApicHeader {
  u8 type;
  u8 length;
} __attribute__((packed)) ApicHeader;

// APIC structure types
enum {
  APIC_TYPE_LOCAL_APIC = 0,
  APIC_TYPE_IO_APIC = 1,
  APIC_TYPE_INTERRUPT_OVERRIDE = 2
};

typedef struct ApicLocalApic {
  ApicHeader header;
  u8 acpiProcessorId;
  u8 apicId;
  u32 flags;
} __attribute__((packed)) ApicLocalApic;

typedef struct ApicIoApic {
  ApicHeader header;
  u8 ioApicId;
  u8 reserved;
  u32 ioApicAddress;
  u32 globalSystemInterruptBase;
} __attribute__((packed)) ApicIoApic;

typedef struct ApicInterruptOverride {
  ApicHeader header;
  u8 bus;
  u8 source;
  u32 interrupt;
  u16 flags;
} __attribute__((packed)) ApicInterruptOverride;

static void acpiparsefacp(AcpiFadt *facp) {
  if (facp->smiCommandPort) {
  } else {
  }
}

static void acpiparseapic(Acpi a, AcpiMadt *madt) {
  a->localapicaddr = (void *)madt->localApicAddr;

  u8 *p = (u8 *)(madt + 1);
  u8 *end = (u8 *)madt + madt->header.length;
  while (p < end) {
    ApicHeader *header = (ApicHeader *)p;
    u8 type = header->type;
    u8 length = header->length;

    switch (type) {
    case APIC_TYPE_LOCAL_APIC: {
      ApicLocalApic *s = (ApicLocalApic *)p;
      if (a->cpucount < a->maxcpucount) {
        a->cpuids[a->cpucount++] = s->apicId;
      }
      break;
    }
    case APIC_TYPE_IO_APIC: {
      ApicIoApic *s = (ApicIoApic *)p;
      a->ioapicaddr = (void *)s->ioApicAddress;
      break;
    }
    case APIC_TYPE_INTERRUPT_OVERRIDE: {
      ApicInterruptOverride *s = (ApicInterruptOverride *)p;
      break;
    }
    default:
      break;
    }

    p += length;
  }
}


static void acpiparsedt(Acpi a, AcpiHeader *header) {
  u32 signature = header->signature;
  if (signature == 0x50434146) {
    acpiparsefacp((AcpiFadt *)header);
  } else if (signature == 0x43495041) {
    acpiparseapic(a, (AcpiMadt *)header);
  }
}

static void acpiparsersdt(Acpi a, AcpiHeader *rsdt) {
  u32 *p = (u32 *)(rsdt + 1);
  u32 *end = (u32 *)((u8 *)rsdt + rsdt->length);
  while (p < end) {
    u32 address = *p++;
    acpiparsedt(a, (AcpiHeader *)address);
  }
}

static void acpiparsexsdt(Acpi a, AcpiHeader *xsdt) {
  u64 *p = (u64 *)(xsdt + 1);
  u64 *end = (u64 *)((u8 *)xsdt + xsdt->length);
  while (p < end) {
    u64 address = *p++;
    acpiparsedt(a, (AcpiHeader *)address);
  }
}

void acpiinit(Acpi a, Console c) {
  {
    // See 5.2.5.1 and 5.2.5.3 of ACPI specification on where the root
    // system description pointer can be located.
    u8 *p = (u8 *)0x000e0000;
    u8 *end = (u8 *)0x000fffff;
    while (p < end) {
      u64 sig = *(u64*) p;
      if (sig == 0x2052545020445352) {
        cprint(c, "acpi: found rsdp at "), cprintint(c, (u64)p, 16, 0), cnl(c);
        u8 sum = 0;
        for (int i = 0; i < 20; ++i) {
          sum += p[i];
        }
        if (sum) {
          p += 16;
          continue;
        }
        char oem[7];
        mmove(oem, p+9, 6);
        oem[6] = '\0';
        cprint(c, "acpi: OEM is "), cprint(c, oem), cnl(c);
        u8 revision = p[15];
        if (revision == 0) {
          cprint(c, "acpi: version 1\n");
          u32 rsdtaddress = *(u32 *)(p + 16);
          acpiparsersdt(a, (AcpiHeader *)rsdtaddress);
        } else if (revision == 2) {
          cprint(c, "acpi: version 2\n");
          u32 rsdtaddress = *(u32 *)(p + 16);
          u64 xsdtaddress = *(u64 *)(p + 24);
          if (xsdtaddress) {
            acpiparsexsdt(a, (AcpiHeader *)xsdtaddress);
          } else {
            acpiparsersdt(a, (AcpiHeader *)rsdtaddress);
          }
        } else {
        }
        break;
      }
      p += 16;
    }
  }
}
