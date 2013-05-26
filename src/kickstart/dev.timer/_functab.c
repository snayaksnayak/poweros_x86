#include "timer_intern.h"

#undef SysBase
#include "sysbase.h"
#include "resident.h"

// Include our Protos
#include "exec_funcs.h"

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
__attribute__((no_instrument_function)) BOOL TimerVBLIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase);
__attribute__((no_instrument_function)) BOOL TimerRTCIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase);


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

static __inline__ void
outb(UINT16 port, UINT8 value)
{
   __asm__ __volatile__ ("outb %0, %1" : :"a" (value), "d" (port));
}

static __inline__ UINT8
inb(UINT16 port)
{
   UINT8 value;
   __asm__ __volatile__ ("inb %1, %0" :"=a" (value) :"d" (port));
   return value;
}

void NMI_enable(void)
{
	outb(0x70, inb(0x70)&0x7F);
}

void NMI_disable(void)
{
	outb(0x70, inb(0x70)|0x80);
}



void start_irq_8(struct TimerBase *TimerBase)
{
	SysBase* SysBase;
	SysBase = TimerBase->Timer_SysBase;
	UINT32 ipl;
	UINT8 rate = 15; //9
	UINT8 prevA, prevB;

	ipl = Disable();

NMI_disable();
outb(0x70, 0x8A);		// set index to register A, disable NMI
prevA=inb(0x71);	// get initial value of register A
outb(0x70, 0x8A);// reset index to A
outb(0x71, (prevA & 0xF0) | (rate & 0x0F)); //write only our rate to A. Note, rate is the bottom 4 bits.

outb(0x70, 0x8B);		// select register B, and disable NMI
prevB=inb(0x71);	// read the current value of register B
outb(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
outb(0x71, prevB | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
NMI_enable();

	Enable(ipl);


}

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
		case UNIT_MICROHZ:
		case UNIT_ECLOCK:
			TimerBase->TimerUnit[unit_num].AddRequest = AddDelay;
			break;
		case UNIT_WAITUNTIL:
		case UNIT_WAITECLOCK:
			TimerBase->TimerUnit[unit_num].AddRequest = AddAlarm;
			break;
		};
	}

	TimerBase->TimerVBLIntServer = CreateIntServer(DevName, TIMER_INT_PRI, TimerVBLIRQServer, TimerBase);
	AddIntServer(IRQ_CLK, TimerBase->TimerVBLIntServer);

	//EClock Timer
	TimerBase->TimerECLOCKIntServer = CreateIntServer(DevName, TIMER_INT_PRI, TimerRTCIRQServer, TimerBase);
	AddIntServer(IRQ_RTC, TimerBase->TimerECLOCKIntServer);

	start_irq_8(TimerBase);

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

