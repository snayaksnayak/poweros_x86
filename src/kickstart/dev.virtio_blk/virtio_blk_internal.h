#ifndef virtio_blk_internal_h
#define virtio_blk_internal_h

#include "types.h"
#include "device.h"
#include "io.h"

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

typedef struct VirtioBlkBase
{
	struct Device		Device;
	APTR				VirtioBlk_SysBase;
	struct VirtioBlkDeviceInfo Info;

	UINT32				VirtioBlkIRQ;
	struct Interrupt	*VirtioBlkIntServer;

	struct Unit unit;

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
void INTERN_VirtioBlkEndCommand(VirtioBlkBase *VirtioBlkBase, UINT32 error, struct IOStdReq *ioreq);
void INTERN_VirtioBlkQueueRequest(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq);

//irq handler
__attribute__((no_instrument_function)) BOOL VirtioBlkIRQServer(UINT32 number, VirtioBlkBase *VirtioBlkBase, APTR SysBase);

#endif //virtio_blk_internal_h


