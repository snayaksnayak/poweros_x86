#include "timer_intern.h"
#include "sysbase.h"
#include "resident.h"

// Include our Protos
#include "exec_funcs.h"

static char DevName[] = "timer.device";
static char Version[] = "\0$VER: timer.device 0.1 ("__DATE__")\r\n";
#define TIMER_INT_PRI 0
#define TICK  100
#define IRQ_CLK  0
#define IRQ_RTC  8


struct TimerBase *timer_OpenDev(struct TimerBase *TimerBase, struct IORequest *ioreq, UINT32 unitNum, UINT32 flags);
APTR timer_CloseDev(struct TimerBase *TimerBase, struct IORequest *ioreq);
UINT32 *timer_ExpungeDev(struct TimerBase *TimerBase);
UINT32 timer_ExtFuncDev(void);
void timer_BeginIO(TimerBase *TimerBase, struct IORequest *ioreq);
void timer_AbortIO(TimerBase *TimerBase, struct IORequest *ioreq);
INT32 timer_CmpTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_AddTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_SubTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_GetSysTime(struct TimerBase *TimerBase, struct TimeVal *src);
UINT32 timer_ReadEClock(struct TimerBase *TimerBase, struct EClockVal *src);
__attribute__((no_instrument_function)) BOOL TimerVBLIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase);
__attribute__((no_instrument_function)) BOOL TimerMICROHZIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase);
void set_timer(UINT32 hz);
UINT16 get_timer();
void start_pit_mode_4();

APTR timer_FuncTab[] =
{
 (void(*)) timer_OpenDev,
 (void(*)) timer_CloseDev,
 (void(*)) timer_ExpungeDev,
 (void(*)) timer_ExtFuncDev,

 (void(*)) timer_BeginIO,
 (void(*)) timer_AbortIO,

 (void(*)) timer_AddTime,
 (void(*)) timer_CmpTime,
 (void(*)) timer_SubTime,
 (void(*)) timer_GetSysTime,
 (void(*)) timer_ReadEClock,

 (APTR) ((UINT32)-1)
};

struct TimerBase *timer_InitDev(struct TimerBase *TimerBase, UINT32 *segList, struct SysBase *SysBase)
{
	TimerBase->Timer_SysBase = SysBase;

	for(int unit_num=0; unit_num < UNIT_MAX; unit_num++)
	{
		// Initialise Unit Command Queue
		NewList((struct List *)&TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_MsgList);
		TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)DevName;
		TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
		TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_SigTask = NULL;

		// Initialize different behaviours of Units
		switch(unit_num)
		{
		case UNIT_VBLANK:
		case UNIT_ECLOCK:
			TimerBase->TimerUnit[unit_num].AddRequest = AddDelay;
			break;
		case UNIT_MICROHZ:
			TimerBase->TimerUnit[unit_num].AddRequest = AddMHZDelay;
			break;
		case UNIT_WAITUNTIL:
		case UNIT_WAITECLOCK:
			TimerBase->TimerUnit[unit_num].AddRequest = AddAlarm;
			break;
		};
	}

	//VBL Timer
	TimerBase->TimerVBLIntServer = CreateIntServer(DevName, TIMER_INT_PRI, TimerVBLIRQServer, TimerBase);
	AddIntServer(IRQ_RTC, TimerBase->TimerVBLIntServer);

	//MICROHZ Timer
	TimerBase->TimerMICROHZIntServer = CreateIntServer(DevName, TIMER_INT_PRI, TimerMICROHZIRQServer, TimerBase);
	AddIntServer(IRQ_CLK, TimerBase->TimerMICROHZIntServer);
	start_pit_mode_4();
	//set_timer(0);
/*
	set_timer(0);
	UINT16 val = 0;
	val = get_timer();
	DPrintF("Now val = %d\n", val);
	val = get_timer();
	DPrintF("Now val = %d\n", val);
	set_timer(40000);
	val = get_timer();
	DPrintF("Now val = %d\n", val);
	val = get_timer();
	DPrintF("Now val = %d\n", val);
*/

	return TimerBase;
}

static const struct TimerBase TimerDevData =
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

	.CurrentTime.tv_secs  = 0,
	.CurrentTime.tv_micro = 0,

	.VBlankTime.tv_secs = 0,
	.VBlankTime.tv_micro = 1000000/TICK,

	.Elapsed.tv_secs = 0,
	.Elapsed.tv_micro = 0
};

// ROMTAG Resident
struct InitTable
{
	UINT32	LibBaseSize;
	APTR	FunctionTable;
	APTR	DataTable;
	APTR	InitFunction;
} timer_InitTab =
{
	sizeof(struct TimerBase),
	timer_FuncTab,
	(APTR)&TimerDevData,
	timer_InitDev
};
static APTR TimerEndResident;

struct Resident TimerRomTag =
{
	RTC_MATCHWORD,
	&TimerRomTag,
	&TimerEndResident,
	RTF_AUTOINIT | RTF_COLDSTART,
	VERSION,
	NT_DEVICE,
	50,
	DevName,
	Version,
	0,
	&timer_InitTab
};

