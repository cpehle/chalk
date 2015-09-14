#include "u.h"
#include "delay.h"

#include "io.h"

void udelay(u64 usecs) {
        u64 i;
        for(i = 0; i < usecs; i++) {
                inb(0x80); // Port 0x80 is used by the BIOS to report errors while booting, so it is harmless to read it and it introduces some delay.
        }
}
