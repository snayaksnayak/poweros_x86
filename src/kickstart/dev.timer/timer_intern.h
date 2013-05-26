#ifndef timer_h
#define timer_h

#include "types.h"
#include "device.h"
#include "io.h"

#define IRQ_RTC 8

/* Units */
#define UNIT_MICROHZ    0
#define UNIT_VBLANK     1
#define UNIT_ECLOCK     2
#define UNIT_WAITUNTIL  3
#define UNIT_WAITECLOCK 4
#define UNIT_MAX		5

/* IO-Commands */
#define TR_ADDREQUEST (CMD_NONSTD+0)
#define TR_GETSYSTIME (CMD_NONSTD+1)
#define TR_SETSYSTIME (CMD_NONSTD+2)

#define SysBase TimerBase->Timer_SysBase

#define VERSION  0
#define REVISION 2

#define DEVNAME "timer"
#define DEVVER  " 0.2 __DATE__"


struct EClockVal
{
    UINT32 ev_hi;
    UINT32 ev_lo;
};

struct TimeVal
{
	UINT32	tv_secs;
	UINT32	tv_micro;
};

struct TimeRequest
{
    struct IORequest tr_node;
    struct TimeVal   tr_time;
};

struct TimerUnit
{
	struct Unit tu_unit;
	APTR AddRequest;
};

typedef struct TimerBase
{
	struct Device		Device;
	APTR				Timer_SysBase;
	UINT32				MiscFlags;
	struct TimeVal		CurrentTime;
	struct TimeVal		VBlankTime;
	struct TimeVal		Elapsed;

	UINT32				TimerIRQ;
	struct Interrupt	*TimerVBLIntServer;
	struct Interrupt	*TimerECLOCKIntServer;
	struct TimeVal		TimerIntTime;

	struct TimerUnit	TimerUnit[UNIT_MAX];

} TimerBase;

struct rtc_time
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

extern void (*TimerCmdVector[])(TimerBase *, struct IORequest *);
//extern INT8 TimerCmdQuick[];

void INTERN_EndCommand(TimerBase *TimerBase, UINT32 error, struct IORequest *ioreq);
void INTERN_QueueRequest(TimerBase *TimerBase, struct IORequest *ioreq);

void timer_BeginIO(TimerBase *TimerBase, struct IORequest *ioreq);
void timer_AbortIO(TimerBase *TimerBase, struct IORequest *ioreq);
INT32 timer_CmpTime(TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_AddTime(TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_SubTime(TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_GetSysTime(struct TimerBase *TimerBase, struct TimeVal *src);
UINT32 timer_ReadEClock(struct TimerBase *TimerBase, struct EClockVal *src);

void TimerInvalid(TimerBase *TimerBase, struct IORequest *ioreq);
void TimerAddRequest(TimerBase *TimerBase, struct IORequest *ioreq);
void TimerGetSysTime(TimerBase *TimerBase, struct IORequest *ioreq);
void TimerSetSysTime(TimerBase *TimerBase, struct IORequest *ioreq);

void AddAlarm(TimerBase *TimerBase, struct IORequest *ioreq);
void AddDelay(TimerBase *TimerBase, struct IORequest *ioreq);

#endif


