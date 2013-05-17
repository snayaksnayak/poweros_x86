#include "exec_funcs.h"
#include "timer_intern.h"

void AddAlarm(TimerBase *TimerBase, struct IORequest *ioreq)
{
	UINT32 ipl;
	ipl = Disable();

	if(timer_CmpTime(TimerBase, &TimerBase->CurrentTime, &((struct TimeRequest*)ioreq)->tr_time) <= 0)
	{
		Enable(ipl);

		((struct TimeRequest*)ioreq)->tr_time.tv_secs = ((struct TimeRequest*)ioreq)->tr_time.tv_micro = 0;
		INTERN_EndCommand(TimerBase, 0, (struct IORequest *)ioreq);
	}
	else
	{
		// Ok, we add this to the list
		INTERN_QueueRequest(TimerBase, ioreq);
		//AddTail((struct List *)&TimerBase->Lists[UNIT_WAITUNTIL], (struct Node *)ioreq);

		Enable(ipl);

		//CLEAR_BITS(((struct TimeRequest*)ioreq)->tr_node.io_Flags, IOF_QUICK);
		((struct TimeRequest*)ioreq)->tr_node.io_Flags &= ~IOF_QUICK;
	}
}
void AddDelay(TimerBase *TimerBase, struct IORequest *ioreq)
{
	UINT32 ipl;
	ipl = Disable();

	    timer_AddTime(TimerBase, &TimerBase->Elapsed, &((struct TimeRequest*)ioreq)->tr_time);

        //AddTail((struct List *)&TimerBase->Lists[UNIT_VBLANK], (struct Node *)ioreq);
	    INTERN_QueueRequest(TimerBase, ioreq);

	    Enable(ipl);

	    //CLEAR_BITS(((struct TimeRequest*)ioreq)->tr_node.io_Flags, IOF_QUICK);
	    ((struct TimeRequest*)ioreq)->tr_node.io_Flags &= ~IOF_QUICK;
}

void INTERN_QueueRequest(TimerBase *TimerBase, struct IORequest *ioreq)
{
	UINT32 ipl = Disable();

	struct Unit	*unit = ioreq->io_Unit;

	if (ioreq->io_Error == 0)
	{
		PutMsg(&unit->unit_MsgPort, &ioreq->io_Message);
		//DPrintF("DEBUG| Head %x, Adress Node %x\n", unit->unit_MsgPort.mp_MsgList.lh_Head, &io->io_Message.mn_Node);
	}
	Enable(ipl);
	return;
}
