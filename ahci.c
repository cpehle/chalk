#include "u.h"
#include "dat.h"
#include "pci.h"
#include "ahci.h"
#include "console.h"
#include "mem.h"
#include "arena.h"



typedef struct Drive {

} Drive;


//
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
//
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
  u32 rsv1[4];                 // Reserved
} __attribute__((packed)) AhciCommandHeader;

typedef volatile struct {
  u8 fis_raw[64];
  u8 atapi_cmd[16];
  u8 _reserved0[48];
  struct {
    u64 data_base_addr;
    u32 _reserved0;
    u32 flags;
  } prdt[8]; /* Can be up to 65,535 prds,
                but implementation needs multiple of 128 bytes. */
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


/* static u8* cfissetup() { */
/*   u8 *cfis; */
/*   cfis = pc->pm->ctab->cfis; */
/*   memset(cfis, 0, 0x20); */
/*   cfis[0] = 0x27; */
/*   cfis[1] = 0x80; */
/*   cifs[7] = Obs; */
/*   return cfis; */
/* } */

/* static void listsetup(Aportc *pc, int flags) { */
/*   Alist *list; */

/*   list = pc->pm->list; */
/*   list->flags = flags | 5; */
/*   list->len = 0; */
/*   list->ctab = pc->pm->ctab; */
/*   list->ctabhi = 0; */
/* } */



/* static int setupdmamode() { */
/*   u8 *c; */
/*   c = cfissetup(pc); */
/*   c[2] = 0xef; */
/*   c[3] = 3; */
/*   c[12] = 0x40 | f; */
/*   listsetup(pc, Lwrite); */

/* } */


void ahciread(AhciPort const *port, u64 start, u32 count, u8 *buf) {
  AhciCommandHeader *cmdhdr = (AhciCommandHeader *)port->commandlist_base_addr;

  AhciCommandTable *cmdtbl =
      (AhciCommandTable *)cmdhdr->command_table_base_addr;
}

// TODO: Timeout? Need to figure out how to handle errors.
static void ahci_start_command_engine(AhciPort *p) {
  while (p->command_status & AHCI_PxCMD_CR)
    ;
  p->command_status |= AHCI_PxCMD_FRE;
  p->command_status |= AHCI_PxCMD_ST;
}
// TODO: Need to figure out, if we want to time out after a while.
static void ahci_stop_command_engine(AhciPort *p) {
  p->command_status &= ~AHCI_PxCMD_ST;
  while (1) {
    if (p->command_status & AHCI_PxCMD_FR)
      continue;
    if (p->command_status & AHCI_PxCMD_CR)
      continue;
    break;
  }
  p->command_status &= ~AHCI_PxCMD_FRE;
}

/* void ahciinitdevice(Arena *m, AhciControl *const ctrl, AhciPort *const port, */
/*                       const int portnum) { */
/*   const int ncs = AHCI_NCS(ctrl->capabilties); // find number of command slots */
/*   AhciDev *const dev = arena_push_struct(m, AhciDev); */
/*   AhciCommandHeader *const cmdlist = */
/*       arena_push_array(m, 32 * sizeof(AhciCommandHeader), AhciCommandHeader); */
/* } */

void ahciprobeport(Arena *m, AhciControl *const ctrl, AhciPort *const port,
                     const int portnum) {
  if (ctrl->capabilties & AHCI_CAP_SSS) {
    port->command_status |= AHCI_PxCMD_SUD;
  }
  // ahci_clear_status(port, sata_error);
  // ahci_clear_status(port, intr_status);
  //ahci_init_device(m, ctrl, port, portnum);
}

void ahcipciinit(Console c, u8 bus, u8 slot) {
  PciConf conf = pciconfread(bus, slot);
  // PCI class id 0x01 (Mass storage device) and subclass id 0x06 (serial ATA).
  if (conf.class_code != 0x01 || conf.subclass != 0x06) {
    return;
  }
  cprint(c, "ahci: Found SATA controller");

  //pcicmd &= ~(PCI_command_io | PCI_command_int_disable);
  //pcicmd |= PCI_command_master | PCI_command_memory;


  AhciControl *const ctrl = (AhciControl *)(conf.dev.base_address_register[4].u.address);
  // size_t size = pciInfo.u.h0.base_register_sizes[5];
  cprint(c, "Registers at: "), cprintint(c, (u32)ctrl, 16, 0);
  AhciPort *const ports = ctrl->ports;
  ctrl->global_host_control |= AHCI_GHC_HOST_RESET; // reset the controller.
  while (ctrl->global_host_control & AHCI_GHC_HOST_RESET) {
  }
  cprint(c, "ahci: Reset SATA controller.");
  ctrl->global_host_control |= AHCI_GHC_AHCI_ENABLE; // enable AHCI
  for (int i = 0; i < 32; ++i) {
    if (ctrl->ports_implemented & (1 << i)) {
      // ahci_probe_port(m, ctrl, &ports[i], i + 1);
    }
  }
  return;
}
