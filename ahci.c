#include "u.h"
#include "dat.h"
#include "pci.h"
#include "ahci.h"

//PCI class id 0x01(Mass storage device) and subclass id 0x06 (serial ATA)
//

// Logic block addressing (LBA)

enum {
        HBA_PxCMD_ST = (1 << 0),
        HBA_PxCMD_FRE = (1 << 4),
        HBA_PxCMD_FR = (1 << 14),
        HBA_PxCMD_CR = (1 << 15),
};






/* // */
/* static ahci_read_sectors(AhciPort* p, const Lba start, size_t count, u8 *const buffer) { */

/*         AhciCommandTable *tbl = (AhciCommandTable *)p->commandlist_base_addr; */

/*         FisData fis = {}; */


/* } */

// TODO: Timeout? Need to figure out how to handle errors.
static void ahci_start_command_engine(AhciPort* p) {
        while (p->command_status & HBA_PxCMD_CR);
        p->command_status |= HBA_PxCMD_FRE;
        p->command_status |= HBA_PxCMD_ST;
}
// TODO: Need to figure out, if we want to time out after a while.
static void ahci_stop_command_engine(AhciPort* p) {
        p->command_status &= ~HBA_PxCMD_ST;
        while(1) {
                if (p->command_status & HBA_PxCMD_FR)
                        continue;
                if (p->command_status & HBA_PxCMD_CR)
                        continue;
                break;
        }
        p->command_status &= ~HBA_PxCMD_FRE;
}


void ahci_port_probe(AhciControl *const ctrl, AhciPort *const port, const int portnum) {


        ahci_device_init(ctrl, port, portnum);
}



int ahci_pci_init(PciDev* d) {

        PciConf conf = pci_read_conf();
        AhciControl *const ctrl = (AhciControl *)(conf.dev.base_address_register[5]);
        AhciPort *const ports = ctrl->ports;
        // reset the controller.
        ctrl->global_host_control |= HBA_CTRL_RESET;
        // wait for finish.
        ctrl->global_host_control |= HBA_CTRL_AHCI_EN;
        for (int i = 0; i<32; ++i) {
                if (ctrl->ports_implemented & (1 << i)) {
                        ahci_port_probe(ctrl, &ports[i], i+1);
                }
        }
}
