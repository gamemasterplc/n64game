#pragma once

#include <ultra64.h>

#define ROMREAD_BUF_SIZE 1024

void RomRead(void *dst, void *src, u32 len);