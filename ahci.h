typedef volatile struct {
  u64 commandlist_base_addr;
  u64 frameinfo_base_addr;
  u32 interrupt_status;
  u32 interrupt_enable;
  u32 command_status;
  u32 _reserved0;
  u32 taskfile_data;
  u32 signature;
  u32 sata_status;
  u32 sata_control;
  u32 sata_error;
  u32 sata_active;
  u32 command_issue;
  u32 sata_notify;
  u32 fis_based_switch;
  u32 _reserved1[11];
  u32 _vendor[4];
} AhciPort;

// Host Bus Adapter Control (HBAControl)
typedef volatile struct {
  u32 capabilties;
  u32 global_host_control;
  u32 interrupt_status;
  u32 ports_implemented;
  u32 version;
  u32 command_completion_coalescing_control;
  u32 command_completion_coalescing_ports;
  u32 enclosure_management_location;
  u32 enclosure_management_control;
  u32 extended_capabilities;
  u32 handoff_control_status;
  u32 _reserved0[29];
  u32 _vendor[24];
  AhciPort ports[32];
} AhciControl;


// The host issues commands to the device through a command list,
// it has up to 32 slots, each of which can hold a command header.
// The command headers describe ATA or ATAPI commands.

typedef enum {
    AHCI_CMD_LIST_FLAG_CFL = (1 << 0),
    AHCI_CMD_LIST_FLAG_A   = (1 << 5), // ATAPI
    AHCI_CMD_LIST_FLAG_W   = (1 << 6), // Write
    AHCI_CMD_LIST_FLAG_P   = (1 << 7), // Prefetchable
    AHCI_CMD_LIST_FLAG_R   = (1 << 8), // Reset
    AHCI_CMD_LIST_FLAG_B   = (1 << 9), // BIST
} AhciCommandListFlag;


typedef struct AhciCommandHeader
{
    u16 flags;
    u16	prdtl;		// Physical region descriptor table length in entrie
    volatile u32 prdbc;		// Physical region descriptor byte count transferred
    u64	ctba;		// Command table descriptor base address
    DWORD	rsv1[4];	// Reserved
} AhciCommandHeader __attribute__(packed);



typedef volatile struct {
  u8 fis_raw[64];
  u8 atapi_cmd[16];
  u8 _reserved0[48];
  struct {
    u64 data_base_addr;
    u32 _reserved0;
    u32 flags;
  } prdt[8]; /* Can be up to 65,535 prds,
                but implementation needs multiple of 128 bytes. */
} AhciCommandTable;


// Frame Information Structure types (FISType)
typedef enum {
  FIS_TYPE_REG_H2D = 0x27,   // Register FIS - host to device
  FIS_TYPE_REG_D2H = 0x34,   // Register FIS - device to host
  FIS_TYPE_DMA_ACT = 0x39,   // DMA activate FIS - device to host
  FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
  FIS_TYPE_DATA = 0x46,      // Data FIS - bidirectional
  FIS_TYPE_BIST = 0x58,      // BIST activate FIS - bidirectional
  FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
  FIS_TYPE_DEV_BITS = 0xA1,  // Set device bits FIS - device to host
} FISType;

typedef struct FisData {
  u8 fis_type;             // FIS_TYPE_DATA
  u8 port_multiplier_port; // actually only the first 4 bits.
  u8 reserver0[2];
  u32 data[1]; // Payload
} FISData __attribute__(packed);

typedef struc FISRegD2H {
  u8 fis_type; // FIS_TYPE_REG_D2H
  u8 port_multiplier_interrupt;
  u8 status; // Status register
  u8 error;  // Error register
  u64 lba;
  u16 count;
  u8 reserved[6];
}
FISRegD2H __attribute__(packed);

typedef struct FISRegH2D {
  u8 fis_type; // FIS_TYPE_REG_H2D
  u8 pmp_c;    // first 4 bits port multiplier, last bit 1:Command 0:Control.
  u8 command;  // Command register
  u8 featurel; // Feature register, 7:0
  u8 lba0;     // LBA low register, 7:0
  u8 lba1;     // LBA mid register, 15:8
  u8 lba2;     // LBA high register, 23:16
  u8 device;   // Device register
  u8 lba3;     // LBA register, 31:24
  u8 lba4;     // LBA register, 39:32
  u8 lba5;     // LBA register, 47:40
  u8 featureh; // Feature register, 15:8
  u16 count;
  u8 icc;     // Isochronous command completion
  u8 control; // Control register
  u8 rsv1[4]; // Reserved
} FISRegH2D __attribute__(packed);

typedef struct FISPioSetup {
  u8 fis_type; // FIS_TYPE_PIO_SETUP
  u32 pmp_d_i;
  u8 status; // Status register
  u8 error;  // Error register
  u64 lba;
  u16 count;
  u8 rsv3;     // Reserved
  u8 e_status; // New value of status register
  u16 tc;      // Transfer count
  u8 rsv4[2];  // Reserved
} FISPioSetup __attribute__(packed);

typedef struct FisDMASetup {
  // DWORD 0
  u8 fis_type; // FIS_TYPE_DMA_SETUP
  u8 pmp_d_i_a;
  u8 rsved[2]; // Reserved
  // u32 1&2
  u64 DMAbufferID; // DMA Buffer Identifier. Used to Identify DMA buffer in host
                   // memory. SATA Spec says host specific and not in Spec.
                   // Trying AHCI spec might work.
  // u32 3
  u32 rsvd; // More reserved
  // u32 4
  u32 DMAbufOffset; // Byte offset into buffer. First 2 bits must be 0
  // u32 5
  u32 TransferCount; // Number of bytes to transfer. Bit 0 must be 0
  // u32 6
  u32 resvd; // Reserved
} FISDMASetup;
