#include "u.h"
#include "io.h"
#include "dat.h"
#include "pci.h"
#include "arena.h"
#include "console.h"
#include "mem.h"

#define BITEXTRACT(c,x,y) ((c & (((1 << x) - 1) << y)) >> y)

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
  Ahcicommandtable * commandtable;
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

  }
  if (timeout < 0) {
    return 1;
  }
  p->commandstatus &= ~(1 << 4);
  timeout = 10000;
  while ((p->commandstatus & (1 << 4)) && timeout--) {
  }
  if (timeout < 0) {
    return 1;
  }
  return 0;
}

void ahcicommandslotprepare(Ahcidevice *const d) {
  const int BYTES_PER_PRD = 512;
  int buf_len = 13;
  int length = 0;
  int k = 0;
  int slotnum = 0;
  d->commandlist[slotnum].command = 0;
  d->commandlist[slotnum].commandtablebase = (u64)d->commandtable;

  if (length > 0) {
    u64 prdtlength;
    u8 *buf;
    d->commandlist[slotnum].prdtlength = prdtlength;
    for (int i = 0; i < prdtlength; ++i) {
      const u64 bytes = (buf_len < BYTES_PER_PRD) ? buf_len : BYTES_PER_PRD;
      d->commandtable[k].prdt[i].base = buf;
      d->commandtable[k].prdt[i].flags = PRDTABLEBYTES(bytes);
      buf += bytes;
    }
  }
}

void ahcicommandslotexecute(Ahcidevice *const d) {

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

  if (conf.class_code != 0x01 ||
      conf.subclass != 0x06) {
    return;
  }
  cprint(c, "pci: found SATA controller.\n");
  // The address of the AHCI Host Bus Adapter is located at the base address register 5.
  volatile Ahcihostbusadapter* const h = conf.dev.base_address_register[5].address;

  {
    if (h->extendedcapabilities & (1 << 0)) {
      if (!(h->handoffcontrolstatus & (1 << 0))) {
        cprint(c, "ahci: AHCI is owned by the BIOS, will try to obtain control.\n");
      } else {
        cprint(c, "ahci: AHCI is owned by us.\n");
      }
    }
  }





  #if 0
  // Reset HBA
  {
    h->globalhostcontrol |= (1 << 0); // HBA_GHC_RESET
    // TODO: We need some timeout mechanism instead.
    for (int i = 0; i < 100; ++i) { cputc(c, '.');}
    if (!(h->globalhostcontrol & (1 << 0))) {
      cprint(c, "ahci: Failed to reset HBA.\n");
      return;
    }
  }
  #endif

  !(h->capabilities & (1 << 18)) ? h->globalhostcontrol |= (1<<31) : 0; // HBA_GHC_AHCI_ENABLE
  reg = h->globalhostcontrol;
  if (!(reg & (1<<31))) { return; }

  reg = h->capabilities;
  {

    commandslotcount = BITEXTRACT(reg,5,8);
    portcount = BITEXTRACT(reg,5,0);
    Pair p[] = {{"command slot count", commandslotcount},
                {"portcount", portcount}};
    cprint(c, "ahci: "), cprintpairs(c, p, 2), cnl(c);
  }


  for (int i = 0; i < 32; ++i) {
    if (!(h->portsimplemented & (1 << i))) {
      continue;
    } else {
      Ahciport* const p = &h->port[i];
      if (h->capabilities & (1 << 27)) { // SSS -- Staggered spinup supported
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
        cprint(c, "ahci: failed to stop command engine.");
        continue;
      }
      p->commandlistbaseaddress = (u64)cl;
      p->frameinfobase = (u64)rf;
      if (ahcistartcommandengine(p)) {
        cprint(c, "ahci: failed to start command engine.");
        continue;
      }
      p->commandstatus |= (1 << 28); // ICC Active
      int timeout = 10000;
      while ((p->taskfiledata & ((1 << 7))) && timeout--) { // Taskfiledata busy
        cputc(c, '.');
      }
      if (p->taskfiledata & (1 << 7)) {
        cprint(c, "ahci: device failed to spin up in time\n");
        continue;
      }
      switch (p->signature) {
      default:
        break;
      }

    }
  }
}
