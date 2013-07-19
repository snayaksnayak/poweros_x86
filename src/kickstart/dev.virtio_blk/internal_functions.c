#include "exec_funcs.h"
#include "virtio_blk_internal.h"

#define SysBase VirtioBlkBase->VirtioBlk_SysBase


void virtio_write8(UINT16 base, UINT16 offset, UINT8 val)
{
	IO_Out8(base+offset, val);
}
void virtio_write16(UINT16 base, UINT16 offset, UINT16 val)
{
	IO_Out16(base+offset, val);
}
void virtio_write32(UINT16 base, UINT16 offset, UINT32 val)
{
	IO_Out32(base+offset, val);
}

UINT8 virtio_read8(UINT16 base, UINT16 offset)
{
	return IO_In8(base+offset);
}
UINT16 virtio_read16(UINT16 base, UINT16 offset)
{
	return IO_In16(base+offset);
}
UINT32 virtio_read32(UINT16 base, UINT16 offset)
{
	return IO_In32(base+offset);
}



void VirtioBlk_queue_command(VirtioBlkBase *VirtioBlkBase, struct IOStdReq *ioreq)
{
	UINT32 ipl = Disable();

	struct Unit	*unit = ioreq->io_Unit;

	if (ioreq->io_Error == 0)
	{
		PutMsg(&unit->unit_MsgPort, &ioreq->io_Message);
	}
	Enable(ipl);
	return;
}

void VirtioBlk_end_command(VirtioBlkBase *VirtioBlkBase, UINT32 error, struct IOStdReq *ioreq)
{
	ioreq->io_Error = error;
	if (TEST_BITS(ioreq->io_Flags, IOF_QUICK)) return;
	ReplyMsg((struct Message *)ioreq);
	return;
}


void VirtioBlk_exchange_features(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vb)
{
	UINT32 guest_features = 0, host_features = 0;
	virtio_feature *f;

	//collect host features
	host_features = virtio_read32(vb->io_addr, VIRTIO_HOST_F_OFFSET);

	for (int i = 0; i < vb->num_features; i++)
	{
		f = &vb->features[i];

		// prepare the features the guest/driver supports
		guest_features |= (f->guest_support << f->bit);
		DPrintF("guest feature %d\n", (f->guest_support << f->bit));

		// just load the host/device feature int the struct
		f->host_support |=  ((host_features >> f->bit) & 1);
		DPrintF("host feature %d\n\n", ((host_features >> f->bit) & 1));
	}

	// let the device know about our features
	virtio_write32(vb->io_addr, VIRTIO_GUEST_F_OFFSET, guest_features);
}


void VirtioBlk_free_phys_queue(VirtioBlkBase *VirtioBlkBase, struct virtio_queue *q)
{
	FreeVec(q->unaligned_addr);
	q->paddr = 0;
	q->num = 0;
	FreeVec(q->data);
	q->data = NULL;
}

int VirtioBlk_alloc_phys_queue(VirtioBlkBase *VirtioBlkBase, struct virtio_queue *q)
{
	/* How much memory do we need? */
	q->ring_size = vring_size(q->num, 4096);
	DPrintF("q->ring_size (%d)\n", q->ring_size);

	UINT32 addr;
	q->unaligned_addr = AllocVec(q->ring_size, MEMF_FAST|MEMF_CLEAR);
	DPrintF("q->unaligned_addr (%x)\n", q->unaligned_addr);
	DPrintF("q->unaligned_addr + q->ring_size = (%x)\n", q->unaligned_addr + q->ring_size);

	addr = (UINT32)q->unaligned_addr & 4095;
	DPrintF("addr (%x)\n", addr);
	addr = (UINT32)q->unaligned_addr - addr;
	DPrintF("addr (%x)\n", addr);
	addr = addr + 4096;
	DPrintF("addr (%x)\n", addr);

	q->paddr = (void*)addr;

	if (q->unaligned_addr == NULL)
		return 0;

	q->data = AllocVec(sizeof(q->data[0]) * q->num, MEMF_FAST|MEMF_CLEAR);

	if (q->data == NULL) {
		FreeVec(q->unaligned_addr);
		q->unaligned_addr = NULL;
		q->paddr = 0;
		return 0;
	}

	return 1;
}

void VirtioBlk_init_phys_queue(VirtioBlkBase *VirtioBlkBase, struct virtio_queue *q)
{
	//not needed
	//memset(q->vaddr, 0, q->ring_size);
	//memset(q->data, 0, sizeof(q->data[0]) * q->num);

	/* physical page in guest */
	q->page = (UINT32)q->paddr / 4096;

	/* Set pointers in q->vring according to size */
	vring_init(&q->vring, q->num, q->paddr, 4096);

	DPrintF("vring desc %x\n", q->vring.desc);
	DPrintF("vring avail %x\n", q->vring.avail);
	DPrintF("vring used %x\n", q->vring.used);

/*
	// Everything's free at this point
	for (int i = 0; i < q->num; i++) {
		q->vring.desc[i].flags = VRING_DESC_F_NEXT;
		q->vring.desc[i].next = (i + 1) & (q->num - 1);
	}
*/

	q->free_num = q->num;
	q->free_head = 0;
	q->free_tail = q->num - 1;
	q->last_used = 0;

	return;
}


int VirtioBlk_init_phys_queues(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vb)
{
	/* Initialize all queues */
	int i, j, r;
	struct virtio_queue *q;

	for (i = 0; i < vb->num_queues; i++)
	{
		q = &vb->queues[i];

		/* select the queue */
		virtio_write16(vb->io_addr, VIRTIO_QSEL_OFFSET, i);
		q->num = virtio_read16(vb->io_addr, VIRTIO_QSIZE_OFFSET);
		DPrintF("Queue %d, q->num (%d)\n", i, q->num);
		if (q->num & (q->num - 1)) {
			DPrintF("Queue %d num=%d not ^2", i, q->num);
			r = 0;
			goto free_phys_queues;
		}

		r = VirtioBlk_alloc_phys_queue(VirtioBlkBase,q);

		if (r != 1)
			goto free_phys_queues;

		VirtioBlk_init_phys_queue(VirtioBlkBase, q);

		/* Let the host know about the guest physical page */
		virtio_write32(vb->io_addr, VIRTIO_QADDR_OFFSET, q->page);
	}

	return 1;

/* Error path */
free_phys_queues:
	for (j = 0; j < i; j++)
	{
		VirtioBlk_free_phys_queue(VirtioBlkBase, &vb->queues[i]);
	}

	return r;
}


int VirtioBlk_alloc_phys_queues(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vb, int num_queues)
{
	int r = 1;

	// Assume there's no device with more than 256 queues
	if (num_queues < 0 || num_queues > 256)
		return 0;

	vb->num_queues = num_queues;

	// allocate queue memory
	vb->queues = AllocVec(num_queues * sizeof(vb->queues[0]), MEMF_FAST|MEMF_CLEAR);

	if (vb->queues == NULL)
		return 0;

	//not needed in because of MEMF_CLEAR
	//memset(dev->queues, 0, num_queues * sizeof(dev->queues[0]));

	r = VirtioBlk_init_phys_queues(VirtioBlkBase, vb);

	if ((r != 1)) {
		DPrintF("Could not initialize queues (%d)\n", r);
		FreeVec(vb->queues);
		vb->queues = NULL;
	}

	return r;
}


void VirtioBlk_free_phys_queues(VirtioBlkBase *VirtioBlkBase, VirtioBlk *dev)
{
	int i;
	for (i = 0; i < dev->num_queues; i++)
	{
		VirtioBlk_free_phys_queue(VirtioBlkBase, &dev->queues[i]);
	}

	dev->num_queues = 0;
	dev->queues = NULL;
}

int VirtioBlk_alloc_phys_requests(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vb)
{
	/* Allocate memory for request headers and status field */

	vb->hdrs = AllocVec(VIRTIO_BLK_NUM_QUEUES * sizeof(vb->hdrs[0]),
				MEMF_FAST|MEMF_CLEAR);

	if (!vb->hdrs)
		return 0;

	vb->status = AllocVec(VIRTIO_BLK_NUM_QUEUES * sizeof(vb->status[0]),
				  MEMF_FAST|MEMF_CLEAR);

	if (!vb->status) {
		FreeVec(vb->hdrs);
		return 0;
	}

	return 1;
}

int VirtioBlk_supports(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vb, int bit, int host)
{
	for (int i = 0; i < vb->num_features; i++)
	{
		struct virtio_feature *f = &vb->features[i];

		if (f->bit == bit)
			return host ? f->host_support : f->guest_support;
	}
	DPrintF("ERROR: Bit not found!!\n");
	return 0;
}



int VirtioBlk_host_supports(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vb, int bit)
{
	return VirtioBlk_supports(VirtioBlkBase, vb, bit, 1);
}

int VirtioBlk_guest_supports(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vb, int bit)
{
	return VirtioBlk_supports(VirtioBlkBase, vb, bit, 0);
}


int VirtioBlk_configuration(VirtioBlkBase *VirtioBlkBase, VirtioBlk *vb)
{
	UINT32 sectors_low, sectors_high, size_mbs;

	/* capacity is always there */
	sectors_low = virtio_read32(vb->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+0);
	sectors_high = virtio_read32(vb->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+4);
	vb->Info.capacity = ((UINT64)sectors_high << 32) | sectors_low;

	/* If this gets truncated, you have a big disk... */
	size_mbs = (UINT32)(vb->Info.capacity * 512 / 1024 / 1024);
	DPrintF("Capacity: %d MB\n", size_mbs);

	if (VirtioBlk_host_supports(VirtioBlkBase, vb, VIRTIO_BLK_F_GEOMETRY))
	{
		vb->Info.geometry.cylinders = virtio_read16(vb->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+16);
		vb->Info.geometry.heads = virtio_read8(vb->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+18);
		vb->Info.geometry.sectors = virtio_read8(vb->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+19);

		DPrintF("Geometry: cyl=%d heads=%d sectors=%d\n",
					vb->Info.geometry.cylinders,
					vb->Info.geometry.heads,
					vb->Info.geometry.sectors);
	}

	if (VirtioBlk_host_supports(VirtioBlkBase, vb, VIRTIO_BLK_F_BLK_SIZE))
	{
		vb->Info.blk_size = virtio_read32(vb->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+20);
		DPrintF("Block Size: %d\n", vb->Info.blk_size);
	}

	return 0;
}

void test_mhz_delay(APTR, int);
void VirtioBlk_transfer(VirtioBlkBase *VirtioBlkBase, VirtioBlk* vb, UINT32 sector_num, UINT8 write, UINT8* buf)
{
	//prepare first out_hdr, since we have only one, replace 0 by a variable
	//memset(&hdrs[0], 0, sizeof(hdrs[0]));

	if(write == 1)
	{
		//for writing to disk
		vb->hdrs[0].type = VIRTIO_BLK_T_OUT;
	}
	else
	{
		//for reading from disk
		vb->hdrs[0].type = VIRTIO_BLK_T_IN;
	}

	//fill up sector
	vb->hdrs[0].ioprio = 0;
	vb->hdrs[0].sector = sector_num;

	//clear status
	vb->status[0] = 1; //0 means success, 1 means error, 2 means unsupported

	DPrintF("\n\n\nsector = %d\n", sector_num);
	DPrintF("idx = %d\n", (vb->queues[0]).vring.avail->idx);


	//fill into descriptor table
	(vb->queues[0]).vring.desc[0].addr = (UINT32)&(vb->hdrs[0]);
	(vb->queues[0]).vring.desc[0].len = sizeof(vb->hdrs[0]);
	(vb->queues[0]).vring.desc[0].flags = VRING_DESC_F_NEXT;
	(vb->queues[0]).vring.desc[0].next = 1;

	(vb->queues[0]).vring.desc[1].addr = (UINT32)buf;
	(vb->queues[0]).vring.desc[1].len = 512;
	(vb->queues[0]).vring.desc[1].flags = VRING_DESC_F_NEXT | VRING_DESC_F_WRITE;
	(vb->queues[0]).vring.desc[1].next = 2;

	(vb->queues[0]).vring.desc[2].addr = (UINT32)&(vb->status[0]);
	(vb->queues[0]).vring.desc[2].len = sizeof(vb->status[0]);
	(vb->queues[0]).vring.desc[2].flags = VRING_DESC_F_WRITE;
	(vb->queues[0]).vring.desc[2].next = 0;

	//fill in available ring
	(vb->queues[0]).vring.avail->flags = 0; //1 mean no interrupt needed, 0 means interrupt needed
	(vb->queues[0]).vring.avail->ring[0] = 0; // 0 is the head of above request descriptor chain
	(vb->queues[0]).vring.avail->idx = (vb->queues[0]).vring.avail->idx + 3; //next available descriptor

	//notify
	virtio_write16(vb->io_addr, VIRTIO_QNOTFIY_OFFSET, 0); //notify that 1st queue (0) of this device has been updated

	//give some delay
	test_mhz_delay(SysBase, 1);

	DPrintF("vb->status[0] %d\n", vb->status[0]);

/*
	int j=0;

	DPrintF("(vb->queues[0]).vring.used->flags %d\n", (vb->queues[0]).vring.used->flags);
	DPrintF("(vb->queues[0]).vring.used->idx %d\n", (vb->queues[0]).vring.used->idx);
	for(j=0; j<(vb->queues[0]).num;j++)
	{
		DPrintF("(vb->queues[0]).vring.used->ring[%d].id %d\n", j, (vb->queues[0]).vring.used->ring[j].id);
		DPrintF("(vb->queues[0]).vring.used->ring[%d].len %d\n", j, (vb->queues[0]).vring.used->ring[j].len);
	}

	DPrintF("(vb->queues[0]).vring.avail->flags %d\n", (vb->queues[0]).vring.avail->flags);
	DPrintF("(vb->queues[0]).vring.avail->idx %d\n", (vb->queues[0]).vring.avail->idx);
	for(j=0; j<(vb->queues[0]).num;j++)
	{
		DPrintF("(vb->queues[0]).vring.avail->ring[%d] %d\n", j, (vb->queues[0]).vring.avail->ring[j]);
	}
*/

	//See if virtio device generated an interrupt(1) or not(0)
	UINT8 isr;
	isr=virtio_read8(vb->io_addr, VIRTIO_ISR_STATUS_OFFSET);
	DPrintF("virtio_blk_transfer: isr= %d\n", isr);

	DPrintF("virtio_blk_transfer: buf[0]= %x\n", buf[0]);
	DPrintF("virtio_blk_transfer: buf[1]= %x\n", buf[1]);
	DPrintF("virtio_blk_transfer: buf[2]= %x\n", buf[2]);
	DPrintF("virtio_blk_transfer: buf[3]= %x\n", buf[3]);

}
