#include "virtio_blk_internal.h"
#include "sysbase.h"
#include "resident.h"
#include "exec_funcs.h"

char DevName[] = "virtio_blk.device";
char Version[] = "\0$VER: virtio_blk.device 0.1 ("__DATE__")\r\n";

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

	.Info.capacity = 0,
	.Info.blk_size = 0,
	.Info.geometry.cylinders = 0,
	.Info.geometry.heads = 0,
	.Info.geometry.sectors = 0
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

