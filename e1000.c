#include "u.h"
#include "io.h"
#include "mem.h"
#include "arena.h"
#include "dat.h"
#include "console.h"
#include "pci.h"
#include "relptr.h"
#include "e1000.h"

// Transmission commands
typedef enum {
  TxCmdEop = 1 << 0, // end of packet
  TxCmdIfcs = 1 << 1, // insert fcs
  TxCmdIc = 1 << 2, // insert checksum
  TxCmdRs = 1 << 3, // report status
  TxCmdRps = 1 << 4, // report packet send
  TxCmdVle = 1 << 6, // vlan packet enable
  TxCmdIde = 1 << 7 // interrupt delay enable
} TxCmd;

typedef enum {
  Rctrl = 0x0000, // device control
  Reerd = 0x0014, // eeprom read
  Ricr = 0x00c0,  // Interrupt cause read
  Rims = 0x00d0,  // Interrupt mask set/read

  Rrctl = 0x0100, // receive control
  Rtctl = 0x0400, // transmit control

  Rrdbal = 0x2800, // receive descriptor base low
  Rrdbah = 0x2804, // receive descriptor base high
  Rrdlen = 0x2808, // receive descriptor length
  Rrdh   = 0x2810, // receive descriptor head
  Rrdt   = 0x2818, // receive descriptor tail

  Rtdbal = 0x3800, // transmit descriptor base low
  Rtdbah = 0x3804, // transmit descriptor base high
  Rtdlen = 0x3808, // transmit descriptor length
  Rtdh   = 0x3810, // transmit descriptor head
  Rtdt   = 0x3818, // transmit descriptor tail


  Rmta = 0x5200, // multicast table array
  Rral = 0x5400, // receive address low
  Rrah = 0x5404  // receive address high
} E1000Ctrl;

// Receive control register bits
enum {
  Rctl_EN = (1 << 1),            // Receiver Enable
  Rctl_SBP = (1 << 2),           // Store Bad Packets
  Rctl_UPE = (1 << 3),           // Unicast Promiscuous Enabled
  Rctl_MPE = (1 << 4),           // Multicast Promiscuous Enabled
  Rctl_LPE = (1 << 5),           // Long Packet Reception Enable
  Rctl_LBM_NONE = (0 << 6),      // No Loopback
  Rctl_LBM_PHY = (3 << 6),       // PHY or external SerDesc loopback
  RTCL_RDMTS_HALF = (0 << 8),    // Free Buffer Threshold is 1/2 of RDLEN
  RTCL_RDMTS_QUARTER = (1 << 8), // Free Buffer Threshold is 1/4 of RDLEN
  RTCL_RDMTS_EIGHTH = (2 << 8),  // Free Buffer Threshold is 1/8 of RDLEN
  Rctl_MO_36 = (0 << 12),        // Multicast Offset - bits 47:36
  Rctl_MO_35 = (1 << 12),        // Multicast Offset - bits 46:35
  Rctl_MO_34 = (2 << 12),        // Multicast Offset - bits 45:34
  Rctl_MO_32 = (3 << 12),        // Multicast Offset - bits 43:32
  Rctl_BAM = (1 << 15),          // Broadcast Accept Mode
  Rctl_VFE = (1 << 18),          // VLAN Filter Enable
  Rctl_CFIEN = (1 << 19),        // Canonical Form Indicator Enable
  Rctl_CFI = (1 << 20),          // Canonical Form Indicator Bit Value
  Rctl_DPF = (1 << 22),          // Discard Pause Frames
  Rctl_PMCF = (1 << 23),         // Pass MAC Control Frames
  Rctl_SECRC = (1 << 26),        // Strip Ethernet CRC
};

#define Rctl_BSIZE_256                  (3 << 16)
#define Rctl_BSIZE_512                  (2 << 16)
#define Rctl_BSIZE_1024                 (1 << 16)
#define Rctl_BSIZE_2048                 (0 << 16)
#define Rctl_BSIZE_4096                 ((3 << 16) | (1 << 25))
#define Rctl_BSIZE_8192                 ((2 << 16) | (1 << 25))
#define Rctl_BSIZE_16384                ((1 << 16) | (1 << 25))

// Transmission control register bits
enum {
        Tctl_EN =                         (1 << 1),    // Transmit Enable
        Tctl_PSP =                        (1 << 3),    // Pad Short Packets
        Tctl_CT_SHIFT                   = 4        ,   // Collision Threshold
        Tctl_COLD_SHIFT =                 12       ,   // Collision Distance
        Tctl_SWXOFF =                     (1 << 22),   // Software XOFF Transmission
        Tctl_RTLC =                       (1 << 24),   // Re-transmit on Late Collision
};

// Device Control register bits
enum {
  CtrlSLU = 1 << 6, // set link up
};

typedef struct RecvDesc {
  volatile u64 addr;
  volatile u16 len;
  volatile u16 checksum;
  volatile u8 status;
  volatile u8 errors;
  volatile u16 special;
} __attribute__((packed)) RecvDesc;

typedef struct TransDesc {
  volatile u64 addr;
  volatile u16 len;
  volatile u8 cso;
  volatile u8 cmd;
  volatile u8 status;
  volatile u8 css;
  volatile u16 special;
} __attribute__((packed)) TransDesc;

static void e1000poll() {

}

typedef struct Ethdevdesc {
  int txwrite;
  int txdesccount;
  u32* mmioaddr;
} Ethdevdesc;
typedef Ethdevdesc* Ethdev;

typedef struct Ethbufdesc {
  u64 start;
  u64 end;
} Ethbufdesc;
typedef Ethbufdesc* Ethbuf;


static void e1000receive(Ethdev dev, Ethbuf buf) {
  RecvDesc *r;
  u8* mmio;
  r->addr = buf->start;
  r->len = buf->end - buf->start;
}

static void e1000send(Ethdev dev, Ethbuf buf) {
  TransDesc *t;
  u8* mmio;
  while(!(t->status & 0xf)) {
    // wait
  }
  t->addr = buf->start;
  t->len  = buf->end - buf->start;
  t->cmd  = TxCmdEop | TxCmdIfcs | TxCmdRs;
  t->status = 0;
  dev->txwrite = (dev->txwrite + 1) & (dev->txdesccount - 1);
  mmiowrite32(dev->mmioaddr + Rtdt, dev->txwrite);
}


void e1000init(Arena *a, PciConf *c, Console cons) {
  Ethdevdesc ethdev;
  if ((c->vendor_id != 0x8086) ||
      !(c->device_id == 0x100e || c->device_id == 0x1503)) {
    return;
  }
  PciDevConf d = c->dev;
  volatile u32* mmio = d.base_address_register[0].address;
  ethdev.mmioaddr = mmio;
  cprintint(cons, (u32)mmio, 16, 0);

  RelativePointer rctrl = { .base = mmio, .offset = Rctrl};
  RelativePointer rdbal = { mmio, Rrdbal};

  u8 mac[6] = {};
  // TODO: Should determine number of receive and transmission registers.
  const int Ntx = 32, Nrx = 8;
  ethdev.txdesccount = Ntx;
  u32 ral = mmio[Rral/4];
  if (ral) {
    u32 rah = mmio[Rrah/4];
    mac[0] = (u8)ral;
    mac[1] = (u8)(ral >> 8);
    mac[2] = (u8)(ral >> 16);
    mac[3] = (u8)(ral >> 24);
    mac[4] = (u8)(rah);
    mac[5] = (u8)(rah >> 8);
  } else { // read MAC address from EEPROM instead
    return;
  }
  cprint(cons, "\nMAC:");
  for (int i = 0; i < 6; ++i) {
    cprintint(cons, mac[i], 16, 0),
        (i == 5) ? cputc(cons, '\n') : cputc(cons, ':');
  }

  mmio[Rctrl / 4] |= CtrlSLU;  // set link up
  mset(mmio + Rmta/4, 0, 4 * 128); // clear multicast table
  mmioread32(mmio + Ricr/4); // clear all interrupts

  RecvDesc *rx = arenapusharray(a, Nrx, RecvDesc);
  for (int i = 0; i < Nrx; ++i) {
    rx[i].addr = (u64)arenapusharray(a, 2048, u8);
    rx[i].status = 0;
  }
  mmio[Rrdbal / 4] = (u64)rx;
  mmio[Rrdbah / 4] = (u64)rx >> 32;
  mmio[Rrdlen / 4] = Nrx * sizeof(RecvDesc);
  mmio[Rrdh / 4] = 0;
  mmio[Rrdt / 4] = Nrx - 1;
  mmio[Rrctl / 4] = Rctl_EN | Rctl_SBP | Rctl_UPE | Rctl_MPE | Rctl_LBM_NONE |
                     RTCL_RDMTS_HALF | Rctl_BAM | Rctl_SECRC | Rctl_BSIZE_2048;

  TransDesc *tx = arenapusharray(a, Ntx, TransDesc);
  mset(tx,0,Ntx*sizeof(TransDesc));
  for (int i = 0; i<Ntx; ++i) {
    tx[i].status = (1<<0); // mark descriptor complete
  }
  mmio[Rtdbal / 4] = (u64)tx;
  mmio[Rtdbah / 4] = (u64)tx >> 32;
  mmio[Rtdlen / 4] = Ntx * sizeof(TransDesc);
  mmio[Rtdh / 4] = 0;
  mmio[Rtdt / 4] = 0;
  mmio[Rtctl / 4] = Tctl_EN | Tctl_PSP | (15 < Tctl_CT_SHIFT) | (64 << Tctl_COLD_SHIFT) | Tctl_RTLC;


  return;
  // need to setup the transmission and receive ringbuffers
}
