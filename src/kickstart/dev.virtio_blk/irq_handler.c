#include "virtio_blk_internal.h"
#include "list.h"
#include "exec_funcs.h"

__attribute__((no_instrument_function)) BOOL VirtioBlkIRQServer(UINT32 number, VirtioBlkBase *VirtioBlkBase, APTR SysBase)
{
	DPrintF("VirtioBlkIRQServer\n");

	struct VirtioBlkRequest *vbr, *vbrtmp;

	ForeachNodeSafe(&VirtioBlkBase->unit.unit_MsgPort.mp_MsgList, vbr, vbrtmp)
	{
		if (0)
		{
			DPrintF("Found something from list\n");
			Remove((struct Node *)vbr);
			vbr->node.io_Error = 0;
			ReplyMsg((struct Message *)vbr);
		}
	}


	return 0; // we return 0 so that Tick() can run, otherwise we would cut off Schedule()
}

