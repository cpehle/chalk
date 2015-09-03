
#include "u.h"
#include "dat.h"
#include "console.h"
#include "arena.h"
#include "pci.h"

typedef enum {
        Cap = 0x0,
        Vs = 0x8 >> 4,
        Intms = 0xc >> 4,
        Intmc = 0x10 >> 4,
        Cc = 0x14 >> 4,
} Register;




void nvmepciinit(Arena*m, Console c, u8 bus, u8 slot) {
        PciConf conf = pciconfread(bus, slot);
        volatile u64* mmio =  conf.dev.base_address_register[0].address;

        Pair p[] = {{"Hello", 12}, {"Test", 123}};
        cprintpairs(c, p, 2);
        return;
}
