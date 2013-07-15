#ifndef lib_virtio_h
#define lib_virtio_h

// this file shall go to top most include folder in future

#include "types.h"


//library functions
INT32 add(INT32 a, INT32 b);
INT32 sub(INT32 a, INT32 b);

//vectors
#define add(a,b) (((INT32(*)(APTR, INT32, INT32)) _GETVECADDR(LibVirtioBase, 5))(LibVirtioBase, a, b))
#define sub(a,b) (((INT32(*)(APTR, INT32, INT32)) _GETVECADDR(LibVirtioBase, 6))(LibVirtioBase, a, b))



#endif //lib_virtio_h
