#include "virtio_blk_internal.h"
#include "list.h"
#include "exec_funcs.h"

__attribute__((no_instrument_function)) BOOL VirtioBlkIRQServer(UINT32 number, VirtioBlkBase *VirtioBlkBase, APTR SysBase)
{
	DPrintF("VirtioBlkIRQServer\n");

	struct  Unit *unit = (struct  Unit *)&VirtioBlkBase->unit;
	struct VirtioBlkRequest *head_req = (struct VirtioBlkRequest *)GetHead(&unit->unit_MsgPort.mp_MsgList);

	DPrintF("One request complete\n");
	Remove((struct Node *)head_req);
	head_req->node.io_Error = 0;
	ReplyMsg((struct Message *)head_req);

	struct IOStdReq* next_req = (struct IOStdReq *)GetHead(&unit->unit_MsgPort.mp_MsgList);
	if(next_req != NULL)
	{
		//start processing another request
		VirtioBlk_process_request(VirtioBlkBase, next_req);
	}

	return 0; // we return 0 so that Tick() can run, otherwise we would cut off Schedule()
}

