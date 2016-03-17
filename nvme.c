#include "u.h"
#include "dat.h"
#include "console.h"
#include "arena.h"
#include "pci.h"

// #define SubmissionQueueHead(y) (0x1000 + ((2*y + 1) * (4 << CAP.DSTRD)))
// #define SubmissionQueueTail(y) (0x1000 + (2*y * (4 << CAP.DSTRD)))

typedef enum {
        Capability = 0x0,
        Version = 0x8 >> 4,
        InterruptMaskSet = 0xc >> 4,
        InterruptMaskClear = 0x10 >> 4,
        Configuration = 0x14 >> 4,
        Status = 0x1C >> 4,
        AdminQueueAttributes = 0x24 >> 4,
        AdminSubmissionQueueBaseAddress = 0x28 >> 4,
        AdminCompletionQueueBaseAddress = 0x30 >> 4
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
        cprintint(c,(u64)mmio, 16, 0);
        return;
}
