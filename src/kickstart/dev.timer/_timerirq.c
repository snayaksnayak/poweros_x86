#include "timer_intern.h"
#include "list.h"
#include "exec_funcs.h"

#define FastAddTime(d, s)\
    (d)->tv_micro += (s)->tv_micro;\
    (d)->tv_secs += (s)->tv_secs;\
    if((d)->tv_micro > 999999) {\
	(d)->tv_secs++;\
	(d)->tv_micro -= 1000000;\
    }

__attribute__((no_instrument_function)) BOOL TimerVBLIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase)
{
	DPrintF("TimerVBLIRQServer\n");
	FastAddTime(&TimerBase->CurrentTime, &TimerBase->VBlankTime);
	FastAddTime(&TimerBase->Elapsed, &TimerBase->VBlankTime);

	//DPrintF("TimerBase->CurrentTime->tv_secs = %d\n", TimerBase->CurrentTime.tv_secs);
	//DPrintF("TimerBase->CurrentTime->tv_micro = %d\n", TimerBase->CurrentTime.tv_micro);
	//DPrintF("TimerBase->Elapsed->tv_secs = %d\n", TimerBase->Elapsed.tv_secs);
	//DPrintF("TimerBase->Elapsed->tv_micro = %d\n", TimerBase->Elapsed.tv_micro);

	struct TimeRequest *tr, *trtmp;
	for (int unit_num=0; unit_num < UNIT_MAX; unit_num++)
	{
		switch(unit_num)
		{
		case UNIT_VBLANK:
		case UNIT_MICROHZ:
		case UNIT_ECLOCK:
			ForeachNodeSafe(&TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_MsgList, tr, trtmp)
			{
				if ((tr->tr_time.tv_secs < TimerBase->Elapsed.tv_secs)
				 ||((tr->tr_time.tv_secs <= TimerBase->Elapsed.tv_secs)
				 && (tr->tr_time.tv_micro < TimerBase->Elapsed.tv_micro)))
				{
					DPrintF("Found something from delay\n");
					Remove((struct Node *)tr);
					tr->tr_time.tv_secs = tr->tr_time.tv_micro = 0;
					tr->tr_node.io_Error = 0;
					ReplyMsg((struct Message *)tr);
				}
			}
			break;
		case UNIT_WAITUNTIL:
		case UNIT_WAITECLOCK:
			ForeachNodeSafe(&TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_MsgList, tr, trtmp)
			{
				if ((tr->tr_time.tv_secs < TimerBase->CurrentTime.tv_secs)
				 ||((tr->tr_time.tv_secs <= TimerBase->CurrentTime.tv_secs)
				 && (tr->tr_time.tv_micro < TimerBase->CurrentTime.tv_micro)))
				{
					DPrintF("Found something from alarm\n");
					Remove((struct Node *)tr);
					tr->tr_time.tv_secs = tr->tr_time.tv_micro = 0;
					tr->tr_node.io_Error = 0;
					ReplyMsg((struct Message *)tr);
				}
			}
			break;
		};


	}

	return 0; // we return 0 so that Tick() can run, otherwise we would cut off Schedule()
}

__attribute__((no_instrument_function)) BOOL TimerRTCIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase)
{
	return 0; // we return 0 so that Tick() can run, otherwise we would cut off Schedule()
}
