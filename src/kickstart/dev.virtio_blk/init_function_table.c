#include "virtio_blk_internal.h"
#include "sysbase.h"
#include "resident.h"
#include "exec_funcs.h"
#include "expansion_funcs.h"
#include "lib_virtio.h"
#include "arch_config.h"



char DevName[] = "virtio_blk.device";
char Version[] = "\0$VER: virtio_blk.device 0.1 ("__DATE__")\r\n";

static virtio_feature blkf[] = {
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

APTR virtio_blk_FuncTab[] =
{
 (void(*)) virtio_blk_OpenDev,
 (void(*)) virtio_blk_CloseDev,
 (void(*)) virtio_blk_ExpungeDev,
 (void(*)) virtio_blk_ExtFuncDev,

 (void(*)) virtio_blk_BeginIO,
 (void(*)) virtio_blk_AbortIO,

 (APTR) ((UINT32)-1)
};

struct VirtioBlkBase *virtio_blk_InitDev(struct VirtioBlkBase *VirtioBlkBase, UINT32 *segList, struct SysBase *SysBase)
{
	VirtioBlkBase->VirtioBlk_SysBase = SysBase;

	// Initialise Unit Command Queue
	NewList((struct List *)&VirtioBlkBase->unit.unit_MsgPort.mp_MsgList);
	VirtioBlkBase->unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)DevName;
	VirtioBlkBase->unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	VirtioBlkBase->unit.unit_MsgPort.mp_SigTask = NULL;

//****************
	struct ExpansionBase *ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library", 0);
	VirtioBlkBase->ExpansionBase = ExpansionBase;
	if (VirtioBlkBase->ExpansionBase == NULL) {
		DPrintF("virtio_blk_InitDev: Cant open expansion.library\n");
		return NULL;
	}


	struct LibVirtioBase *LibVirtioBase = (struct LibVirtioBase *)OpenLibrary("lib_virtio.library", 0);
	VirtioBlkBase->LibVirtioBase = LibVirtioBase;
	if (VirtioBlkBase->LibVirtioBase == NULL) {
		DPrintF("virtio_blk_InitDev: Cant open lib_virtio.library\n");
		return NULL;
	}


	VirtioBlk *vb = &(VirtioBlkBase->vb);
	VirtioDevice* vd = &(vb->vd);


//1. setup the device.

	//setup
	VirtioBlk_setup(VirtioBlkBase, vb);

	// Reset the device
	VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_RESET);

	// Ack the device
	VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_ACK);


//2. check if you can drive the device.

	//driver supports these features
	vd->features = blkf;
	vd->num_features = sizeof(blkf) / sizeof(blkf[0]);

	//exchange features
	VirtioExchangeFeatures(vd);

	// We know how to drive the device...
	VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_DRV);


//3. be ready to go.

	// virtio blk has only 1 queue
	VirtioAllocateQueues(vd, VIRTIO_BLK_NUM_QUEUES);

	//init queues
	VirtioInitQueues(vd);

	//Allocate memory for headers and status
	VirtioBlk_alloc_phys_requests(VirtioBlkBase, vb);

	//collect configuration
	VirtioBlk_configuration(VirtioBlkBase, vb);

	//Driver is ready to go!
	VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_DRV_OK);


//4. start data transfer.

/*
	UINT32 sector_num;
	UINT8 write = 0; //0 means "READ" a sector, 1 means "WRITE"
	UINT8 buf[512]; //buffer to which data is read/write, fill this buffer to write into device

	int i;
	for (i=0; i < 8; i++) //8192 max
	{
		//lets try to read the sectors
		sector_num = i;
		memset(buf, 0, 512);
		VirtioBlk_transfer(VirtioBlkBase, vb, sector_num, write, buf);
	}
*/


	return VirtioBlkBase;
}

static const struct VirtioBlkBase VirtioBlkDevData =
{
	.Device.dd_Library.lib_Node.ln_Name = (APTR)&DevName[0],
	.Device.dd_Library.lib_Node.ln_Type = NT_DEVICE,
	.Device.dd_Library.lib_Node.ln_Pri = 50,
	.Device.dd_Library.lib_OpenCnt = 0,
	.Device.dd_Library.lib_Flags = LIBF_SUMUSED|LIBF_CHANGED,
	.Device.dd_Library.lib_NegSize = 0,
	.Device.dd_Library.lib_PosSize = 0,
	.Device.dd_Library.lib_Version = VERSION,
	.Device.dd_Library.lib_Revision = REVISION,
	.Device.dd_Library.lib_Sum = 0,
	.Device.dd_Library.lib_IDString = (APTR)&Version[7],

	.vb.Info.capacity = 0,
	.vb.Info.blk_size = 0,
	.vb.Info.geometry.cylinders = 0,
	.vb.Info.geometry.heads = 0,
	.vb.Info.geometry.sectors = 0
};

//Init table
struct InitTable
{
	UINT32	LibBaseSize;
	APTR	FunctionTable;
	APTR	DataTable;
	APTR	InitFunction;
} virtio_blk_InitTab =
{
	sizeof(struct VirtioBlkBase),
	virtio_blk_FuncTab,
	(APTR)&VirtioBlkDevData,
	virtio_blk_InitDev
};

static APTR VirtioBlkEndResident;

// Resident ROMTAG
struct Resident VirtioBlkRomTag =
{
	RTC_MATCHWORD,
	&VirtioBlkRomTag,
	&VirtioBlkEndResident,
	RTF_AUTOINIT | RTF_COLDSTART,
	VERSION,
	NT_DEVICE,
	50,
	DevName,
	Version,
	0,
	&virtio_blk_InitTab
};

