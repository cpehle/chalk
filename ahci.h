
typedef struct {
  u64 nextsector;
  u64 nextmemory;
  u16 readsectors;
} AhciBlockingRead;

typedef struct AhciControl AhciControl;
typedef struct AhciPort AhciPort;
typedef struct AhciCommand AhciCommand;
typedef struct AhciCommandTable AhciCommandTable;
typedef struct ReceivedFis ReceivedFis;
typedef struct AhciDevDesc {
        AhciControl *control;
        AhciPort *port;
        AhciCommand *commandlist;
        AhciCommandTable *commandtable;
        ReceivedFis *receivedfis;
        u8 *buf, *userbuffer;
        int writeback;
        u64 bufferlength;
        int identify;
        int readsectors;
} AhciDevDesc;
typedef AhciDevDesc* AhciDev;

AhciDev ahcipciinit(Arena *m, Console c, u8 bus, u8 slot, int* count);
AhciBlockingRead ahcireadblocking(AhciDev dev, u64 src, u64 dest, u64 count);
