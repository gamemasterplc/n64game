#pragma once

#include <ultra64.h>
#include "bool.h"

typedef struct fs_file {
	u32 rom_ofs;
	u32 len;
	s32 pos;
} FSFile;

#define FS_SEEK_SET 0
#define FS_SEEK_CUR 1
#define FS_SEEK_END 2

void FSInit();
bool FSVerifyFileID(s32 file_id);
bool FSFastOpen(FSFile *file, s32 file_id);
s32 FSFindFile(char *name);
bool FSOpen(FSFile *file, char *name);
u32 FSGetRomPos(FSFile *file);
u32 FSGetRomLength(FSFile *file);
u32 FSGetLength(FSFile *file);
s32 FSRead(FSFile *file, void *dst, s32 len);
bool FSSeek(FSFile *file, s32 offset, u32 origin);
s32 FSTell(FSFile *file);