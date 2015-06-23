
typedef struct PciDev {
        Dev dev;
} PciDev;

typedef struct PciBridgeConf {
        u32 base_address_register[2];

        u8 secondary_latency_timer;
        u8 subordinate_bus_number;
        u8 secondary_bus_number;
        u8 primary_bus_number;

        u16 secondary_status;
        u8 io_limit;
        u8 io_base;

        u16 memory_limit;
        u16 memory_base;

        // the next four values should probably be merged.
        u16 prefetchable_memory_limit;
        u16 prefetchable_memory_base;

        u16 prefetchable_base_upper;
        u16 prefetchable_limit_upper;
        // and those with io_limit, io_base
        u8 io_limit_upper;
        u8 io_base_upper;

        u8 capability_ptr;

        u32 expansion_rom_address;
        u16 bridge_control;
        u8 interrupt_pin;
        u8 interrupt_line;
} PciBridgeConf;

typedef struct PciDevConf {
        u32 base_address_register[6];

        u32 cardbus_cis_ptr;

        u16 subsystem;
        u16 subsystem_vendor;

        u32 expansion_rom_base_address;

        u8 capability_ptr;

        u8 max_latency;
        u8 min_grant;
        u8 interrupt_pin;
        u8 interrupt_line;
} PciDevConf;

typedef struct PciConf {
        u16 device_id;
        u16 vendor_id;

        u16 status_reg;
        u16 command_reg;

        u8 class_code;
        u8 subclass;
        u8 prog_if;
        u8 revision_id;
        // BIST : Build in self test.
        // BIST Capable | Start BIST | Reserved Completion | Code
        // Bit  7       | 6          | 5-4                 | 3-0
        u8 bist_register;
        u8 header_type;
        u8 latency_timer;
        u8 cache_line_size;
        union {
                PciBridgeConf bridge;
                PciDevConf dev;
        };
} PciConf;

PciConf pci_read_conf();
void pci_scan(Console c);
