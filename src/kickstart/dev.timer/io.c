#include "exec_funcs.h"
#include "timer_intern.h"

#define SysBase TimerBase->Timer_SysBase

void TimerInvalid(TimerBase *TimerBase, struct IORequest *ioreq)
{
	DPrintF("Inside TimerInvalid!\n");
	INTERN_EndCommand(TimerBase, IOERR_NOCMD, ioreq);
}

void TimerAddRequest(TimerBase *TimerBase, struct IORequest *ioreq)
{
	DPrintF("Inside TimerAddRequest!\n");
	struct TimerUnit* tu;
	tu = (struct TimerUnit *)((struct TimeRequest*)ioreq)->tr_node.io_Unit;
	((void(*)(struct TimerBase *, struct IORequest *))tu->AddRequest)(TimerBase, ioreq);
}

void TimerGetSysTime(TimerBase *TimerBase, struct IORequest *ioreq)
{
	DPrintF("Inside TimerGetSysTime!\n");
	((struct TimeRequest*)ioreq)->tr_time.tv_micro = TimerBase->CurrentTime.tv_micro;
	((struct TimeRequest*)ioreq)->tr_time.tv_secs  = TimerBase->CurrentTime.tv_secs;

	INTERN_EndCommand(TimerBase, 0, (struct IORequest *)ioreq);
}

void TimerSetSysTime(TimerBase *TimerBase, struct IORequest *ioreq)
{
	DPrintF("Inside TimerSetSysTime!\n");
	UINT32 ipl = Disable();
	DPrintF("TimerBase->CurrentTime.tv_secs = %d\n", TimerBase->CurrentTime.tv_secs);
	DPrintF("TimerBase->CurrentTime.tv_micro = %d\n", TimerBase->CurrentTime.tv_micro);
	DPrintF("((struct TimeRequest*)ioreq)->tr_time.tv_secs = %d\n", ((struct TimeRequest*)ioreq)->tr_time.tv_secs);
	DPrintF("((struct TimeRequest*)ioreq)->tr_time.tv_micro = %d\n", ((struct TimeRequest*)ioreq)->tr_time.tv_micro);
	TimerBase->CurrentTime.tv_secs = ((struct TimeRequest*)ioreq)->tr_time.tv_secs;
	TimerBase->CurrentTime.tv_micro= ((struct TimeRequest*)ioreq)->tr_time.tv_micro;
	Enable(ipl);

	INTERN_EndCommand(TimerBase, 0, (struct IORequest *)ioreq);
}


void (*TimerCmdVector[])(TimerBase *, struct IORequest * ) =
{
	TimerInvalid,
	0, //TimerReset
	0, //TimerRead
	0, //TimerWrite
	0, //TimerUpdate
	0, //TimerClear
	0, //TimerStopCmd
	0, //TimerStart
	0, //TimerFlush

	TimerAddRequest,
	TimerGetSysTime,
	TimerSetSysTime
};

/*
INT8 TimerCmdQuick[] =
// -1 : not queued
// 0  : queued
{
	-1, //TimerInvalid
	-1, //TimerReset
	-1, //TimerRead
	-1, //TimerWrite
	-1, //TimerUpdate
	-1, //TimerClear
	-1, //TimerStopCmd
	-1, //TimerStart
	-1, //TimerFlush

	0, //TimerAddRequest
	-1, //TimerGetSysTime
	-1 //TimerSetSysTime
};
*/

void INTERN_EndCommand(TimerBase *TimerBase, UINT32 error, struct IORequest *ioreq)
{
	ioreq->io_Error = error;
	if (TEST_BITS(ioreq->io_Flags, IOF_QUICK)) return;
	ReplyMsg((struct Message *)ioreq);
	return;
}
