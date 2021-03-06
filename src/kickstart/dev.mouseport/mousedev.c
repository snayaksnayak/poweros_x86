#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "io.h"
#include "inputevent.h"

#include "mouseport.h"

#include "sysbase.h"
#include "exec_funcs.h"

#define DEVICE_VERSION_STRING "\0$VER: mouseport.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static const char name[] = "mouseport.device";
static const char version[] = DEVICE_VERSION_STRING
static const char EndResident;

APTR mdev_OpenDev(struct MDBase *MDBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags);
APTR mdev_CloseDev(struct MDBase *MDBase, struct IORequest *ioreq);
APTR mdev_ExpungeDev(struct MDBase *MDBase);
APTR mdev_ExtFuncDev(struct MDBase *MDBase);
void mdev_BeginIO(MDBase *MDBase, struct IORequest *ioreq);
void mdev_AbortIO(MDBase *MDBase, struct IORequest *ioreq);
static struct MDBase *mdev_Init(struct MDBase *MDBase, UINT32 *segList, struct SysBase *SysBase);
__attribute__((no_instrument_function)) BOOL mouse_handler(UINT32 number, MDBase *MDBase, APTR SysBase);


static APTR FuncTab[] =
{
	(void(*)) mdev_OpenDev,
	(void(*)) mdev_CloseDev,
	(void(*)) mdev_ExpungeDev,
	(void(*)) mdev_ExtFuncDev,

	(void(*)) mdev_BeginIO,
	(void(*)) mdev_AbortIO,
	(APTR) ((UINT32)-1)
};

static const struct MDBase MDLibData =
{
	.Device.dd_Library.lib_Node.ln_Name = (APTR)&name[0],
	.Device.dd_Library.lib_Node.ln_Type = NT_DEVICE,
	.Device.dd_Library.lib_Node.ln_Pri = 60,

	.Device.dd_Library.lib_OpenCnt = 0,
	.Device.dd_Library.lib_Flags = LIBF_SUMUSED|LIBF_CHANGED,
	.Device.dd_Library.lib_NegSize = 0,
	.Device.dd_Library.lib_PosSize = 0,
	.Device.dd_Library.lib_Version = DEVICE_VERSION,
	.Device.dd_Library.lib_Revision = DEVICE_REVISION,
	.Device.dd_Library.lib_Sum = 0,
	.Device.dd_Library.lib_IDString = (APTR)&version[7],

	.Flags = 0
};

static volatile const APTR InitTab[4]=
{
	(APTR)sizeof(struct MDBase),
	(APTR)FuncTab,
	(APTR)&MDLibData,
	(APTR)mdev_Init
};

static volatile const struct Resident ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT | RTF_COLDSTART,
	DEVICE_VERSION,
	NT_DEVICE,
	60,
	(STRPTR)name,
	(STRPTR)&version[7],
	0,
	&InitTab
};

void arch_ps2m_init(void);

static struct MDBase *mdev_Init(struct MDBase *MDBase, UINT32 *segList, struct SysBase *SysBase)
{
	MDBase->SysBase	= SysBase;

	// Initialise Unit Command Queue
	NewList((struct List *)&MDBase->Unit.unit_MsgPort.mp_MsgList);
	MDBase->Unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)name;
	MDBase->Unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	MDBase->Unit.unit_MsgPort.mp_SigTask = NULL; // Important for our Queue Handling
	MDBase->Unit.unit_Flags &= ~DUB_STOPPED;
	MDBase->BufHead = MDBase->BufTail = 0;

	arch_ps2m_init();

	//DPrintF("PS/2 mouse driver installed\n");

	MDBase->IS = CreateIntServer((STRPTR)"IRQ12 mouse.device", IS_PRIORITY, mouse_handler, MDBase);
	AddIntServer(IRQ_MOUSE, MDBase->IS);
	return MDBase;
}

#define SysBase MDBase->SysBase
void mdev_BeginIO(MDBase *MDBase, struct IORequest *io)
{
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;

	if (cmd > MD_SETTRIGGER) cmd = 0; // Invalidate the command.

	if (mouseCmdQuick[cmd] >= 0)
	{
		QueueCommand((struct IOStdReq*)io, SysBase);
		// Check if we are the first in Queue, if not, just return
		if (!TEST_BITS(io->io_Flags, IOF_CURRENT))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
		// If Device is stopped, just return
		if (TEST_BITS(MDBase->Unit.unit_Flags, DUB_STOPPED))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
		// we are first in Queue, now we are Quick, otherwise we come from the IS Routine
	}
	mouseCmdVector[cmd]((struct IOStdReq*) io, MDBase);
}

void mdev_AbortIO(MDBase *MDBase, struct IORequest *ioreq)
{
	EndCommand(IOERR_ABORTED, (struct IOStdReq*)ioreq, SysBase);
}

static const char EndResident = 0;

