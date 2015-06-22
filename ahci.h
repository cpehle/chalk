typedef enum
{
    FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
    FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
    FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
    FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
    FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FisType;


typedef struct FisRegHostToDevice
{
    // DWORD 0
    u8	fis_type;	// FIS_TYPE_REG_H2D

    u8	pmport:4;	// Port multiplier
    u8	rsv0:3;		// Reserved
    u8	c:1;		// 1: Command, 0: Control

    u8	command;	// Command register
    u8	featurel;	// Feature register, 7:0

    // DWORD 1
    u8	lba0;		// LBA low register, 7:0
    u8	lba1;		// LBA mid register, 15:8
    u8	lba2;		// LBA high register, 23:16
    u8	device;		// Device register

    // DWORD 2
    u8	lba3;		// LBA register, 31:24
    u8	lba4;		// LBA register, 39:32
    u8	lba5;		// LBA register, 47:40
    u8	featureh;	// Feature register, 15:8

    // DWORD 3
    u8	countl;		// Count register, 7:0
    u8	counth;		// Count register, 15:8
    u8	icc;		// Isochronous command completion
    u8	control;	// Control register

    // DWORD 4
    u8	rsv1[4];	// Reserved
} FisRegHostToDevice;


typedef struct FisRegDeviceToHost
{
    // DWORD 0
    u8	fis_type;    // FIS_TYPE_REG_D2H

    u8	pmport:4;    // Port multiplier
    u8	rsv0:2;      // Reserved
    u8	i:1;         // Interrupt bit
    u8	rsv1:1;      // Reserved

    u8	status;      // Status register
    u8	error;       // Error register

    // DWORD 1
    u8	lba0;        // LBA low register, 7:0
    u8	lba1;        // LBA mid register, 15:8
    u8	lba2;        // LBA high register, 23:16
    u8	device;      // Device register

    // DWORD 2
    u8	lba3;        // LBA register, 31:24
    u8	lba4;        // LBA register, 39:32
    u8	lba5;        // LBA register, 47:40
    u8	rsv2;        // Reserved

    // DWORD 3
    u8	countl;      // Count register, 7:0
    u8	counth;      // Count register, 15:8
    u8	rsv3[2];     // Reserved

    // DWORD 4
    u8	rsv4[4];     // Reserved
} FisRegDeviceToHost;

typedef struct FisData
{
    // DWORD 0
    u8	fis_type;	// FIS_TYPE_DATA

    u8	pmport:4;	// Port multiplier
    u8	rsv0:4;		// Reserved

    u8	rsv1[2];	// Reserved

    // DWORD 1 ~ N
    u32	data[1];	// Payload
} FisData;
