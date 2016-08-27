typedef struct ConsoleDesc {
  unsigned short pos;
  unsigned char color;
  unsigned short *buf;
} ConsoleDesc, *Console;

typedef struct {
  char  key[64];
  u64   value;
} Pair;

typedef struct ApciDesc {
  void *localapicaddr;
  void *ioapicaddr;
  const int maxcpucount;
  int cpucount;
  int cpuids[];
} AcpiDesc, *Acpi;
