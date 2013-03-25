#ifndef timer_h
#define timer_h

#include "types.h"
#include "device.h"
#include "io.h"

/* Units */
#define UNIT_MICROHZ    0
#define UNIT_VBLANK     1
#define UNIT_ECLOCK     2
#define UNIT_WAITUNTIL  3
#define UNIT_WAITECLOCK 4

/* IO-Commands */
#define TR_ADDREQUEST (CMD_NONSTD+0)
#define TR_GETSYSTIME (CMD_NONSTD+1)
#define TR_SETSYSTIME (CMD_NONSTD+2)

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



#endif
