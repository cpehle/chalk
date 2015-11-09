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

typedef enum {
        NVME_ADM_CMD_DELETE_SQ,
        NVME_ADM_CMD_CREATE_SQ,
        NVME_ADM_CMD_DELETE_CQ,
        NVME_ADM_CMD_CREATE_CQ,
        NVME_ADM_CMD_IDENITFY,
        NVME_ADM_CMD_SET_FEATURES,
        NVME_ADM_CMD_GET_FEATURES
} AdminCommand;


void nvmemmiowrite() {

}

u64 nvmemmioread() {

}

void nvmepciinit(Arena*m, Console c, PciConf conf) {
        volatile u64* mmio =  conf.dev.base_address_register[0].address;
        return;
}
