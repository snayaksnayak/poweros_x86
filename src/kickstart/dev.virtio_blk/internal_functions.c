#include "exec_funcs.h"
#include "virtio_blk_internal.h"

#define SysBase VirtioBlkBase->VirtioBlk_SysBase

void INTERN_VirtioBlkQueueRequest(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq)
{
	UINT32 ipl = Disable();

	struct Unit	*unit = ioreq->io_Unit;

	if (ioreq->io_Error == 0)
	{
		PutMsg(&unit->unit_MsgPort, &ioreq->io_Message);
	}
	Enable(ipl);
	return;
}

void INTERN_VirtioBlkEndCommand(VirtioBlkBase *VirtioBlkBase, UINT32 error, struct IOStdReq *ioreq)
{
	ioreq->io_Error = error;
	if (TEST_BITS(ioreq->io_Flags, IOF_QUICK)) return;
	ReplyMsg((struct Message *)ioreq);
	return;
}
