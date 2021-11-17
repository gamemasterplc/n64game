#pragma once

#include "gfx.h"
#define IMG_FMT_I4 0
#define IMG_FMT_I8 1
#define IMG_FMT_IA4 2
#define IMG_FMT_IA8 3
#define IMG_FMT_IA16 4
#define IMG_FMT_CI4 5
#define IMG_FMT_CI8 6
#define IMG_FMT_RGBA16 7
#define IMG_FMT_RGBA32 8

#define IMAGE_FLIP_NONE 0
#define IMAGE_FLIP_X 1
#define IMAGE_FLIP_Y 2
#define IMAGE_FLIP_XY 3

typedef struct n64_image
{
	void *data;
	u16 *pal_data;
	u32 fmt;
	u16 w;
	u16 h;
} N64Image;

#define ImagePut(image, x, y) ImagePutTintFlip(image, x, y, IMAGE_FLIP_NONE, GFX_COLOR_WHITE)
#define ImagePutTint(image, x, y, tint) ImagePutTintFlip(image, x, y, IMAGE_FLIP_NONE, tint)
#define ImagePutFlip(image, x, y, flip) ImagePutTintFlip(image, x, y, flip, GFX_COLOR_WHITE)

#define ImagePutPartial(image, x, y, src_x, src_y, src_w, src_h) \
	ImagePutPartialTintFlip(image, x, y, src_x, src_y, src_w, src_h, IMAGE_FLIP_NONE, GFX_COLOR_WHITE)
#define ImagePutPartialTint(image, x, y, src_x, src_y, src_w, src_h, tint) \
	ImagePutPartialTintFlip(image, x, y, src_x, src_y, src_w, src_h, IMAGE_FLIP_NONE, tint)
#define ImagePutPartialFlip(image, x, y, src_x, src_y, src_w, src_h, flip) \
	ImagePutPartialTintFlip(image, x, y, src_x, src_y, src_w, src_h, flip, GFX_COLOR_WHITE)


N64Image *ImageLoad(char *path);
N64Image *ImageCreate(u16 w, u16 h, u32 fmt);
void ImageFlushData(N64Image *image);
void ImageDelete(N64Image *image);
void ImagePutTintFlip(N64Image *image, int x, int y, int flip, u32 tint);
void ImagePutPartialTintFlip(N64Image *image, int x, int y, int src_x, int src_y, int src_w, int src_h, int flip, u32 tint);