#include <nusys.h>
#include "libcext.h"
#include "malloc.h"
#include "util.h"
#include "fs.h"

//ROM Symbols for FS
extern u8 fsheader_start[];
extern u8 fsheader_end[];
extern u8 fsdata_start[];

//Struct declarations for FS data
typedef struct fs_entry {
	char *name;
	u32 rom_ofs;
	u32 len;
} FSEntry;

typedef struct fs_header {
	u32 count;
	FSEntry entries[1];
} FSHeader;

static FSHeader *fs_header;

static void FixupHeader()
{
	for(u32 i=0; i<fs_header->count; i++) {
		FSEntry *entry_ptr = &fs_header->entries[i];
		//Make name RAM pointer
		entry_ptr->name = (char *)((u32)fs_header+(u32)entry_ptr->name);
		//Make rom offset direct ROM pointer
		entry_ptr->rom_ofs += (u32)fsdata_start;
	}
}

void FSInit()
{
	u32 header_size = fsheader_end-fsheader_start;
	//Allocate Filesystem Header
	fs_header = malloc(header_size);
	if(!fs_header) {
		while(1);
	}
	//Load Filesystem Header
	RomRead(fs_header, fsheader_start, header_size);
	FixupHeader();
}

bool FSFastOpen(FSFile *file, s32 file_id)
{
	if(file_id < 0) {
		//Set to dummy file if not found
		file->rom_ofs = 0;
		file->len = 0;
		file->pos = 0;
		return false;
	}
	FSEntry *entry_ptr = &fs_header->entries[file_id];
	//Get information about ROM file
	file->rom_ofs = entry_ptr->rom_ofs;
	file->len = entry_ptr->len;
	file->pos = 0; //Reset file cursor
	return true;
}

s32 FSFindFile(char *name)
{
	//Skip initial slash
	if(*name == '/') {
		name++;
	}
	//Find file in filesystem
	for(u32 i=0; i<fs_header->count; i++) {
		if(!strcmp(name, fs_header->entries[i].name)) {
			//Path found
			return i;
		}
	}
	//File not found
	return -1;
}

bool FSOpen(FSFile *file, char *name)
{
	return FSFastOpen(file, FSFindFile(name));
}

u32 FSGetRomPos(FSFile *file)
{
	return file->rom_ofs;
}

u32 FSGetRomLength(FSFile *file)
{
	//Round to nearest 2-byte boundary
	return (file->len+1) & ~0x1;
}

u32 FSGetLength(FSFile *file)
{
	return file->len;
}

s32 FSRead(FSFile *file, void *dst, s32 len)
{
	//Clamp read to end of file
	if(file->pos+len > file->len) {
		len = file->len-len;
	}
	//Read only if there is data to read
	if(len > 0) {
		RomRead(dst, (void *)(file->rom_ofs+file->pos), len);
		file->pos += len;
	}
	return len;
}

bool FSSeek(FSFile *file, s32 offset, u32 origin)
{
	switch(origin) {
		case FS_SEEK_SET:
		//Relative to start
			file->pos = offset;
			break;
			
		case FS_SEEK_CUR:
		//Relative to current position
			file->pos += offset;
			break;
			
		case FS_SEEK_END:
		//Relative to end
			file->pos = file->len+offset;
			break;
			
		default:
		//Can't seek
			return false;
	}
	//Clamp file cursor to file boundaries
	if(file->pos < 0) {
		file->pos = 0;
	}
	if(file->pos > file->len) {
		file->pos = file->len;
	}
	return true;
}

s32 FSTell(FSFile *file)
{
	return file->pos;
}