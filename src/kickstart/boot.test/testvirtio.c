#include "types.h"
#include "sysbase.h"
#include "expansionbase.h"
#include "expansion_funcs.h"
#include "exec_funcs.h"

static __inline__ void
IO_Out8(UINT16 port, UINT8 value)
{
   __asm__ __volatile__ ("outb %0, %1" : :"a" (value), "d" (port));
}

static __inline__ void
IO_Out16(UINT16 port, UINT16 value)
{
   __asm__ __volatile__ ("outw %0, %1" : :"a" (value), "d" (port));
}

static __inline__ void
IO_Out32(UINT16 port, UINT32 value)
{
   __asm__ __volatile__ ("outl %0, %1" : :"a" (value), "d" (port));
}

static __inline__ UINT8
IO_In8(UINT16 port)
{
   UINT8 value;
   __asm__ __volatile__ ("inb %1, %0" :"=a" (value) :"d" (port));
   return value;
}

static __inline__ UINT16
IO_In16(UINT16 port)
{
   UINT16 value;
   __asm__ __volatile__ ("inw %1, %0" :"=a" (value) :"d" (port));
   return value;
}

static __inline__ UINT32
IO_In32(UINT16 port)
{
   UINT32 value;
   __asm__ __volatile__ ("inl %1, %0" :"=a" (value) :"d" (port));
   return value;
}

void virtio_write8(UINT16 base, UINT16 offset, UINT8 val)
{
	IO_Out8(base+offset, val);
}
void virtio_write16(UINT16 base, UINT16 offset, UINT16 val)
{
	IO_Out16(base+offset, val);
}
void virtio_write32(UINT16 base, UINT16 offset, UINT32 val)
{
	IO_Out32(base+offset, val);
}

UINT8 virtio_read8(UINT16 base, UINT16 offset)
{
	return IO_In8(base+offset);
}
UINT16 virtio_read16(UINT16 base, UINT16 offset)
{
	return IO_In16(base+offset);
}
UINT32 virtio_read32(UINT16 base, UINT16 offset)
{
	return IO_In32(base+offset);
}

#define VIRTIO_VENDOR_ID 0x1af4
#define VIRTIO_BLK_DEVICE_ID 0x1001

#define VIRTIO_HOST_F_OFFSET			0x0000
#define VIRTIO_GUEST_F_OFFSET			0x0004
#define VIRTIO_QADDR_OFFSET				0x0008
#define VIRTIO_QSIZE_OFFSET				0x000C
#define VIRTIO_QSEL_OFFSET				0x000E
#define VIRTIO_QNOTFIY_OFFSET			0x0010
#define VIRTIO_DEV_STATUS_OFFSET		0x0012
#define VIRTIO_ISR_STATUS_OFFSET		0x0013

#define VIRTIO_DEV_SPECIFIC_OFFSET		0x0014

#define VIRTIO_STATUS_RESET			0x00
#define VIRTIO_STATUS_ACK			0x01
#define VIRTIO_STATUS_DRV			0x02
#define VIRTIO_STATUS_DRV_OK		0x04
#define VIRTIO_STATUS_FAIL			0x80

// Feature description
typedef struct virtio_feature
{
	char *name;
	UINT8 bit;
	UINT8 host_support;
	UINT8 guest_support;
} virtio_feature;

typedef struct virtio_test {
	SysBase			*SysBase;
	ExpansionBase	*ExpansionBase;
	PCIAddress		pciAddr;

	/* For registers access */
	volatile UINT16			io_addr;
	int num_features;
	virtio_feature*   features;

} virtio_test;

virtio_test vt_str;
virtio_test* vt = &vt_str;

// Feature bits
#define VIRTIO_BLK_F_BARRIER	0	// Does host support barriers?
#define VIRTIO_BLK_F_SIZE_MAX	1	// Indicates maximum segment size
#define VIRTIO_BLK_F_SEG_MAX	2	// Indicates maximum # of segments
#define VIRTIO_BLK_F_GEOMETRY	4	// Legacy geometry available
#define VIRTIO_BLK_F_RO			5	// Disk is read-only
#define VIRTIO_BLK_F_BLK_SIZE	6	// Block size of disk is available
#define VIRTIO_BLK_F_SCSI		7	// Supports scsi command passthru
#define VIRTIO_BLK_F_FLUSH		9	// Cache flush command support
#define VIRTIO_BLK_F_TOPOLOGY	10	// Topology information is available
#define VIRTIO_BLK_ID_BYTES		20	// ID string length

virtio_feature blkf[] = {
	{ "barrier",	VIRTIO_BLK_F_BARRIER,	0,	0 	},
	{ "sizemax",	VIRTIO_BLK_F_SIZE_MAX,	0,	0	},
	{ "segmax",		VIRTIO_BLK_F_SEG_MAX,	0,	0	},
	{ "geometry",	VIRTIO_BLK_F_GEOMETRY,	0,	0	},
	{ "read-only",	VIRTIO_BLK_F_RO,		0,	0	},
	{ "blocksize",	VIRTIO_BLK_F_BLK_SIZE,	0,	0	},
	{ "scsi",		VIRTIO_BLK_F_SCSI,		0,	0	},
	{ "flush",		VIRTIO_BLK_F_FLUSH,		0,	0	},
	{ "topology",	VIRTIO_BLK_F_TOPOLOGY,	0,	0	},
	{ "idbytes",	VIRTIO_BLK_ID_BYTES,	0,	0	}
};



void exchange_features(APTR SysBase, virtio_test *vt)
{
	UINT32 guest_features = 0, host_features = 0;
	virtio_feature *f;

	//collect host features
	host_features = virtio_read32(vt->io_addr, VIRTIO_HOST_F_OFFSET);

	for (int i = 0; i < vt->num_features; i++)
	{
		f = &vt->features[i];

		// prepare the features the guest/driver supports
		guest_features |= (f->guest_support << f->bit);
		DPrintF("guest feature %d\n", (f->guest_support << f->bit));

		// just load the host/device feature int the struct
		f->host_support |=  ((host_features >> f->bit) & 1);
		DPrintF("host feature %d\n\n", ((host_features >> f->bit) & 1));
	}

	// let the device know about our features
	virtio_write32(vt->io_addr, VIRTIO_GUEST_F_OFFSET, guest_features);
}

void DetectVirtio(APTR SysBase)
{
	DPrintF("DetectVirtio\n");
	struct ExpansionBase *ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library", 0);
	vt->ExpansionBase = ExpansionBase;

	if (vt->ExpansionBase == NULL) {
		DPrintF("DetectVirtio: Cant open expansion.library\n");
		return;
	}

	if (!PCIFindDevice(VIRTIO_VENDOR_ID, VIRTIO_BLK_DEVICE_ID, &vt->pciAddr)) {
		DPrintF("DetectVirtio: No Virtio device found.");
		return ;
	}
	else
	{
		DPrintF("DetectVirtio: Virtio block device found.\n");
	}

	DPrintF("DetectVirtio: (vt->pciAddr).bus %x\n", (vt->pciAddr).bus);
	DPrintF("DetectVirtio: (vt->pciAddr).device %x\n", (vt->pciAddr).device);
	DPrintF("DetectVirtio: (vt->pciAddr).function %x\n", (vt->pciAddr).function);

	PCISetMemEnable(&vt->pciAddr, TRUE);
	vt->io_addr = PCIGetBARAddr(&vt->pciAddr, 0);
	DPrintF("DetectVirtio: ioAddress %x\n", vt->io_addr);

	// Reset the device
	virtio_write8(vt->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_RESET);

	// Ack the device
	virtio_write8(vt->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_ACK);

	//driver supports these features
	vt->features = blkf;
	vt->num_features = sizeof(blkf) / sizeof(blkf[0]);

	//exchange features
	exchange_features(SysBase, vt);

	//initialize desc tables
	//init_indirect_desc_tables();

	// We know how to drive the device...
	virtio_write8(vt->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_DRV);
}



