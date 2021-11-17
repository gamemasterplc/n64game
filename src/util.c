#include "util.h"

static u8 read_buf[ROMREAD_BUF_SIZE] __attribute__((aligned(16))); //16-byte aligned buffer for unaligned reads

static OSPiHandle *GetCartHandle()
{
	static OSPiHandle *handle = NULL;
	//Only acquire handle if necessary
	if(!handle) {
		handle = osCartRomInit();
	}
	return handle;
}

void RomRead(void *dst, void *src, u32 len)
{
	OSIoMesg io_mesg;
	OSMesgQueue dma_msg_queue;
	OSMesg dma_msg;
	u32 src_ofs = (u32)src; //Temporary for source offset
	char *dst_ptr = dst; //Use temporary for destination pointer
	//Initialize DMA Status
	osCreateMesgQueue(&dma_msg_queue, &dma_msg, 1);
	io_mesg.hdr.pri = OS_MESG_PRI_NORMAL;
	io_mesg.hdr.retQueue = &dma_msg_queue;
	//Check for direct DMA being possible
	if((((u32)dst_ptr & 0x7) == 0) && (src_ofs & 0x1) == 0 && (len & 0x1) == 0) {
		//Do DMA directly to destination
		if(((u32)dst_ptr & 0xF) == 0 && (len & 0xF) == 0) {
			//Can skip writeback if 16-byte aligned
			osInvalDCache(dst, len);
		} else {
			//Cannot skip writeback
			osWritebackDCache(dst, (len+15) & ~0xF);
			osInvalDCache(dst, (len+15) & ~0xF);
		}
		while(len) {
			//Calculate chunk read length
			u32 copy_len = ROMREAD_BUF_SIZE;
			if(copy_len > len) {
				copy_len = len;
			}
			//Setup DMA params
			io_mesg.dramAddr = dst_ptr;
			io_mesg.devAddr = src_ofs;
			io_mesg.size = copy_len;
			//Start reading from ROM
			osEPiStartDma(GetCartHandle(), &io_mesg, OS_READ);
			//Wait for ROM read to finish
			osRecvMesg(&dma_msg_queue, &dma_msg, OS_MESG_BLOCK);
			//Advance data pointers
			src_ofs += copy_len;
			dst_ptr += copy_len;
			len -= copy_len;
		}
	} else {
		u32 read_buf_offset = src_ofs & 0x1; //Source fixup offset for odd source offset DMAs
		src_ofs &= ~0x1; //Round down source offset
		//Writeback invalidate destination buffer for RCP usage
		osWritebackDCache(dst, (len+15) & ~0xF);
		osInvalDCache(dst, (len+15) & ~0xF);
		//DMA to temporary buffer
		while(len) {
			//Calculate chunk copy length
			u32 copy_len = ROMREAD_BUF_SIZE;
			if(copy_len > len) {
				copy_len = len;
			}
			u32 read_len = (copy_len+15) & ~0xF; //Round read length to nearest cache line
			//Simple invalidate works here since the buffer is aligned to 16 bytes
			osInvalDCache(read_buf, read_len);
			//Setup DMA params
			io_mesg.dramAddr = read_buf;
			io_mesg.devAddr = src_ofs;
			io_mesg.size = read_len;
			//Start reading from ROM
			osEPiStartDma(GetCartHandle(), &io_mesg, OS_READ);
			//Wait for ROM read to finish
			osRecvMesg(&dma_msg_queue, &dma_msg, OS_MESG_BLOCK);
			//Copy from temporary buffer
			bcopy(read_buf+read_buf_offset, dst_ptr, copy_len);
			//Advance data pointers
			src_ofs += copy_len;
			dst_ptr += copy_len;
			len -= copy_len;
		}
	}
}