#ifndef lib_virtio_internal_h
#define lib_virtio_internal_h

#include "types.h"
#include "library.h"
#include "arch_config.h"
#include "virtio_ring.h"
#include "expansionbase.h"

#define LIB_VIRTIO_VERSION  0
#define LIB_VIRTIO_REVISION 1

#define LIB_VIRTIO_LIBNAME "lib_virtio"
#define LIB_VIRTIO_LIBVER  " 0.1 __DATE__"

//vendor id for all virtio devices
#define VIRTIO_VENDOR_ID 0x1af4

//pci config offsets
#define VIRTIO_HOST_F_OFFSET			0x0000
#define VIRTIO_GUEST_F_OFFSET			0x0004
#define VIRTIO_QADDR_OFFSET				0x0008
#define VIRTIO_QSIZE_OFFSET				0x000C
#define VIRTIO_QSEL_OFFSET				0x000E
#define VIRTIO_QNOTFIY_OFFSET			0x0010
#define VIRTIO_DEV_STATUS_OFFSET		0x0012
#define VIRTIO_ISR_STATUS_OFFSET		0x0013
#define VIRTIO_DEV_SPECIFIC_OFFSET		0x0014

//device status
#define VIRTIO_STATUS_RESET			0x00
#define VIRTIO_STATUS_ACK			0x01
#define VIRTIO_STATUS_DRV			0x02
#define VIRTIO_STATUS_DRV_OK		0x04
#define VIRTIO_STATUS_FAIL			0x80

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


typedef struct VirtioDevice
{
	PCIAddress		pciAddr;
	volatile UINT16			io_addr;

	UINT8 intLine;
	UINT8 intPin;

	int num_features;
	virtio_feature*   features;

	UINT16 num_queues;
	struct virtio_queue *queues;

} VirtioDevice;


typedef struct LibVirtioBase
{
	struct Library		Library;
	APTR				SysBase;

} LibVirtioBase;


extern char LibVirtioLibName[];
extern char LibVirtioLibVer[];


//functions
struct LibVirtioBase *lib_virtio_OpenLib(struct LibVirtioBase *LibVirtioBase);
APTR lib_virtio_CloseLib(struct LibVirtioBase *LibVirtioBase);
APTR lib_virtio_ExpungeLib(struct LibVirtioBase *LibVirtioBase);
APTR lib_virtio_ExtFuncLib(void);

void lib_virtio_Write8(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset, UINT8 val);
void lib_virtio_Write16(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset, UINT16 val);
void lib_virtio_Write32(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset, UINT32 val);
UINT8 lib_virtio_Read8(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset);
UINT16 lib_virtio_Read16(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset);
UINT32 lib_virtio_Read32(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset);

void lib_virtio_ExchangeFeatures(struct LibVirtioBase *LibVirtioBase, VirtioDevice *vd);
int lib_virtio_AllocateQueues(struct LibVirtioBase *LibVirtioBase, VirtioDevice *vd, INT32 num_queues);
int lib_virtio_InitQueues(struct LibVirtioBase *LibVirtioBase, VirtioDevice *vd);
void lib_virtio_FreeQueues(struct LibVirtioBase *LibVirtioBase, VirtioDevice *vd);
int lib_virtio_HostSupports(LibVirtioBase *LibVirtioBase, VirtioDevice *vd, int bit);
int lib_virtio_GuestSupports(LibVirtioBase *LibVirtioBase, VirtioDevice *vd, int bit);


//internals
int LibVirtio_supports(LibVirtioBase *LibVirtioBase, VirtioDevice *vd, int bit, int host);

int LibVirtio_alloc_phys_queue(LibVirtioBase *LibVirtioBase, struct virtio_queue *q);
void LibVirtio_init_phys_queue(LibVirtioBase *LibVirtioBase, struct virtio_queue *q);
void LibVirtio_free_phys_queue(LibVirtioBase *LibVirtioBase, struct virtio_queue *q);



#endif //lib_virtio_internal_h


