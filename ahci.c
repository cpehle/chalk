#include "u.h"
#include "dat.h"
#include "pci.h"
#include "arena.h"
#include "ahci.h"
#include "console.h"
#include "mem.h"

#define SATA_SIG_ATA 0x00000101   // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101    // Port multiplier

enum {
  AHCI_CMD_IOSE = (1 << 0),
  AHCI_CMD_MSE = (1 << 1),
  AHCI_CMD_BME = (1 << 2),
  AHCI_CMD_SCE = (1 << 3)
};

// Logic block addressing (LBA)
enum {
  AHCI_PxCMD_ST = (1 << 0),
  AHCI_PxCMD_SUD = (1 << 1),
  AHCI_PxCMD_POD = (1 << 2),
  AHCI_PxCMD_CLO = (1 << 3),
  AHCI_PxCMD_FRE = (1 << 4),
  AHCI_PxCMD_FR = (1 << 14),
  AHCI_PxCMD_CR = (1 << 15),
};

// Frame Information Structure types (FISType)
typedef enum {
  FIS_TYPE_REG_H2D = 0x27,   // Register FIS - host to device
  FIS_TYPE_REG_D2H = 0x34,   // Register FIS - device to host
  FIS_TYPE_DMA_ACT = 0x39,   // DMA activate FIS - device to host
  FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
  FIS_TYPE_DATA = 0x46,      // Data FIS - bidirectional
  FIS_TYPE_BIST = 0x58,      // BIST activate FIS - bidirectional
  FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
  FIS_TYPE_DEV_BITS = 0xA1,  // Set device bits FIS - device to host
} FISType;

// The host issues commands to the device through a command list,
// it has up to 32 slots, each of which can hold a command header.
// The command headers describe ATA or ATAPI commands.
typedef enum {
  AHCI_CMD_LIST_FLAG_CFL = (1 << 0),
  AHCI_CMD_LIST_FLAG_A = (1 << 5), // ATAPI
  AHCI_CMD_LIST_FLAG_W = (1 << 6), // Write
  AHCI_CMD_LIST_FLAG_P = (1 << 7), // Prefetchable
  AHCI_CMD_LIST_FLAG_R = (1 << 8), // Reset
  AHCI_CMD_LIST_FLAG_B = (1 << 9), // BIST
} AhciCommandListFlag;

// section 3.1.1
enum {
  AHCI_CAP_SXS = (1 << 5),   // Supports external SATA
  AHCI_CAP_EMS = (1 << 6),   // enclosure management support
  AHCI_CAP_CCCS = (1 << 7),  // command completion coalescing
  AHCI_CAP_PSC = (1 << 13),  // partial state capable
  AHCI_CAP_SSC = (1 << 14),  // slumber state capable
  AHCI_CAP_PMD = (1 << 15),  // multiple-block pio
  AHCI_CAP_FBSS = (1 << 16), // Fis based switching support
  AHCI_CAP_SPM = (1 << 17),  // supports port multiplier
  AHCI_CAP_SSS = (1 << 27),  // supports staggered spinup
  AHCI_CAP_S64A = (1 << 31), // supports 64bit addressing
};
#define AHCI_NCS(cap)                                                          \
  (cap & ((1 << 12) - 1) & ~((1 << 7) - 1)) // Number of command slots
#define AHCI_NP(cap) (cap & ((1 << 4) - 1)) // Number of ports

// section 3.1.2
// Global Host Control
enum {
  AHCI_GHC_HOST_RESET = (1 << 0),       // Host reset
  AHCI_GHC_INTERRUPT_ENABLE = (1 << 1), // Interrupt enable
  AHCI_GHC_AHCI_ENABLE = (1 << 31),     // Ahci enable
};

struct SataCmdFis {
  u8 reg;
  u8 pmp_type;
  u8 command;
  u8 feature;
  u8 lba_low;
  u8 lba_mid;
  u8 lba_high;
  u8 device;
  u8 lba_low2;
  u8 lba_mid2;
  u8 lba_high2;
  u8 feature2;
  u8 sector_count;
  u8 sector_count2;
  u8 reserved_1;
  u8 control;
  u8 reserved_2[64 - 16];
};

// For documentation see:
// http://www.intel.com/content/www/us/en/io/serial-ata/serial-ata-ahci-spec-rev1_1.html
typedef volatile struct {
  u64 commandlist_base_addr;
  u64 frameinfo_base_addr;
  u32 interrupt_status;
  u32 interrupt_enable;
  u32 command_status;
  u32 _reserved0;
  u32 taskfile_data;
  u32 signature;
  u32 sata_status;
  u32 sata_control;
  u32 sata_error;
  u32 sata_active;
  u32 command_issue;
  u32 sata_notify;
  u32 fis_based_switch;
  u32 _reserved1[15];
} __attribute__((packed)) AhciPort;
// Host Bus Adapter Control
// See section 3.1
typedef struct {
  u32 capabilties;
  u32 global_host_control;
  u32 interrupt_status;
  u32 ports_implemented;
  u32 version;
  u32 command_completion_coalescing_control;
  u32 command_completion_coalescing_ports;
  u32 enclosure_management_location;
  u32 enclosure_management_control;
  u32 extended_capabilities;
  u32 handoff_control_status;
  u32 _reserved0[29];
  u32 _vendor[24];
  AhciPort ports[32];
} AhciControl;
typedef volatile struct {
  u16 cmd;
  u16 prdt_length;
  u32 prd_bytes;
  u64 cmdtable_base;
  u8 _reserved[16];
} AhciCommand;
// Section 4.2.2
typedef struct AhciCommandHeader {
  u16 flags;
  u16 prdtl;          // Physical region descriptor table length in entrie
  volatile u32 prdbc; // Physical region descriptor byte count transferred
  u64 command_table_base_addr; // Command table descriptor base address
  u32 reserved_1[4];           // Reserved
} __attribute__((packed)) AhciCommandHeader;
typedef volatile struct {
  u8 fis_raw[64];
  u8 atapi_cmd[16];
  u8 reserved_1[48];
  struct {
    u32 basel;
    u32 baseu;
    u32 flags;
  } prdt[];
} AhciCommandTable;
typedef struct {
  AhciControl *ctrl;
  AhciPort *port;
  AhciCommand *cmdlist;
  AhciCommandTable *cmdtable;
  u8 *buf, *user_buf;
  int write_back;
  u64 buflen;
} AhciDev;
typedef struct FisData {
  u8 fis_type;             // FIS_TYPE_DATA
  u8 port_multiplier_port; // actually only the first 4 bits.
  u8 reserver0[2];
  u32 data[1]; // Payload
} __attribute__((packed)) FISData;
typedef struct FISRegD2H {
  u8 fis_type; // FIS_TYPE_REG_D2H
  u8 port_multiplier_interrupt;
  u8 status; // Status register
  u8 error;  // Error register
  u64 lba;
  u16 count;
  u8 reserved[6];
} __attribute__((packed)) FISRegD2H;
typedef struct FISRegH2D {
  u8 fis_type; // FIS_TYPE_REG_H2D
  u8 flags;
  u8 command;  // Command register
  u8 featurel; // Feature register, 7:0
  u8 lba0;     // LBA low register, 7:0
  u8 lba1;     // LBA mid register, 15:8
  u8 lba2;     // LBA high register, 23:16
  u8 device;   // Device register
  u8 lba3;     // LBA register, 31:24
  u8 lba4;     // LBA register, 39:32
  u8 lba5;     // LBA register, 47:40
  u8 featureh; // Feature register, 15:8
  u16 count;
  u8 icc;     // Isochronous command completion
  u8 control; // Control register
  u8 rsv1[4]; // Reserved
} __attribute__((packed)) FISRegH2D;
typedef struct FISPioSetup {
  u8 fis_type; // FIS_TYPE_PIO_SETUP
  u32 flags;
  u8 status; // Status register
  u8 error;  // Error register
  u64 lba;
  u16 count;
  u8 rsv3;     // Reserved
  u8 e_status; // New value of status register
  u16 tc;      // Transfer count
  u8 rsv4[2];  // Reserved
} __attribute__((packed)) FISPioSetup;
typedef struct FisDMASetup {
  u8 fis_type; // FIS_TYPE_DMA_SETUP
  u16 flags;
  u64 DMAbufferID; // DMA Buffer Identifier. Used to Identify DMA buffer in host
                   // memory. SATA Spec says host specific and not in Spec.
                   // Trying AHCI spec might work.
  u32 reserved1;   // More reserved
  u32 DMAbufOffset;  // Byte offset into buffer. First 2 bits must be 0
  u32 TransferCount; // Number of bytes to transfer. Bit 0 must be 0
  u32 reserved2;
} __attribute__((packed)) FISDMASetup;

// ahci read and write
void ahciwrite(AhciPort const *port) {
}
/* void ahciread(AhciPort * port, u64 start, u64 count, u64 buf) { */
/*   // Setup command list */
/*   port->interrupt_status = 0xffff; */
/*   int slot = ahcifindcmdslot(port); */
/*   u64 addr = 0; */
/*   AhciCommandHeader *cmdheader = (AhciCommandHeader*)port->commandlist_base_addr; */
/*   cmdheader += slot; */
/*   cmdheader->flags |= (); */
/*   cmdheader->prdtl = ((count-1)>>4) + 1; */

/*   AhciCommandTable *cmdtbl = (AhciCommandTable*)(cmdheader->command_table_base_addr); */

/*   for (int i = 0, N=cmdheader->prdtl-1; i < N; ++i) { */
/*     cmdtbl->prdt[i].basel = buf & 0xffffffff; */
/*     cmdtbl->prdt[i].baseu = ((buf << 32) & 0xffffffff); */
/*     cmdtbl->prdt[i].flags = ; */
/*     buf += 4*1024; */
/*     count -= 16; */
/*   } */

/*   // Command FIS setup */
/*   FISRegH2D *cmdfis = (FISRegH2D*)(&cmdtbl->fis_raw); */
/*   cmdfis->fis_type = FIS_TYPE_REG_H2D; */
/*   cmdfis->flags = */
/*   cmdfis->lba0 = */
/*   cmdfis->lba1 = */
/*   cmdfis->lba2 = */
/*   cmdfis->device = */
/*   cmdfis->lba3 = */
/*   cmdfis->lba4 = */
/*   cmdfis->lba5 = */
/*   cmdfis->count = count; */
/*   // PRDT setup */
/* } */

/* u32 ahcipread(AhciPort const* port, u64 offset) { */
/*   port-> */
/* } */

int ahcipwaiteq(AhciPort const *port, u32 mask, u32 target) {
  for (int i = 0; i < 1000; ++i) {
  }
  return 1;
}


void ahciinitdevice(Arena *m, AhciControl *const ctrl, AhciPort *const port,
                    const int portnum) {
  const int ncs = AHCI_NCS(ctrl->capabilties); // find number of command slots
  AhciDev *const dev = arenapushstruct(m, AhciDev);
  AhciCommandHeader *const cmdlist =
      arenapusharray(m, 32 * sizeof(AhciCommandHeader), AhciCommandHeader);
}

void ahciresetport(AhciControl *const ctrl, int port) {
  if (ctrl->ports[port].command_status &
      (AHCI_PxCMD_ST | AHCI_PxCMD_CR | AHCI_PxCMD_FRE | AHCI_PxCMD_FR)) {
    ctrl->ports[port].command_status &= ~(AHCI_PxCMD_ST | AHCI_PxCMD_FRE);
  }
}

void ahcireset(AhciControl *const ctrl) {
  ctrl->global_host_control |= AHCI_GHC_AHCI_ENABLE | AHCI_GHC_INTERRUPT_ENABLE;
  for (int i = 0; i < 32; ++i) {
    if (ctrl->ports_implemented & (1 << i)) {
      ahciresetport(ctrl, i);
    }
  }
}

static inline int ahciportisactive(const AhciPort *const port) {
  return (port->sata_status & ((0xf << 8) | 0xf)) == ((1 << 8) | (3 << 0));
}

void ahciinitializedevice() {}

void ahciidentify() {
  FISRegH2D fis = {};
  fis.fis_type = FIS_TYPE_REG_H2D;
  //fis.command = ATA_CMD_IDENTIFY;
  fis.device = 0;
  fis.control = 1;
}

u32 ahcichecktype(volatile AhciPort *port) {
  u32 s = port->sata_status;
  u8 ipm, det;
  ipm = (s >> 8) & 0x0f;
  det = s & 0x0f;
  if (ipm != 1 || det != 3) {
    return 0;
  }
  return port->signature;
}

void ahciprobeport(Arena *m, AhciControl *const ctrl, AhciPort *port,
                   const int portnum) {
  port = arenapushstruct(m, AhciPort);
}

void ahcipciinit(Arena *m, Console c, u8 bus, u8 slot) {
  PciConf conf = pciconfread(bus, slot);
  if (conf.class_code != 0x01 ||
      conf.subclass != 0x06) { // PCI class id 0x01 (Mass storage device) and
                               // subclass id 0x06 (serial ATA).
    return;
  }
  cprint(c, "ahci: Found SATA controller.\n");
  AhciControl *const ctrl =
      (AhciControl *)(conf.dev.base_address_register[5].u.address);
  u32 size = conf.dev.base_address_register[5].size;
  AhciPort *const ports = ctrl->ports;
  // reset host controller
  // delay for up to one second, if we had a coorporative scheduler we would
  // yield here for one second (max). Instead we print some dots.
  ctrl->global_host_control |= AHCI_GHC_HOST_RESET;
  for (int i = 0; i < 10; ++i) {
    cputc(c, '.');
  }
  cputc(c, '\n');
  if (ctrl->global_host_control & AHCI_GHC_HOST_RESET) {
    cprint(c, "ahci: Error, controller did not reset.");
    return;
  }
  ctrl->global_host_control |= AHCI_GHC_AHCI_ENABLE; // set ahci access

  const int ncs = AHCI_NCS(ctrl->capabilties);
  AhciCommand *const cmdarr = arenapusharray(m, ncs, AhciCommand);
  AhciCommandTable *const cmdtable = arenapushstruct(m, AhciCommandTable);
  // print some info
  {
    const char *speed_in_gbps;
    u32 speed, slots, ports;
    cprint(c, "registers at: "), cprintint(c, (u32)ctrl, 16, 0),
        cprint(c, " size: "), cprintint(c, size, 16, 0);
    cputc(c, '\n');
    speed = (ctrl->capabilties >> 20) & 0xf;
    if (speed == 1) {
      speed_in_gbps = "1.5";
    } else if (speed == 2) {
      speed_in_gbps = "3";
    } else if (speed == 3) {
      speed_in_gbps = "6";
    } else {
      speed_in_gbps = "?";
    }
    slots = ((ctrl->capabilties >> 8) & 0x1f) + 1;
    ports = (ctrl->capabilties & 0x1f) + 1;
    cprint(c, "slots, ports, Gbps: ");
    cprintint(c, slots, 16, 0), cprint(c, ", "), cprintint(c, ports, 16, 0),
        cprint(c, ", "), cprint(c, speed_in_gbps), cputc(c, '\n');
  }
  for (int i = 0; i < 32; ++i) {
    if (ctrl->ports_implemented & (1 << i)) {
      ahciprobeport(m, ctrl, &ports[i], i);
    }
  }
}
