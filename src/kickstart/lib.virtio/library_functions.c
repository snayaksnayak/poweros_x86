#include "exec_funcs.h"
#include "lib_virtio_internal.h"

#define SysBase LibVirtioBase->SysBase

struct LibVirtioBase* lib_virtio_OpenLib(struct LibVirtioBase *LibVirtioBase)
{
    LibVirtioBase->Library.lib_OpenCnt++;

	return(LibVirtioBase);
}

APTR lib_virtio_CloseLib(struct LibVirtioBase *LibVirtioBase)
{
	LibVirtioBase->Library.lib_OpenCnt--;

	return (LibVirtioBase);
}

APTR lib_virtio_ExpungeLib(struct LibVirtioBase *LibVirtioBase)
{
	return (NULL);
}

APTR lib_virtio_ExtFuncLib(void)
{
	return (NULL);
}

int lib_virtio_add(struct LibVirtioBase *LibVirtioBase, int a, int b)
{
	return 0;
}

int lib_virtio_sub(struct LibVirtioBase *LibVirtioBase, int a, int b)
{
	return 0;
}





