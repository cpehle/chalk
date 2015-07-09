typedef struct Region Region;
struct Region {
  long lwb;
  long upb;
  char permissions;
  int turfID;
  int threadID;
};

typedef struct ConsoleDesc {
  unsigned short pos;
  unsigned char color;
  unsigned short *buf;
} ConsoleDesc;
typedef ConsoleDesc *Console;


typedef struct ApciDesc {
  Console c;
  void *localapicaddr;
  void *ioapicaddr;
  const int maxcpucount;
  int cpucount;
  int cpuids[];
} AcpiDesc;
typedef AcpiDesc *Acpi;



typedef struct Dev {
  char *name;
} Dev;

/*
struct Dev {
  int dc;
  char *name;

  void (*reset)(void);
  void (*init)(void);
  void (*shutdown)(void);
  Chan *(*attach)(char *);
  Walkqid *(*walk)(Chan *, Chan *, char **, int);
  int (*stat)(Chan *, uchar *, int);
  Chan *(*open)(Chan *, int);
  void (*create)(Chan *, char *, int, ulong);
  void (*close)(Chan *);
  long (*read)(Chan *, void *, long, vlong);
  Block *(*bread)(Chan *, long, ulong);
  long (*write)(Chan *, void *, long, vlong);
  long (*bwrite)(Chan *, Block *, ulong);
  void (*remove)(Chan *);
  int (*wstat)(Chan *, uchar *, int);
  void (*power)(int);
  int (*config)(int, char *, DevConf *);
  };
*/
