#ifndef virtio_blk_internal_h
#define virtio_blk_internal_h

#include "types.h"
#include "device.h"
#include "io.h"
#include "expansionbase.h"
#include "arch_config.h"

#include "virtio_ring.h"

#define VERSION  0
#define REVISION 1

#define DEVNAME "virtio_blk"
#define DEVVER  " 0.1 __DATE__"

#define VIRTIO_BLK_INT_PRI 0

// Units
//#define UNIT_DISK    0
//#define UNIT_CD    1
//#define UNIT_DVD    2
//#define UNIT_MAX		3

// non standard async commands
#define VB_GETDEVICEINFO (CMD_NONSTD+0)


//****************
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

/* These two define direction. */
#define VIRTIO_BLK_T_IN		0
#define VIRTIO_BLK_T_OUT	1

//how many ques we have
#define VIRTIO_BLK_NUM_QUEUES 1


struct virtio_queue
{
	void* unaligned_addr;
	void* paddr;			/* physical addr of ring */
	UINT32 page;				/* physical guest page  = paddr/4096*/

	UINT16 num;				/* number of descriptors collected from device config offset*/
	UINT32 ring_size;			/* size of ring in bytes */
	struct vring vring;

	UINT16 free_num;				/* free descriptors */
	UINT16 free_head;			/* next free descriptor */
	UINT16 free_tail;			/* last free descriptor */
	UINT16 last_used;			/* we checked in used */

	void **data;				/* pointer to array of pointers */
};

// Feature description
typedef struct virtio_feature
{
	char *name;
	UINT8 bit;
	UINT8 host_support;
	UINT8 guest_support;
} virtio_feature;

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



/* This is the first element of the read scatter-gather list. */
struct virtio_blk_outhdr {
	/* VIRTIO_BLK_T* */
	UINT32 type;
	/* io priority. */
	UINT32 ioprio;
	/* Sector (ie. 512 byte offset) */
	UINT64 sector;
};

//*****************

struct VirtioBlkGeometry
{
	UINT16 cylinders;
	UINT8 heads;
	UINT8 sectors;
};

struct VirtioBlkDeviceInfo
{
	UINT64 capacity; //number of 512 byte sectors
	UINT32 blk_size; //512 or 1024 etc.
	struct VirtioBlkGeometry geometry;
};


struct VirtioBlkRequest
{
    struct IOStdReq node;
    struct VirtioBlkDeviceInfo info;
    UINT32 sector_num;
    UINT8* buf;
};

typedef struct VirtioBlk
{
	PCIAddress		pciAddr;
	volatile UINT16			io_addr;

	UINT8 intLine;
	UINT8 intPin;

	int num_features;
	virtio_feature*   features;

	UINT16 num_queues;
	struct virtio_queue *queues;

	/* Headers for requests */
	struct virtio_blk_outhdr *hdrs;

	/* Status bytes for requests.
	 *
	 * Usually a status is only one byte in length, but we need the lowest bit
	 * to propagate writable. For this reason we take u16_t and use a mask for
	 * the lower byte later.
	 */
	UINT16 *status;

	struct VirtioBlkDeviceInfo Info;

} VirtioBlk;

typedef struct VirtioBlkBase
{
	struct Device		Device;
	APTR				VirtioBlk_SysBase;
	ExpansionBase		*ExpansionBase;

	UINT32				VirtioBlkIRQ;
	struct Interrupt	*VirtioBlkIntServer;

	struct Unit unit;

	struct VirtioBlk vb;

} VirtioBlkBase;


extern char DevName[];
extern char Version[];


extern void (*VirtioBlkCmdVector[])(VirtioBlkBase *, struct IOStdReq*);
//extern INT8 VirtioBlkCmdQuick[];

//functions
struct VirtioBlkBase *virtio_blk_OpenDev(struct VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq, UINT32 unitNum, UINT32 flags);
APTR virtio_blk_CloseDev(struct VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);
APTR virtio_blk_ExpungeDev(struct VirtioBlkBase *VirtioBlkBase);
APTR virtio_blk_ExtFuncDev(void);
void virtio_blk_BeginIO(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);
void virtio_blk_AbortIO(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);


//commands
void VirtioBlkInvalid(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);
void VirtioBlkStart(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);
void VirtioBlkStop(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);
void VirtioBlkRead(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);
void VirtioBlkWrite(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);
void VirtioBlkGetDeviceInfo(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);

//internals
void VirtioBlk_end_command(VirtioBlkBase *VirtioBlkBase, UINT32 error, struct IOStdReq *ioreq);
void VirtioBlk_queue_command(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);

void virtio_write8(UINT16 base, UINT16 offset, UINT8 val);
void virtio_write16(UINT16 base, UINT16 offset, UINT16 val);
void virtio_write32(UINT16 base, UINT16 offset, UINT32 val);
UINT8 virtio_read8(UINT16 base, UINT16 offset);
UINT16 virtio_read16(UINT16 base, UINT16 offset);
UINT32 virtio_read32(UINT16 base, UINT16 offset);

void VirtioBlk_exchange_features(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vt);

int VirtioBlk_alloc_phys_queue(VirtioBlkBase *VirtioBlkBase, struct virtio_queue *q);
void VirtioBlk_init_phys_queue(VirtioBlkBase *VirtioBlkBase, struct virtio_queue *q);
void VirtioBlk_free_phys_queue(VirtioBlkBase *VirtioBlkBase, struct virtio_queue *q);

int VirtioBlk_alloc_phys_queues(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vt, int num_queues);
int VirtioBlk_init_phys_queues(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vt);
void VirtioBlk_free_phys_queues(VirtioBlkBase *VirtioBlkBase, VirtioBlk *dev);

int VirtioBlk_alloc_phys_requests(VirtioBlkBase *VirtioBlkBase,VirtioBlk *vb);

int VirtioBlk_supports(VirtioBlkBase *VirtioBlkBase, VirtioBlk *dev, int bit, int host);
int VirtioBlk_host_supports(VirtioBlkBase *VirtioBlkBase, VirtioBlk *dev, int bit);
int VirtioBlk_guest_supports(VirtioBlkBase *VirtioBlkBase, VirtioBlk *dev, int bit);

int VirtioBlk_configuration(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vt);
void VirtioBlk_transfer(VirtioBlkBase *VirtioBlkBase, VirtioBlk* vt, UINT32 sector_num, UINT8 write, UINT8* buf);


//irq handler
__attribute__((no_instrument_function)) BOOL VirtioBlkIRQServer(UINT32 number, VirtioBlkBase *VirtioBlkBase, APTR SysBase);

#endif //virtio_blk_internal_h


