#include "u.h"
#include "io.h"

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF
#define VBE_DISPI_INDEX_ID              0x0
#define VBE_DISPI_INDEX_XRES            0x1
#define VBE_DISPI_INDEX_YRES            0x2
#define VBE_DISPI_INDEX_BPP             0x3
#define VBE_DISPI_INDEX_ENABLE          0x4
#define VBE_DISPI_INDEX_BANK            0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH      0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT     0x7
#define VBE_DISPI_INDEX_X_OFFSET        0x8
#define VBE_DISPI_INDEX_Y_OFFSET        0x9

#define VBE_DISPI_DISABLED              0x00
#define VBE_DISPI_ENABLED               0x01
#define VBE_DISPI_GETCAPS               0x02
#define VBE_DISPI_8BIT_DAC              0x20
#define VBE_DISPI_LFB_ENABLED           0x40
#define VBE_DISPI_NOCLEARMEM            0x80

struct Vbeinfoblock {
    // VBE 1.x fields
    u32		signature;
    struct {
        u8	minor;
        u8	major;
    } version;
    u32		oem_string;
    u32		capabilities;
    u32		mode_list;
    u16		total_memory;	// in 64k blocks
    // VBE 2.0+ fields only
    // Note, the block is 256 bytes in size for VBE 1.x as well,
    // but doesn't define these fields. VBE 3 doesn't define
    // any additional fields.
    u16		oem_software_revision;
    u32		oem_vendor_name_string;
    u32		oem_product_name_string;
    u32		oem_product_revision_string;
    u8		reserved[222];
    u8		oem_data[256];
} __attribute__((packed));

struct Vbemodeinfo {
    u16		attributes;
    u8		window_a_attributes;
    u8		window_b_attributes;
    u16		window_granularity;
    u16		window_size;
    u16		window_a_segment;
    u16		window_b_segment;
    u32		window_function;	// real mode pointer
    u16		bytes_per_row;

    // VBE 1.2 and above
    u16		width;
    u16		height;
    u8		char_width;
    u8		char_height;
    u8		num_planes;
    u8		bits_per_pixel;
    u8		num_banks;
    u8		memory_model;
    u8		bank_size;
    u8		num_image_pages;
    u8		_reserved0;

    // direct color fields
    u8		red_mask_size;
    u8		red_field_position;
    u8		green_mask_size;
    u8		green_field_position;
    u8		blue_mask_size;
    u8		blue_field_position;
    u8		reserved_mask_size;
    u8		reserved_field_position;
    u8		direct_color_mode_info;

    // VBE 2.0 and above
    u32		physical_base;
    u32		_reserved1;
    u16		_reserved2;

    // VBE 3.0 and above
    u16		linear_bytes_per_row;
    u8		banked_num_image_pages;
    u8		linear_num_image_pages;

    u8		linear_red_mask_size;
    u8		linear_red_field_position;
    u8		linear_green_mask_size;
    u8		linear_green_field_position;
    u8		linear_blue_mask_size;
    u8		linear_blue_field_position;
    u8		linear_reserved_mask_size;
    u8		linear_reserved_field_position;

    u32		max_pixel_clock;		// in Hz

    u8		_reserved[189];
} __attribute__((packed));


u16 vbe_read(u16 index) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    return inw(VBE_DISPI_IOPORT_DATA);
}

void vbe_write(u16 index, u16 value)
{
   outw(VBE_DISPI_IOPORT_INDEX, index);
   outw(VBE_DISPI_IOPORT_DATA, value);
}


void vbe_set(u16 xres, u16 yres, u16 bpp)
{
   vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
   vbe_write(VBE_DISPI_INDEX_XRES, xres);
   vbe_write(VBE_DISPI_INDEX_YRES, yres);
   vbe_write(VBE_DISPI_INDEX_BPP, bpp);
   vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
}

void vbe_init() {

    if (!(vbe_read(VBE_DISPI_INDEX_ID) == 0xB0C5)) {
        return;
    };
    // 0x1234:0x1111

}
