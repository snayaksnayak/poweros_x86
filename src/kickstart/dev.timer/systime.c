#include "exec_funcs.h"
#include "timer_intern.h"

void set_timer(UINT32 hz);
UINT16 get_timer();

#define SysBase TimerBase->Timer_SysBase

void AddAlarm(TimerBase *TimerBase, struct IORequest *ioreq)
{
	DPrintF("Inside AddAlarm!\n");
	UINT32 ipl;
	ipl = Disable();

	DPrintF("TimerBase->CurrentTime->tv_secs = %d\n", TimerBase->CurrentTime.tv_secs);
	DPrintF("TimerBase->CurrentTime->tv_micro = %d\n", TimerBase->CurrentTime.tv_micro);
	DPrintF("((struct TimeRequest*)ioreq)->tr_time->tv_secs = %d\n", ((struct TimeRequest*)ioreq)->tr_time.tv_secs);
	DPrintF("((struct TimeRequest*)ioreq)->tr_time->tv_micro = %d\n", ((struct TimeRequest*)ioreq)->tr_time.tv_micro);

	// if CurrentTime >= tr_time, dont add alarm!
	if(timer_CmpTime(TimerBase, &TimerBase->CurrentTime, &((struct TimeRequest*)ioreq)->tr_time) >= 0)
	{
		Enable(ipl);
		DPrintF("Can't Add Alarm!\n");
		((struct TimeRequest*)ioreq)->tr_time.tv_secs = ((struct TimeRequest*)ioreq)->tr_time.tv_micro = 0;
		INTERN_EndCommand(TimerBase, 0, (struct IORequest *)ioreq);
	}
	else
	{
		// Ok, we add this to the list
		INTERN_QueueRequest(TimerBase, ioreq);

		Enable(ipl);

		DPrintF("Alarm added!\n");
		//CLEAR_BITS(((struct TimeRequest*)ioreq)->tr_node.io_Flags, IOF_QUICK);
		((struct TimeRequest*)ioreq)->tr_node.io_Flags &= ~IOF_QUICK;
	}
}
void AddDelay(TimerBase *TimerBase, struct IORequest *ioreq)
{
	DPrintF("Inside AddDelay!\n");
	UINT32 ipl;
	ipl = Disable();

	    timer_AddTime(TimerBase, &TimerBase->Elapsed, &((struct TimeRequest*)ioreq)->tr_time);

	    INTERN_QueueRequest(TimerBase, ioreq);

	    Enable(ipl);

		DPrintF("Delay added!\n");
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
	}
	Enable(ipl);
	return;
}
