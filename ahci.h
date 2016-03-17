typedef struct {
  u64 nextsector;
  u64 nextmemory;
  u16 readsectors;
} AhciBlockingRead;

typedef volatile struct AhciControl AhciControl;
typedef volatile struct AhciPort AhciPort;
typedef volatile struct AhciCommand AhciCommand;
typedef volatile struct AhciCommandTable AhciCommandTable;
typedef volatile struct ReceivedFis ReceivedFis;

void ahcipciinit(Arena *m, Console c, u8 bus, u8 slot);
AhciBlockingRead ahcireadblocking(AhciPort * const port, u64 src, u64 dst, u64 count);
