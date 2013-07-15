#ifndef lib_virtio_internal_h
#define lib_virtio_internal_h

#include "types.h"
#include "library.h"

#define LIB_VIRTIO_VERSION  0
#define LIB_VIRTIO_REVISION 1

#define LIB_VIRTIO_LIBNAME "lib_virtio"
#define LIB_VIRTIO_LIBVER  " 0.1 __DATE__"


typedef struct LibVirtioBase
{
	struct Library		Library;
	APTR				SysBase;

} LibVirtioBase;


extern char LibVirtioLibName[];
extern char LibVirtioLibVer[];


//functions
struct LibVirtioBase *lib_virtio_OpenLib(struct LibVirtioBase *LibVirtioBase);
APTR lib_virtio_CloseLib(struct LibVirtioBase *LibVirtioBase);
APTR lib_virtio_ExpungeLib(struct LibVirtioBase *LibVirtioBase);
APTR lib_virtio_ExtFuncLib(void);

int lib_virtio_add(struct LibVirtioBase *LibVirtioBase, int a, int b);
int lib_virtio_sub(struct LibVirtioBase *LibVirtioBase, int a, int b);


//internals
void INTERN_LibVirtioABC(LibVirtioBase *LibVirtioBase);


#endif //lib_virtio_internal_h


