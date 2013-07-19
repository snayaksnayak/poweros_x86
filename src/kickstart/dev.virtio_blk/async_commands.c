#include "exec_funcs.h"
#include "virtio_blk_internal.h"

#define SysBase VirtioBlkBase->VirtioBlk_SysBase

void VirtioBlkInvalid(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq)
{
	DPrintF("Inside VirtioBlkInvalid!\n");
	VirtioBlk_end_command(VirtioBlkBase, IOERR_NOCMD, ioreq);
}
void VirtioBlkStart(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq)
{
	UINT32 irq = 0;
	DPrintF("Inside VirtioBlkStart!\n");

	// start irq server after driver is ok
	VirtioBlkBase->VirtioBlkIntServer = CreateIntServer(DevName, VIRTIO_BLK_INT_PRI, VirtioBlkIRQServer, VirtioBlkBase);
	AddIntServer(irq, VirtioBlkBase->VirtioBlkIntServer);

	VirtioBlk_end_command(VirtioBlkBase, IOERR_NOCMD, ioreq);

}
void VirtioBlkStop(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq)
{
DPrintF("Inside VirtioBlkStop!\n");
	VirtioBlk_end_command(VirtioBlkBase, IOERR_NOCMD, ioreq);

}
void VirtioBlkRead(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq)
{
DPrintF("Inside VirtioBlkRead!\n");
	VirtioBlk_end_command(VirtioBlkBase, IOERR_NOCMD, ioreq);

}
void VirtioBlkWrite(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq)
{
DPrintF("Inside VirtioBlkWrite!\n");
	VirtioBlk_end_command(VirtioBlkBase, IOERR_NOCMD, ioreq);

}

void VirtioBlkGetDeviceInfo(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq)
{
	DPrintF("Inside TimerSetSysTime!\n");
	UINT32 ipl = Disable();
	//do something critical here
	Enable(ipl);

	VirtioBlk_end_command(VirtioBlkBase, 0, (struct IOStdReq *)ioreq);
}


void (*VirtioBlkCmdVector[])(VirtioBlkBase *, struct IOStdReq * ) =
{
	//standard
	VirtioBlkInvalid, //VirtioBlkInvalid
	0, //VirtioBlkReset
	VirtioBlkRead, //VirtioBlkRead
	VirtioBlkWrite, //VirtioBlkWrite
	0, //VirtioBlkUpdate
	0, //VirtioBlkClear
	VirtioBlkStop, //VirtioBlkStop
	VirtioBlkStart, //VirtioBlkStart
	0, //VirtioBlkFlush

	//non standard
	VirtioBlkGetDeviceInfo //VirtioBlkGetDeviceInfo
};

/*
INT8 VirtioBlkCmdQuick[] =
// -1 : not queued
// 0  : queued
{
	-1, //VirtioBlkInvalid
	-1, //VirtioBlkReset
	0, //VirtioBlkRead
	0, //VirtioBlkWrite
	-1, //VirtioBlkUpdate
	-1, //VirtioBlkClear
	-1, //VirtioBlkStop
	-1, //VirtioBlkStart
	-1, //VirtioBlkFlush

	-1, //VirtioBlkGetDeviceInfo
};
*/

