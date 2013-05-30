#include "exec_funcs.h"
#include "timer_intern.h"

struct TimerBase *timer_OpenDev(struct TimerBase *TimerBase, struct IORequest *ioreq, UINT32 unitNum, UINT32 flags)
{
	//DPrintF("[TimerDev] Open Unit: %d\n", unitNum);
    TimerBase->Device.dd_Library.lib_OpenCnt++;
    TimerBase->Device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
    if (unitNum >= UNIT_MICROHZ && unitNum <= UNIT_WAITECLOCK)
    {
		ioreq->io_Error = 0;
	    ioreq->io_Unit = (struct  Unit *)&TimerBase->TimerUnit[unitNum];
	    ioreq->io_Device = (struct Device *)TimerBase;
	}
	else
	{
		ioreq->io_Error = IOERR_OPENFAIL;
	}
	//DPrintF("[TimerDev] Open Unit: %d\n", unitNum);
	return(TimerBase);
}

APTR timer_CloseDev(struct TimerBase *TimerBase, struct IORequest *ioreq)
{
	TimerBase->Device.dd_Library.lib_OpenCnt--;
	if(!TimerBase->Device.dd_Library.lib_OpenCnt)
	{
		// Should we "expunge" the device?
	}
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return (TimerBase);
}

APTR timer_ExpungeDev(struct TimerBase *TimerBase)
{
	return (NULL);
}

APTR timer_ExtFuncDev(void)
{
	return (NULL);
}



