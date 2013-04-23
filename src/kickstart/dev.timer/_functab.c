#include "timer_intern.h"

#undef SysBase
#include "sysbase.h"
#include "resident.h"

// Include our Protos
#include "exec_funcs.h"

//#define TIMERNAME "timer.device"

static char DevName[] = "timer.device";
static char Version[] = "\0$VER: timer.device 0.1 ("__DATE__")\r\n";
#define TIMER_INT_PRI 0
#define TICK  100
#define IRQ_CLK  0

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
__attribute__((no_instrument_function)) BOOL TimerVBLIRQServer(UINT32 number, APTR regs, TimerBase *TimerBase, APTR SysBase);
//__attribute__((no_instrument_function)) BOOL Timer3IRQServer(UINT32 number, istate* istate, TimerBase *TimerBase, APTR SysBase);


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

	NewList((struct List *) &TimerBase->Lists[UNIT_MICROHZ] );
	NewList((struct List *) &TimerBase->Lists[UNIT_VBLANK] );
	NewList((struct List *) &TimerBase->Lists[UNIT_ECLOCK] );
	NewList((struct List *) &TimerBase->Lists[UNIT_WAITUNTIL] );
	NewList((struct List *) &TimerBase->Lists[UNIT_WAITECLOCK] );

	//DPrintF("[Timer] TimerBase: %x, OpenDevice: %x\n", TimerBase, timer_BeginIO);
	// VBL (100hz) Timer
	TimerBase->TimerVBLIntServer = CreateIntServer(DevName, TIMER_INT_PRI, TimerVBLIRQServer, TimerBase);
	AddIntServer(IRQ_CLK, TimerBase->TimerVBLIntServer);

	// EClock Timer
//	TimerBase->Timer1IntServer = CreateIntServer(DevName, TIMER_INT_PRI, Timer1IRQServer, TimerBase);
//	AddIntServer(IRQ_TIMER1, TimerBase->Timer1IntServer);

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
	.VBlankTime.tv_micro = 1000000/TICK, //STC_FREQ_HZ / TICK; //TimerPeriod;

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

