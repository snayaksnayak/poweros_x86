#ifndef lib_virtio_h
#define lib_virtio_h

// this file shall go to top most include folder in future

#include "types.h"


//library functions
void VirtioWrite8(UINT16 base, UINT16 offset, UINT8 val);
void VirtioWrite16(UINT16 base, UINT16 offset, UINT16 val);
void VirtioWrite32(UINT16 base, UINT16 offset, UINT32 val);
UINT8 VirtioRead8(UINT16 base, UINT16 offset);
UINT16 VirtioRead16(UINT16 base, UINT16 offset);
UINT32 VirtioRead32(UINT16 base, UINT16 offset);

void VirtioExchangeFeatures(VirtioDevice *vd);
int VirtioAllocateQueues(VirtioDevice *vd, INT32 num_queues);
int VirtioInitQueues(VirtioDevice *vd);
void VirtioFreeQueues(VirtioDevice *vd);
int VirtioHostSupports(VirtioDevice *vd, int bit);
int VirtioGuestSupports(VirtioDevice *vd, int bit);



//vectors
#define VirtioWrite8(a,b,c) (((void(*)(APTR, UINT16, UINT16, UINT8)) 	_GETVECADDR(LibVirtioBase, 5))(LibVirtioBase, a, b, c))
#define VirtioWrite16(a,b,c) (((void(*)(APTR, UINT16, UINT16, UINT16)) 	_GETVECADDR(LibVirtioBase, 6))(LibVirtioBase, a, b, c))
#define VirtioWrite32(a,b,c) (((void(*)(APTR, UINT16, UINT16, UINT32)) 	_GETVECADDR(LibVirtioBase, 7))(LibVirtioBase, a, b, c))
#define VirtioRead8(a,b) (((UINT8(*)(APTR, UINT16, UINT16)) 	_GETVECADDR(LibVirtioBase, 8))(LibVirtioBase, a, b))
#define VirtioRead16(a,b) (((UINT16(*)(APTR, UINT16, UINT16)) 	_GETVECADDR(LibVirtioBase, 9))(LibVirtioBase, a, b))
#define VirtioRead32(a,b) (((UINT32(*)(APTR, UINT16, UINT16)) 	_GETVECADDR(LibVirtioBase, 10))(LibVirtioBase, a, b))

#define VirtioExchangeFeatures(a) (((void(*)(APTR, VirtioDevice*)) 	_GETVECADDR(LibVirtioBase, 11))(LibVirtioBase, a))
#define VirtioAllocateQueues(a,b) (((int(*)(APTR, VirtioDevice*, INT32)) 	_GETVECADDR(LibVirtioBase, 12))(LibVirtioBase, a, b))
#define VirtioInitQueues(a) (((int(*)(APTR, VirtioDevice)) 	_GETVECADDR(LibVirtioBase, 13))(LibVirtioBase, a))
#define VirtioFreeQueues(a) (((void(*)(APTR, VirtioDevice)) 	_GETVECADDR(LibVirtioBase, 14))(LibVirtioBase, a))
#define VirtioHostSupports(a,b) (((int(*)(APTR, VirtioDevice*, int)) 	_GETVECADDR(LibVirtioBase, 15))(LibVirtioBase, a, b))
#define VirtioGuestSupports(a,b) (((int(*)(APTR, VirtioDevice*, int)) 	_GETVECADDR(LibVirtioBase, 16))(LibVirtioBase, a, b))




#endif //lib_virtio_h
