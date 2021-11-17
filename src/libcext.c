#include "libcext.h"

//Use hidden internal function from libultra
extern int _Printf(char *(*copyfunc)(char *, char *, size_t), void*, const char*, va_list);

//Fake debug hardware registers
#define DBG_MAGIC (*(volatile u32 *)0xBFF00000)
#define DBG_TEXTLEN (*(volatile u32 *)0xBFF00004)
#define DBG_TEXTBUF (*(volatile u32 *)0xBFF00008)

#define DBG_HW_PRESENT 0x44424748

//Linker script export
extern char _heap_start[];

//Copied from libdragon
void *sbrk(int incr)
{
    static char *heap_end = 0;
    static char *heap_top = 0;
    char *prev_heap_end;
	
	//Disable interrupts
	OSIntMask prev_mask = osSetIntMask(OS_IM_NONE);
	
    if(heap_end == 0)
    {
		//Initialize heap ranges
        heap_end = _heap_start;
        heap_top = OS_PHYSICAL_TO_K0(osMemSize);
    }
	//Increment heap
    prev_heap_end = heap_end;
    heap_end += incr;

    // check if out of memory
    if(heap_end > heap_top)
    {
        heap_end -= incr;
        prev_heap_end = (char *)-1;
    }
	//Reenable interrupts
	osSetIntMask(prev_mask);
	
    return (void *)prev_heap_end;
}

void *memset(void *str, int c, size_t n)
{
	char *ptr = str;
	for(size_t i=0; i<n; i++) {
		*ptr++ = c;
	}
	return ptr;
}

int memcmp(const void *str1, const void *str2, size_t n)
{
	const unsigned char *ptr1 = (const unsigned char *)str1;
	const unsigned char *ptr2 = (const unsigned char *)str2;
	for(size_t i=0; i<n; i++) {
		if(*ptr1 != *ptr2) {
			return *ptr1-*ptr2;
		}
		ptr1++;
		ptr2++;
	}
	return 0;
}

int strncmp(const unsigned char *str1, const unsigned char *str2, size_t n)
{
	for(size_t i=0; i<n; i++) {
		if(*str1 != *str2 || *str1 == 0) {
			return *str1-*str2;
		}
		str1++;
		str2++;
	}
	return 0;
}

int strcmp(const unsigned char *str1, const unsigned char *str2)
{
	while(*str1 == *str2 && *str1 != 0) {
		str1++;
		str2++;
	}
	return *str1-*str2;
}

static char *WriteDebug(char *dst, char *src, size_t len)
{
	//Send text buffer to fake debug hardware
	DBG_TEXTBUF = (u32)src;
	DBG_TEXTLEN = len;
	return src+len;
}

static char *WriteBuf(char *dst, char *src, size_t len)
{
    return (char *)memcpy(dst, src, len) + len;
}

//vsnprintf buffer parameters
static char *vsnprintf_buf;
static size_t vsnprintf_len;

static char *WriteVsnprintf(char *dst, char *src, size_t len)
{
	if(vsnprintf_len == 0) {
		//Special Case for 0-length buffer
		return src+len;
	} else {
		char *dst_end = vsnprintf_buf+vsnprintf_len;
		if(dst > dst_end) {
			//Out of buffer space
			return dst+len;
		} else {
			if(dst+len > dst_end) {
				//Will be out of buffer space
				return (char *)memcpy(dst, src, dst_end-dst) + len;
			} else {
				//Still have buffer space
				return (char *)memcpy(dst, src, len) + len;
			}
		}
	}
}

int vsnprintf(char *str, size_t n, const char *format, va_list arg)
{
	int written;
	vsnprintf_buf = str;
	vsnprintf_len = n;
    written = _Printf(WriteVsnprintf, str, format, arg);
    if(n != 0 && written >= 0 && written < n) {
        str[written] = 0;
    }
    return written;
}

int snprintf(char *str, size_t n, const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vsnprintf(str, n, format, arg);
	va_end(arg);
}

int vsprintf(char *str, const char *format, va_list arg)
{
	int written;
	written = _Printf(WriteBuf, str, format, arg);
	if (written >= 0) {
		str[written] = 0;
	}
	return written;
}

void vprintf(const char *format, va_list arg)
{
	if(DBG_MAGIC == DBG_HW_PRESENT) {  //Check if fake debug hardware is present
		_Printf(WriteDebug, NULL, format, arg);
	}
}

void printf(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
}