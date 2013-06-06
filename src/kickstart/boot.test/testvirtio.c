#include "types.h"
#include "exec_funcs.h"

#define VIRTIO_VENDOR_ID 0x1af4

BOOL PciFindDeviceByVendorId(UINT16 VendorId, int index, PCI_DEVICE_INFO* Info);

typedef struct PCI_BASE
{
   INT8  Space;
   INT8  Type;
   INT8  Prefetch;
   INT32 Address;
}PCI_BASE;

typedef struct PCI_DEVICE_INFO
{
   UINT16 DeviceId;
   UINT16 VendorId;
   int      ClassCode;
   int      SubClass;
   int    Bus;
   int    Device;
   int    Function;
   PCI_BASE Bases[6];
}PCI_DEVICE_INFO;


void DetectVirtio(APTR SysBase)
{
	PCI_DEVICE_INFO dev;
	int index = 0;

	while (1)
	{
		if (!PciFindDeviceByVendorId(VIRTIO_VENDOR_ID, index, &dev))
		{
			break;
		}
		else
		{
			DPrintF("************got a virtio************\n");
			DPrintF("Vendor id %x\n", dev.VendorId);
			DPrintF("Device id %x\n", dev.DeviceId);
			index++;
		}
   }
}
