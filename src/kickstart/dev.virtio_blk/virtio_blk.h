#ifndef virtio_blk_h
#define virtio_blk_h

// this file shall go to top most include folder in future

#include "types.h"
#include "device.h"
#include "io.h"

// Units
//#define UNIT_DISK    0
//#define UNIT_CD    1
//#define UNIT_DVD    2

// non standard async commands
#define VB_GETDEVICEINFO (CMD_NONSTD+0)

struct VirtioBlkDeviceInfo
{
	UINT64 capacity; //number of 512 byte sectors
	UINT32 blk_size; //512 or 1024 etc.
	struct VirtioBlkGeometry geometry;
};

struct VirtioBlkGeometry
{
	UINT16 cylinders;
	UINT8 heads;
	UINT8 sectors;
};

struct VirtioBlkRequest
{
    struct IOStdReq node;
    struct VirtioBlkDeviceInfo info;
    UINT32 sector_num;
    UINT8* buf;
};


// Blk Device library style synchronous functions (non standard or device specific)
// we have nothing now

// library style vectors to synchronous functions (non standard or device specific)
// we have nothing now

#endif //virtio_blk_h
