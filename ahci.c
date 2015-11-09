#include "u.h"
#include "io.h"
#include "dat.h"
#include "pci.h"
#include "arena.h"
#include "console.h"
#include "mem.h"
#include "delay.h"

#define BITEXTRACT(c,x,y) ((c & (((1 << x) - 1) << y)) >> y)

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

typedef enum
{
    FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
    FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
    FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
    FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
    FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

enum {
  SATA_SIG_ATA = 0x00000101,	// SATA drive
  SATA_SIG_ATAPI = 0xEB140101,	// SATAPI drive
  SATA_SIG_SEMB = 0xC33C0101,	// Enclosure management bridge
  SATA_SIG_PM  = 0x96690101	// Port multiplier
};

typedef struct {
  u64 commandlistbaseaddress;
  u64 frameinfobase;
  u32 interruptstatus;
  u32 interruptenable;
  u32 commandstatus;
  u32 _r0;
  u32 taskfiledata;
  u32 signature;
  u32 satastatus;
  u32 satacontrol;
  u32 sataerror;
  u32 sataactive;
  u32 commandissue;
  u32 satanotify;
  u32 fisbasedswitch;
  u32 _r1[11];
  u32 _vendor[4];
} __attribute__((packed)) Ahciport;

typedef struct  {
  u32 capabilities;
  u32 globalhostcontrol;
  u32 interruptstatus;
  u32 portsimplemented;
  u32 version;
  u32 ccccontrol;
  u32 cccports;
  u32 emlocation;
  u32 emcontrol;
  u32 extendedcapabilities;
  u32 handoffcontrolstatus;
  u32 _r0[13];
  u32 _nvmchi[16];
  u32 _vendor[24];
  Ahciport port[32];
} __attribute__((packed)) Ahcihostbusadapter;

typedef struct {
  u16 command;
  u16 prdtlength;
  u32 prdtbytes;
  u64 commandtablebase;
  u8  _r0[16];
} __attribute__((packed)) Ahcicommand;

typedef volatile struct {
    u8 fis[64];
    u8 atapicommand[16];
    u8 _r0[48];
    struct {
        u64 base;
        u32 _reserved0;
        u32 flags;
    } prdt[65535];
} __attribute__((packed)) Ahcicommandtable;

typedef volatile struct {
    u8 dmasetupfis[28];
    u8 _r0[4];
    u8 piosetupfis[20];
    u8 _r1[12];
    u8 d2hregisterfis[20];
    u8 _r2[4];
    u8 setdevicebitsfis[8];
    u8 unknownfis[64];
    u8 _r3[96];
} __attribute__((packed)) Ahcireceivedfis;

typedef struct Ahcidevice {
  Ahciport * port;
  Ahcicommand * commandlist;
  Ahcicommandtable* commandtable;
} Ahcidevice;

static void ahcisetuptransfer() {

}

static inline u32 _ahciclearstatus(volatile u32 *const reg)
{
    const u32 bits = *reg;
    if (bits)
        *reg = bits;
    return bits;
}
#define ahciclearstatus(p, r) _ahciclearstatus(&(p)->r)

int ahcistartcommandengine(Ahciport *const p) {
  int timeout = 1000;
  while ((p->commandstatus & (1 << 15)) && timeout--) {
    udelay(1);
  }
  if (timeout < 0) {
    return 1;
  }
  p->commandstatus |= (1 << 4); // FIS Receive Enable
  p->commandstatus |= (1 << 0); // Start command processing
  return 0;
}

int ahcistopcommandengine(Ahciport *const p) {
  int timeout = 1000;
  p->commandstatus &= ~(1 << 0);
  while ((p->commandstatus & (1 << 15)) && timeout--) {
    udelay(1);
  }
  if (timeout < 0) {
    return 1;
  }
  p->commandstatus &= ~(1 << 4);
  timeout = 10000;
  while ((p->commandstatus & (1 << 4)) && timeout--) {
    udelay(1);
  }
  if (timeout < 0) {
    return 1;
  }
  return 0;
}

void ahcicommandslotprepare(Ahcidevice *const d) {
  const int BYTES_PER_PRD = 512;
  const int BYTES_PER_PRD_SHIFT = 9;
  int buf_len = 13;
  int length = 0;
  int k = 0;
  int slotnum = 0;
  d->commandlist[slotnum].command = 0;
  d->commandlist[slotnum].commandtablebase = (u64)d->commandtable;

  if (length > 0) {
    u64 prdtlength = ((buf_len - 1) >> BYTES_PER_PRD_SHIFT) + 1;
    u8 *buf;
    d->commandlist[slotnum].prdtlength = prdtlength;
    for (u64 i = 0; i < prdtlength; ++i) {
      const u64 bytes = (buf_len < BYTES_PER_PRD) ? buf_len : BYTES_PER_PRD;
      d->commandtable[k].prdt[i].base = buf;
      d->commandtable[k].prdt[i].flags = PRDTABLEBYTES(bytes);
      buf += bytes;
    }
  }
}

size_t ahcicommandslotexecute(Ahcidevice *const d) {
  const int slotnumber = 0;

  d->port->commandissue |= (1 << slotnumber);
  int timeout = 10000;
  while ((d->port->commandissue) & (1 << slotnumber) &&
         !(d->port->interruptstatus & (1 << 30))
           && timeout--) {
    udelay(100);
  }
  if (timeout < 0) {
    return -1;
  }
}

void ahciidentifydevice(Ahcidevice *const d) {
  int i = 0;
  ahcicommandslotprepare(d);
  mset(&d->commandtable[i], 0, sizeof(Ahcicommandtable));
  d->commandtable[i].fis[0] = 0x27; // Fis Host to Device
  d->commandtable[i].fis[1] = 0x80; // Host to Device Command
  d->commandtable[i].fis[2] = 0;
  ahcicommandslotexecute(d);
}



static inline int ahciportisactive(const Ahciport *const port)
{
  return (port->satastatus & ((0xf << 8) | (0xf << 0)))
    == ((1 << 8) | (3 << 0));
}


// ahcipciinit -- Initialize a SATA controller and the devices attached to it
void ahcipciinit(Arena *m, Console c, u8 bus, u8 slot) {
  PciConf conf = pciconfread(bus, slot);
  u32 portcount;
  u32 commandslotcount;
  volatile u32 reg;
  //
  if (conf.class_code != 0x01 ||
      conf.subclass != 0x06) {
    return;
  }
  cprint(c, "pci : Found SATA controller\n");
  // The address of the AHCI Host Bus Adapter is located at the base address register 5.
  // NOTE(Christian): Aparently there are exceptions to this rule.
  // See for example the linux or openbsd code.
  volatile Ahcihostbusadapter* const h = conf.dev.base_address_register[5].address;
  {
    if (h->extendedcapabilities & (1 << 0)) {
      if (!(h->handoffcontrolstatus & (1 << 0))) {
        cprint(c, "ahci: AHCI is owned by the BIOS, we will try to obtain control\n");
      } else {
        cprint(c, "ahci: AHCI is owned by us\n");
      }
    }
  }

  #if 0
  // Reset HBA
  {
    h->globalhostcontrol |= (1 << 0); // HBA_GHC_RESET
    for (int i = 0; i < 100; ++i) {
      udelay(1);
    }
    if (!(h->globalhostcontrol & (1 << 0))) {
      cprint(c, "ahci: Failed to reset HBA\n");
      return;
    }
  }
  #endif

  !(h->capabilities & (1 << 18)) ? h->globalhostcontrol |= (1<<31) : 0; // HBA_GHC_AHCI_ENABLE
  reg = h->globalhostcontrol;
  if (!(reg & (1<<31))) { return; }

  reg = h->capabilities;
  {
    commandslotcount = BITEXTRACT(reg,5,8) + 1;
    portcount = BITEXTRACT(reg,5,0) + 1;
    Pair p[] = {{"Command slot count", commandslotcount}, {"portcount", portcount}};
    cprint(c, "ahci: "), cprintpairs(c, p, 2), cnl(c);
  }

  for (int i = 0; i < 32; ++i) {
    if (!(h->portsimplemented & (1 << i))) {
      continue;
    } else {
      Ahciport* const p = &h->port[i];
      if (h->capabilities & (1 << 27)) { // SSS -- Staggered spinup supported
        cprint(c, "ahci: spinning up device...");
        p->commandstatus |= (1 << 1); // Spin up device
      }
      ahciclearstatus(p, sataerror);
      ahciclearstatus(p, interruptstatus);
      const int ncs = 20;
      Ahcidevice *const d = arenapushstruct(m, Ahcidevice);
      Ahcicommand *const cl = arenapusharrayalign(m, ncs, Ahcicommand, 1024);
      Ahcicommandtable *const t = arenapusharrayalign(m, ncs, Ahcicommandtable, 128);
      Ahcireceivedfis *const rf = arenapushstructalign(m, Ahcireceivedfis, 256);
      d->port = p;
      d->commandlist = cl;
      d->commandtable = t;
      if (ahcistopcommandengine(p)) {
        cprint(c, "ahci: failed to stop command engine\n");
        continue;
      }
      p->commandlistbaseaddress = (u64)cl;
      p->frameinfobase = (u64)rf;
      if (ahcistartcommandengine(p)) {
        cprint(c, "ahci: failed to start command engine\n");
        continue;
      }
      p->commandstatus |= (1 << 28); // ICC Active
      int timeout = 10000;
      while ((p->taskfiledata & ((1 << 7))) && timeout--) { // Taskfiledata busy
        udelay(1);
      }
      if (p->taskfiledata & (1 << 7)) {
        cprint(c, "ahci: device failed to spin up in time\n");
        continue;
      }
      switch (p->signature) {
      case SATA_SIG_ATA:
        cprint(c, "ahci: found SATA device on port "), cprintint(c, i, 16, 0), cnl(c);
        break;
      default:
        break;
      }

    }
  }
}
