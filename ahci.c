#include "u.h"
#include "dat.h"
#include "pci.h"
#include "arena.h"
#include "ahci.h"
#include "console.h"
#include "mem.h"

#define CMD_CFL_SHIFT	0		/* CFL - Command FIS Length */
#define CMD_CFL_MASK	(0xf << CMD_CFL_SHIFT)
#define CMD_CFL(x)	((((x) >> 2) << CMD_CFL_SHIFT) & CMD_CFL_MASK)

typedef enum AtaCommand
{
    ATA_NOP                                 = 0x00,
    ATA_CFA_REQUEST_EXTENDED_ERROR_CODE     = 0x03,
    ATA_DATA_SET_MANAGEMENT                 = 0x06,
    ATA_DEVICE_RESET                        = 0x08,
    ATA_RECALIBRATE                         = 0x10,
    ATA_READ_SECTORS                        = 0x20,
    ATA_READ_SECTORS_WITHOUT_RETRIES        = 0x21,
    ATA_READ_LONG                           = 0x22,
    ATA_READ_LONG_WITHOUT_RETRIES           = 0x23,
    ATA_READ_SECTORS_EXT                    = 0x24,
    ATA_READ_DMA_EXT                        = 0x25,
    ATA_READ_DMA_QUEUED_EXT                 = 0x26,
    ATA_READ_NATIVE_MAX_ADDRESS_EXT         = 0x27,
    ATA_READ_MULTIPLE_EXT                   = 0x29,
    ATA_READ_STREAM_DMA_EXT                 = 0x2a,
    ATA_READ_STREAM_EXT                     = 0x2b,
    ATA_READ_LOG_EXT                        = 0x2f,
    ATA_WRITE_SECTORS                       = 0x30,
    ATA_WRITE_SECTORS_WITHOUT_RETRIES       = 0x31,
    ATA_WRITE_LONG                          = 0x32,
    ATA_WRITE_LONG_WITHOUT_RETRIES          = 0x33,
    ATA_WRITE_SECTORS_EXT                   = 0x34,
    ATA_WRITE_DMA_EXT                       = 0x35,
    ATA_WRITE_DMA_QUEUED_EXT                = 0x36,
    ATA_SET_MAX_ADDRESS_EXT                 = 0x37,
    ATA_CFA_WRITE_SECTORS_WITHOUT_ERASE     = 0x38,
    ATA_WRITE_MULTIPLE_EXT                  = 0x39,
    ATA_WRITE_STREAM_DMA_EXT                = 0x3a,
    ATA_WRITE_STREAM_EXT                    = 0x3b,
    ATA_WRITE_VERIFY                        = 0x3c,
    ATA_WRITE_DMA_FUA_EXT                   = 0x3d,
    ATA_WRITE_DMA_QUEUED_FUA_EXT            = 0x3e,
    ATA_WRITE_LOG_EXT                       = 0x3f,
    ATA_READ_VERIFY_SECTORS                 = 0x40,
    ATA_READ_VERIFY_SECTORS_WITHOUT_RETRIES = 0x41,
    ATA_READ_VERIFY_SECTORS_EXT             = 0x42,
    ATA_WRITE_UNCORRECTABLE_EXT             = 0x45,
    ATA_READ_LOG_DMA_EXT                    = 0x47,
    ATA_FORMAT_TRACK                        = 0x50,
    ATA_CONFIGURE_STREAM                    = 0x51,
    ATA_WRITE_LOG_DMA_EXT                   = 0x57,
    ATA_TRUSTED_RECEIVE                     = 0x5c,
    ATA_TRUSTED_RECEIVE_DMA                 = 0x5d,
    ATA_TRUSTED_SEND                        = 0x5e,
    ATA_TRUSTED_SEND_DMA                    = 0x5f,
    ATA_READ_FPDMA_QUEUED                   = 0x60,
    ATA_WRITE_FPDMA_QUEUED                  = 0x61,
    ATA_SEEK                                = 0x70,
    ATA_CFA_TRANSLATE_SECTOR                = 0x87,
    ATA_EXECUTE_DEVICE_DIAGNOSTIC           = 0x90,
    ATA_INITIALIZE_DEVICE_PARAMETERS        = 0x91,
    ATA_DOWNLOAD_MICROCODE                  = 0x92,
    ATA_STANDBY_IMMEDIATE__ALT              = 0x94,
    ATA_IDLE_IMMEDIATE__ALT                 = 0x95,
    ATA_STANDBY__ALT                        = 0x96,
    ATA_IDLE__ALT                           = 0x97,
    ATA_CHECK_POWER_MODE__ALT               = 0x98,
    ATA_SLEEP__ALT                          = 0x99,
    ATA_PACKET                              = 0xa0,
    ATA_IDENTIFY_PACKET_DEVICE              = 0xa1,
    ATA_SERVICE                             = 0xa2,
    ATA_SMART                               = 0xb0,
    ATA_DEVICE_CONFIGURATION_OVERLAY        = 0xb1,
    ATA_NV_CACHE                            = 0xb6,
    ATA_CFA_ERASE_SECTORS                   = 0xc0,
    ATA_READ_MULTIPLE                       = 0xc4,
    ATA_WRITE_MULTIPLE                      = 0xc5,
    ATA_SET_MULTIPLE_MODE                   = 0xc6,
    ATA_READ_DMA_QUEUED                     = 0xc7,
    ATA_READ_DMA                            = 0xc8,
    ATA_READ_DMA_WITHOUT_RETRIES            = 0xc9,
    ATA_WRITE_DMA                           = 0xca,
    ATA_WRITE_DMA_WITHOUT_RETRIES           = 0xcb,
    ATA_WRITE_DMA_QUEUED                    = 0xcc,
    ATA_CFA_WRITE_MULTIPLE_WITHOUT_ERASE    = 0xcd,
    ATA_WRITE_MULTIPLE_FUA_EXT              = 0xce,
    ATA_CHECK_MEDIA_CARD_TYPE               = 0xd1,
    ATA_GET_MEDIA_STATUS                    = 0xda,
    ATA_ACKNOWLEDGE_MEDIA_CHANGE            = 0xdb,
    ATA_BOOT_POST_BOOT                      = 0xdc,
    ATA_BOOT_PRE_BOOT                       = 0xdd,
    ATA_MEDIA_LOCK                          = 0xde,
    ATA_MEDIA_UNLOCK                        = 0xdf,
    ATA_STANDBY_IMMEDIATE                   = 0xe0,
    ATA_IDLE_IMMEDIATE                      = 0xe1,
    ATA_STANDBY                             = 0xe2,
    ATA_IDLE                                = 0xe3,
    ATA_READ_BUFFER                         = 0xe4,
    ATA_CHECK_POWER_MODE                    = 0xe5,
    ATA_SLEEP                               = 0xe6,
    ATA_FLUSH_CACHE                         = 0xe7,
    ATA_WRITE_BUFFER                        = 0xe8,
    ATA_WRITE_SAME                          = 0xe9,
    ATA_FLUSH_CACHE_EXT                     = 0xea,
    ATA_IDENTIFY_DEVICE                     = 0xec,
    ATA_MEDIA_EJECT                         = 0xed,
    ATA_IDENTIFY_DMA                        = 0xee,
    ATA_SET_FEATURES                        = 0xef,
    ATA_SECURITY_SET_PASSWORD               = 0xf1,
    ATA_SECURITY_UNLOCK                     = 0xf2,
    ATA_SECURITY_ERASE_PREPARE              = 0xf3,
    ATA_SECURITY_ERASE_UNIT                 = 0xf4,
    ATA_SECURITY_FREEZE_LOCK                = 0xf5,
    ATA_SECURITY_DISABLE_PASSWORD           = 0xf6,
    ATA_READ_NATIVE_MAX_ADDRESS             = 0xf8,
    ATA_SET_MAX                             = 0xf9
} AtaCommand;

/*

  HBA State machine (c.f 5.1)
  ---------------------------

  * Idle states

  ** H:Init
  Entered by setting GHC.HR == 1
  post condition GHC.HR == 0

  HBA Port state machine (c.f. 5.2)
  ---------------------------------

*/

enum {
  SATA_SIG_ATA = 0x00000101,    // SATA drive
  SATA_SIG_ATAPI = 0xEB140101, // SATAPI drive
  SATA_SIG_SEMB = 0xC33C0101,  // Enclosure management bridge
  SATA_SIG_PM = 0x96690101    // Port multiplier
};
enum {
  AHCI_CMD_IOSE = (1 << 0),
  AHCI_CMD_MSE = (1 << 1),
  AHCI_CMD_BME = (1 << 2),
  AHCI_CMD_SCE = (1 << 3)
};
enum {
  AHCI_PxIS_PCS = 1,
};

enum {
  AHCI_DMAReceive,
  AHCI_DMATransmit,
  AHCI_PIOTransmit,
  AHCI_PIOReceive,
};

#define BYTES_PER_PRD_SHIFT	20
#define BYTES_PER_PRD		(4 << 20)

#define PRD_TABLE_I		(1 << 31) /* I - Interrupt on Completion */
#define PRD_TABLE_BYTES_MASK	0x3fffff
#define PRD_TABLE_BYTES(x)	(((x) - 1) & PRD_TABLE_BYTES_MASK)


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
#define AHCI_PxCMD_ICC_SHIFT	28
#define AHCI_PxCMD_ICC_MASK	(0xf << AHCI_PxCMD_ICC_SHIFT)
#define AHCI_PxCMD_ICC_ACTIVE	(0x1 << AHCI_PxCMD_ICC_SHIFT)

#define AHCI_PxSSTS_IPM_SHIFT		8
#define AHCI_PxSSTS_IPM_MASK		(0xf << AHCI_PxSSTS_IPM_SHIFT)
#define AHCI_PxSSTS_IPM_ACTIVE		(1 << AHCI_PxSSTS_IPM_SHIFT)
#define AHCI_PxSSTS_DET_SHIFT		0
#define AHCI_PxSSTS_DET_MASK		(0xf << AHCI_PxSSTS_DET_SHIFT)
#define AHCI_PxSSTS_DET_ESTABLISHED	(3 << AHCI_PxSSTS_DET_SHIFT)


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

#define FIS_H2D_CMD	(1 << 7)
#define FIS_H2D_FIS_LEN	20
#define FIS_H2D_DEV_LBA	(1 << 6)

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
  AHCI_CAP_SAM = (1 << 18),  // supports only AHCI mode
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
volatile struct AhciPort {
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
  u32 _reserved1[11];
  u32 _vendor[4];
} __attribute__((packed));

// Host Bus Adapter Control
// See section 3.1
struct AhciControl {
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
};

volatile struct  ReceivedFis {
    u8 dma_setup_fis[28];
    u8 _reserved0[4];
    u8 pio_setup_fis[20];
    u8 _reserved1[12];
    u8 d2h_register_fis[20];
    u8 _reserved2[4];
    u8 set_device_bits_fis[8];
    u8 unknown_fis[64];
    u8 _reserved3[96];
};

volatile struct AhciCommand {
  u16 cmd;
  u16 prdt_length;
  u32 prd_bytes;
  u64 cmdtable_base;
  u8 _reserved[16];
};

// Section 4.2.2
typedef struct AhciCommandHeader {
  u16 flags;
  u16 prdtl;          // Physical region descriptor table length in entrie
  volatile u32 prdbc; // Physical region descriptor byte count transferred
  u64 command_table_base_addr; // Command table descriptor base address
  u32 reserved_1[4];           // Reserved
} __attribute__((packed)) AhciCommandHeader;

typedef struct {
  u32 basel;
  u32 baseu;
  u32 reserved_2;
  u32 descriptorinformation;
} __attribute__((packed)) AhciPrdt;

volatile struct AhciCommandTable{
  u32 fis_raw[16];
  u8 atapi_cmd[16];
  u8 reserved_1[48];
  AhciPrdt prdt[65535];
};

static inline u32 _ahciclearstatus(volatile u32 *const reg)
{
    const u32 bits = *reg;
    if (bits)
        *reg = bits;
    return bits;
}
#define ahciclearstatus(p, r) _ahciclearstatus(&(p)->r)

// ahci read and write
void ahciwriteblocking(AhciPort const *port) {
}

void ahciwriteasync(AhciPort const *port) {
}

typedef struct {
  u64 nextsector;
  u64 nextmemory;
  u16 readsectors;
} AhciAsyncRead;



static int ahcicommandslotexec(AhciDev const dev) {
  const int slot = 0;
  if (!(dev->port->command_status & AHCI_PxCMD_CR)) {
    return -1;
  }
  dev->port->command_issue |= (1<<slot); // trigger command execution
  // poll for completetion

  while ((dev->port->command_issue & (1<<slot))) {
    /* if (port->interrupt_status & AHCI_PxIS_TFES) { // task file error */
    /* } */
  }
}

static int ahciidentifydevice(AhciDev const dev, u64 bufbase)
{
  // ahcicommandslotprepare(dev, bufbase, 512, 0);
    dev->commandtable->fis_raw[0] = FIS_TYPE_REG_H2D;
    dev->commandtable->fis_raw[1] = FIS_H2D_CMD;
    dev->commandtable->fis_raw[2] = ATA_IDENTIFY_DEVICE;
    if ((ahcicommandslotexec(dev) < 0) || (dev->commandlist->prd_bytes != 512))
        return -1;
    else
        return 0;
}

void ahcireadasync() {

}

AhciBlockingRead ahcireadblocking(AhciDev dev, u64 src, u64 dst, u64 length) {
  AhciPort *port = dev->port;
  AhciBlockingRead res = {src, dst, 0};

  if (src >= (1ULL << 48)) {
    return res;
  }
  const int slot = 0;
  int readcount = 0;
  dev->commandlist[slot].cmd = CMD_CFL(FIS_H2D_FIS_LEN);
  dev->commandlist[slot].cmdtable_base = (u64)dev->commandtable;

  AhciCommandHeader * cmdheader = ((AhciCommandHeader*)dev->port->commandlist_base_addr) + slot;

  // Physical region descriptor table setup
  if (length > 0) {
    u64 prdtlength = ((length - 1) >> BYTES_PER_PRD_SHIFT) + 1;
    if (prdtlength > 65535) {
      prdtlength = 65535;
      length = prdtlength << BYTES_PER_PRD_SHIFT;
    }
    dev->commandlist[slot].prdt_length = prdtlength;
    for (int i = 0; i < prdtlength; ++i) {
      const int bytes = (length < BYTES_PER_PRD) ? length : BYTES_PER_PRD;
      dev->commandtable->prdt[i].basel = (dst >> 0) & 0xffffffff;
      dev->commandtable->prdt[i].baseu = (dst >> 32) & 0xffffffff;
      dev->commandtable->prdt[i].descriptorinformation = PRD_TABLE_BYTES(bytes);
      dst += bytes;
      length -= bytes;
    }
  }


  u64 addr = 0;
  cmdheader->flags = 0;
  if (((length-1)>>4) + 1 >= 65535) {
    return res;
  }
  cmdheader->prdtl = ((length-1)>>4) + 1;

  AhciCommandTable *cmdtbl = (AhciCommandTable*)(cmdheader->command_table_base_addr);
  // 4.2.3.3 Physical region descriptor table
  // Contains between 0 and 65535 entries

  // Command FIS setup
  cmdtbl->fis_raw[ 0] = 0x00258027;




  return res;
}

/* u32 ahcipread(AhciPort const* port, u64 offset) { */
/*   port-> */
/* } */

int ahcipwaiteq(AhciPort const *port, u32 mask, u32 target) {
  for (int i = 0; i < 1000; ++i) {
  }
  return 1;
}

void ahcihandleintterrupts(AhciPort *const port) {
  // 5.5.3
  // 1. check on which ports there are interrupts pending

  // port->interrupt_status;

  // 2. clear the important interrupt registers
  // 3. clear the interrupt bits
  // 4. For non queued commands read PxCI / PxSACT /
  // 5. recover from errors
}

void ahciatadmawrite() {
}

void ahciatadmaread() {
  // Fetch -> Transmit

  // CFIS:Success

}



void ahcifindemptycommandslot(AhciPort *const port) {
  int commandslot = 0;
  if ((port->command_issue & commandslot) && (port->sata_control & commandslot)) {
  }
  // build a command FIS in system memory at this location
  ((u32 *)(port->commandlist_base_addr))[commandslot] = 0;
  // 5.5.1
  // 3.
  // a. need to set number of PRD table
  // b. CFL set to length of the command in the cfis area
  // d. W bit set if data is going to the device
  // e. P prefetch bit set
  // f. Port multipliers

  // 4. If it is a queued command PxSACT.DS(commandslot)
  port->sata_control &= (1<<commandslot);
  // 5. Set command issue slot
  port->command_issue &= (1<<commandslot);
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

int ahcistartcommandengine(Console c, volatile AhciPort * port) {
  int timeout = 1000;
  while ((port->command_status & AHCI_PxCMD_CR) && timeout--) {
  }
  if (timeout < 0) {
    cprint(c, "ahci: command engine did not start.\n");
    return 1;
  }
  port->command_status |= AHCI_PxCMD_FRE;
  port->command_status |= AHCI_PxCMD_ST;
  return 0;
}

int ahcistopcommandengine(Console c, volatile AhciPort * port) {
  port->command_status &= ~AHCI_PxCMD_ST;
  int timeout = 1000;
  while ((port->command_status & (AHCI_PxCMD_FR | AHCI_PxCMD_CR)) && timeout--) {
  }
  if (timeout < 0) {
    cprint(c, "ahci: command engine did not stop.\n");
    return 1;
  }

  port->command_status &= ~AHCI_PxCMD_FRE;
  return 0;
}

void ahciportrebase(volatile AhciPort * port) {

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


void ahcideviceinit(Arena *m, Console c, AhciDev const dev, AhciControl *const control, AhciPort *port,
                     const int portnum) {
  const int ncs = AHCI_NCS(control->capabilties);
  // need to be alligned to 1024
  AhciCommand *const commandlist = arenapusharrayalign(m, ncs, AhciCommand, 1024);
  AhciCommandTable *const commandtable = arenapushstructalign(m, AhciCommandTable, 128);
  ReceivedFis *const receivedfis = arenapushstructalign(m, ReceivedFis, 256);
  // AhciDev const dev = arenapushstruct(m, AhciDevDesc);

  if (!ahcistopcommandengine(c, port)) {
         // cleanup
  };
  port->commandlist_base_addr = commandlist;
  port->frameinfo_base_addr = receivedfis;
  if (!ahcistartcommandengine(c, port)) {
      // cleanup
  };

  // put port into active state
  port->command_status |= AHCI_PxCMD_ICC_ACTIVE;

  dev->control = control;
  dev->port = port;
  dev->commandlist = commandlist;
  dev->commandtable = commandtable;
  dev->receivedfis = receivedfis;

  // int timeout = 20000;
  // while ((port->taskfile_data & AHCI_PxTFD_BSY) && timeout--) {
  //}

  switch (ahcichecktype(port)) {
  case SATA_SIG_ATA: {
    cprint(c, "ahci: found ata device on port "), cprintint(c, portnum, 16, 0),
        cputc(c, '\n');
    // dev->identify = ahciidentifydevice;
    // dev->readsectors = ahciatareadsectors;
    //return ataattachdevice(&dev->atadevice, PORT_TYPE_SATA);
    break;
  }
  case SATA_SIG_ATAPI: {
    break;
  }
  case SATA_SIG_SEMB: {
    break;
  }
  case SATA_SIG_PM: {
    break;
  }
  default: { break; }
  };
}

static inline int ahciportisactive(const AhciPort *const port)
{
    return (port->sata_status & (AHCI_PxSSTS_IPM_MASK | AHCI_PxSSTS_DET_MASK))
        == (AHCI_PxSSTS_IPM_ACTIVE | AHCI_PxSSTS_DET_ESTABLISHED);
}

// ahciprobeport -- initializes a single port
void ahciprobeport(Arena *m, Console c, AhciDev dev, AhciControl *const ctrl, AhciPort *port,
                   const int portnum) {
  // devices that support stagerred spinup, need to be spun up.
  if (ctrl->capabilties & AHCI_CAP_SSS) {
    port->command_status |= AHCI_PxCMD_SUD;
  }
  if ((ctrl->capabilties & AHCI_CAP_SSS) || !(ctrl->ports_implemented & ((1 << (portnum - 1)) - 1))) {
    for (int i = 0; i<10; ++i) { cputc(c,'.');}
  }
  if (!ahciportisactive(port)) {
    // cprint(c, "port failed to activate");
    return;
  }
  cputc(c, '\n');

  ahciclearstatus(port, sata_error);
  ahciclearstatus(port, interrupt_status);

  ahcideviceinit(m, c, dev, ctrl, port, portnum);
}

// ahcipciinit -- Initialize a SATA controller and the devices attached to it.
AhciDev ahcipciinit(Arena *m, Console c, u8 bus, u8 slot, int* count) {
  count[0] = 0;
  PciConf conf = pciconfread(bus, slot);
  if (conf.class_code != 0x01 ||
      conf.subclass != 0x06) { // PCI class id 0x01 (Mass storage device) and
                               // subclass id 0x06 (serial ATA).
    return 0;
  }
  cprint(c, "ahci: Found SATA controller ");
  {
    const int dat[] = {bus, slot, conf.vendor_id, conf.device_id};
    for (int i = 0; i<4; ++i) {
      cprintint(c, dat[i], 16, 0);
      (i < 3) ? cputc(c, ' ') : cputc(c, '\n');
    }
  }

  AhciControl *const ctrl =
      (AhciControl *)(conf.dev.base_address_register[5].address);
  u32 size = conf.dev.base_address_register[5].size;
  AhciPort *const ports = ctrl->ports;

  // See HBA State machine above
  // Reset host controller
  // delay for up to one second, if we had a coorporative scheduler we would
  // yield here for one second (max). Instead we print some dots.
  ctrl->global_host_control |= AHCI_GHC_HOST_RESET;
  // delay for some time, if we had a scheduler at this point we would be able to yield for one second.
  for (int i = 0; i < 10; ++i) {
    cputc(c, '.');
  }
  cputc(c, '\n');
  if (ctrl->global_host_control & AHCI_GHC_HOST_RESET) {
    cprint(c, "ahci: Error, controller did not reset.\n");
    return 0;
  }

  if (ctrl->capabilties & AHCI_CAP_SAM) { // CAP.SAM == 0
    ctrl->global_host_control |= AHCI_GHC_AHCI_ENABLE;
  }

 if (ctrl->capabilties & AHCI_CAP_CCCS) { // Command coalescing
   ctrl->command_completion_coalescing_control |= 0x1;
   // TODO: Command coalescing features.
  }

  const int ncs = AHCI_NCS(ctrl->capabilties);
  // print some info
  {
    const char *speed_in_gbps;
    u32 speed, slots, ports;
    cprint(c, "registers at: "), cprintint(c, (u32)ctrl, 16, 0), cprint(c, " size: "), cprintint(c, size, 16, 0);
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
      count[0]++;
    }
  }
  AhciDev dev = arenapusharray(m, count[0], AhciDevDesc);
  // Probe for devices attached to the controller.
  for (int i = 0; i < 32; ++i) {
    if (ctrl->ports_implemented & (1 << i)) {
      ahciprobeport(m, c, &dev[i], ctrl, &ports[i], i+1);
    }
  }
  return dev;
}
