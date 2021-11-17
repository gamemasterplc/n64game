#include <ultra64.h>
#include "fs.h"
#include "gfx.h"
#include "malloc.h"
#include "image.h"

N64Image *ImageLoad(char *path)
{
	FSFile file;
	if(!FSOpen(&file, path)) {
		//Failed to load file
		return NULL;
	}
	//Read image file
	u32 length = FSGetLength(&file);
	N64Image *image = malloc(length);
	FSRead(&file, image, length);
	//Fixup header pointers
	image->data = (void *)((u32)image+(u32)image->data);
	if(image->pal_data) {
		image->pal_data = (u16 *)((u32)image+(u32)image->pal_data);
	}
	return image;
}

N64Image *ImageCreate(u16 w, u16 h, u32 fmt)
{
	N64Image *image = malloc(sizeof(N64Image));
	image->fmt = fmt;
	image->w = w;
	image->h = h;
	//Allocate image data
	//4bpp images round up for odd numbers of pixels
	//CI4 and CI8 allocate a palette
	switch(fmt) {
		case IMG_FMT_I4:
		case IMG_FMT_IA4:
			image->data = malloc(((w*h)+1)/2);
			image->pal_data = NULL;
			break;
			
		case IMG_FMT_I8:
		case IMG_FMT_IA8:
			image->data = malloc(w*h);
			image->pal_data = NULL;
			break;
			
		case IMG_FMT_CI4:
			image->data = malloc(((w*h)+1)/2);
			image->pal_data = malloc(16*2);
			break;
			
		case IMG_FMT_CI8:
			image->data = malloc(w*h);
			image->pal_data = malloc(256*2);
			break;
			
		case IMG_FMT_IA16:
		case IMG_FMT_RGBA16:
			image->data = malloc(w*h*2);
			image->pal_data = NULL;
			break;
			
		case IMG_FMT_RGBA32:
			image->data = malloc(w*h*4);
			image->pal_data = NULL;
			break;
			
		default:
			image->data = NULL;
			image->pal_data = NULL;
			break;
	}
	return image;
}

void ImageFlushData(N64Image *image)
{
	switch(image->fmt) {
		case IMG_FMT_I4:
		case IMG_FMT_IA4:
			osWritebackDCache(image->data, ((image->w*image->h)+1)/2);
			break;
			
		case IMG_FMT_I8:
		case IMG_FMT_IA8:
			osWritebackDCache(image->data, image->w*image->h);
			break;
			
		case IMG_FMT_CI4:
			osWritebackDCache(image->data, ((image->w*image->h)+1)/2);
			osWritebackDCache(image->pal_data, 16*2);
			break;
			
		case IMG_FMT_CI8:
			osWritebackDCache(image->data, image->w*image->h);
			osWritebackDCache(image->pal_data, 256*2);
			break;
			
		case IMG_FMT_IA16:
		case IMG_FMT_RGBA16:
			osWritebackDCache(image->data, image->w*image->h*2);
			break;
			
		case IMG_FMT_RGBA32:
			osWritebackDCache(image->data, image->w*image->h*4);
			break;
			
		default:
			break;
	}
}

void ImageDelete(N64Image *image)
{
	if(image->data != (char *)image+16) {
		//Free image data if not from file
		free(image->data);
		if(image->pal_data) {
			free(image->pal_data);
		}
	}
	free(image);
}

static int slice_word_count[] = { 512, 512, 512, 512, 512, 256, 256, 512, 512 };
static u8 fmt_tile_bytes[] = { 0, 1, 0, 1, 2, 0, 1, 2, 2 };

static int GetSliceHeight(N64Image *image, int width)
{
	int stride;
	if(fmt_tile_bytes[image->fmt] != 0) {
		//Proper Texture Stride Calculation
		if(image->fmt == IMG_FMT_RGBA32) {
			stride = ((width*fmt_tile_bytes[image->fmt])+7)>>2;
		} else {
			stride = ((width*fmt_tile_bytes[image->fmt])+7)>>3;
		}
	} else {
		//Proper 4-bit Texture Stride Calculation
		stride = ((width>>1)+7)>>3;
	}
	return slice_word_count[image->fmt]/stride;
}

static void LoadPalette(N64Image *image)
{
	if(image->fmt == IMG_FMT_CI4 || image->fmt == IMG_FMT_CI8) {
		gDPSetTextureLUT(gfx_dlistp++, G_TT_RGBA16);
		if(image->fmt == IMG_FMT_CI8) {
			gDPLoadTLUT_pal256(gfx_dlistp++, image->pal_data);
		} else {
			gDPLoadTLUT_pal16(gfx_dlistp++, 0, image->pal_data);
		}
	} else {
		gDPSetTextureLUT(gfx_dlistp++, G_TT_NONE);
	}
}

static void LoadTexture(N64Image *image, u16 uls, u16 ult, u16 lrs, u16 lrt)
{
	switch(image->fmt) {
		case IMG_FMT_I4:
			gDPLoadTextureTile_4b(gfx_dlistp++, image->data, G_IM_FMT_I, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case IMG_FMT_I8:
			gDPLoadTextureTile(gfx_dlistp++, image->data, G_IM_FMT_I, G_IM_SIZ_8b, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case IMG_FMT_IA4:
			gDPLoadTextureTile_4b(gfx_dlistp++, image->data, G_IM_FMT_IA, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case IMG_FMT_IA8:
			gDPLoadTextureTile(gfx_dlistp++, image->data, G_IM_FMT_IA, G_IM_SIZ_8b, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case IMG_FMT_IA16:
			gDPLoadTextureTile(gfx_dlistp++, image->data, G_IM_FMT_IA, G_IM_SIZ_16b, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case IMG_FMT_CI4:
			gDPLoadTextureTile_4b(gfx_dlistp++, image->data, G_IM_FMT_CI, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case IMG_FMT_CI8:
			gDPLoadTextureTile(gfx_dlistp++, image->data, G_IM_FMT_CI, G_IM_SIZ_8b, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case IMG_FMT_RGBA16:
			gDPLoadTextureTile(gfx_dlistp++, image->data, G_IM_FMT_RGBA, G_IM_SIZ_16b, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
			
		case IMG_FMT_RGBA32:
			gDPLoadTextureTile(gfx_dlistp++, image->data, G_IM_FMT_RGBA, G_IM_SIZ_32b, image->w, image->h,
				uls, ult, lrs, lrt, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
			break;
	}
}

static void PutImageNoFlip(N64Image *image, int x, int y, int src_x, int src_y, int src_w, int src_h)
{
	int slice_h = GetSliceHeight(image, src_w);
	if(src_h <= slice_h) {
		//Fast one-slice path
		LoadTexture(image, src_x, src_y, (src_x+src_w)-1, (src_y+src_h)-1);
		gSPScisTextureRectangle(gfx_dlistp++, x*4, y*4,
			(x+src_w)*4, (y+src_h)*4, 0, src_x*32, src_y*32, 1024, 1024);
		gDPPipeSync(gfx_dlistp++);
	} else {
		//Multi-slice path
		int num_slices = src_h/slice_h;
		int remainder_h = src_h%slice_h;
		int slice_y = 0;
		//Render slices
		for(int i=0; i<num_slices; i++) {
			if(y+slice_y < -slice_h) {
				//Skip offscreen slices
				slice_y += slice_h;
				continue;
			}
			LoadTexture(image, src_x, src_y+slice_y, (src_x+src_w)-1, (src_y+slice_y+slice_h)-1);
			gSPScisTextureRectangle(gfx_dlistp++, x*4, (y+slice_y)*4,
				(x+src_w)*4, (y+slice_y+slice_h)*4, 0, 
				src_x*32, (src_y+slice_y)*32, 1024, 1024);
			gDPPipeSync(gfx_dlistp++);
			slice_y += slice_h;
		}
		if(remainder_h == 0) {
			return;
		}
		//Render remainder
		LoadTexture(image, src_x, src_y+slice_y, (src_x+src_w)-1, (src_y+slice_y+remainder_h)-1);
		gSPScisTextureRectangle(gfx_dlistp++, x*4, (y+slice_y)*4,
			(x+src_w)*4, (y+slice_y+remainder_h)*4, 0,
			src_x*32, (src_y+slice_y)*32, 1024, 1024);
		gDPPipeSync(gfx_dlistp++);
	}
}

static void PutImageFlipX(N64Image *image, int x, int y, int src_x, int src_y, int src_w, int src_h)
{
	int slice_h = GetSliceHeight(image, src_w);
	if(src_h <= slice_h) {
		//Fast one-slice path
		LoadTexture(image, src_x, src_y, (src_x+src_w)-1, (src_y+src_h)-1);
		gSPScisTextureRectangle(gfx_dlistp++, x*4, y*4, (x+src_w)*4, (y+src_h)*4,
			0, (src_x+src_w-1)*32, src_y*32, -1024, 1024);
		gDPPipeSync(gfx_dlistp++);
	} else {
		//Multi-slice path
		int num_slices = src_h/slice_h;
		int remainder_h = src_h%slice_h;
		int slice_y = 0;
		//Render slices
		for(int i=0; i<num_slices; i++) {
			if(y+slice_y < -slice_h) {
				//Skip offscreen slices
				slice_y += slice_h;
				continue;
			}
			LoadTexture(image, src_x, src_y+slice_y, (src_x+src_w)-1, (src_y+slice_y+slice_h)-1);
			gSPScisTextureRectangle(gfx_dlistp++, x*4, (y+slice_y)*4,
				(x+src_w)*4, (y+slice_y+slice_h)*4, 0,
				(src_x+src_w-1)*32, (src_y+slice_y)*32, -1024, 1024);
			gDPPipeSync(gfx_dlistp++);
			slice_y += slice_h;
		}
		if(remainder_h == 0) {
			return;
		}
		//Render remainder
		LoadTexture(image, src_x, src_y+slice_y, (src_x+src_w)-1, (src_y+slice_y+remainder_h)-1);
		gSPScisTextureRectangle(gfx_dlistp++, x*4, (y+slice_y)*4,
			(x+src_w)*4, (y+slice_y+remainder_h)*4, 0,
			(src_x+src_w-1)*32, (src_y+slice_y)*32, -1024, 1024);
		gDPPipeSync(gfx_dlistp++);
	}
}

static void PutImageFlipY(N64Image *image, int x, int y, int src_x, int src_y, int src_w, int src_h)
{
	int slice_h = GetSliceHeight(image, src_w);
	if(src_h <= slice_h) {
		//Fast one-slice path
		LoadTexture(image, src_x, src_y, (src_x+src_w)-1, (src_y+src_h)-1);
		gSPScisTextureRectangle(gfx_dlistp++, x*4, y*4,
			(x+src_w)*4, (y+src_h)*4, 0, src_x*32, (src_y+src_h-1)*32, 1024, -1024);
		gDPPipeSync(gfx_dlistp++);
	} else {
		//Multi-slice path
		int num_slices = src_h/slice_h;
		int remainder_h = src_h%slice_h;
		int slice_y = 0;
		//Render slices
		for(int i=0; i<num_slices; i++) {
			if(y+slice_y < -slice_h) {
				//Skip offscreen slices
				slice_y += slice_h;
				continue;
			}
			LoadTexture(image, src_x, src_y+slice_y, (src_x+src_w)-1, (src_y+slice_y+slice_h)-1);
			gSPScisTextureRectangle(gfx_dlistp++, x*4, (y+(src_h-(slice_y+slice_h)))*4,
				(x+src_w)*4, (y+(src_h-slice_y))*4, 0, 
				src_x*32, (src_y+slice_y+slice_h-1)*32, 1024, -1024);
			gDPPipeSync(gfx_dlistp++);
			slice_y += slice_h;
		}
		if(remainder_h == 0) {
			return;
		}
		//Render remainder
		LoadTexture(image, src_x, src_y+slice_y, (src_x+src_w)-1, (src_y+slice_y+remainder_h)-1);
		gSPScisTextureRectangle(gfx_dlistp++, x*4, y*4,
			(x+src_w)*4, (y+remainder_h)*4, 0,
			src_x*32, (src_y+slice_y+remainder_h-1)*32, 1024, -1024);
		gDPPipeSync(gfx_dlistp++);
	}
}

static void PutImageFlipXY(N64Image *image, int x, int y, int src_x, int src_y, int src_w, int src_h)
{
	int slice_h = GetSliceHeight(image, src_w);
	if(src_h <= slice_h) {
		//Fast one-slice path
		LoadTexture(image, src_x, src_y, (src_x+src_w)-1, (src_y+src_h)-1);
		gSPScisTextureRectangle(gfx_dlistp++, x*4, y*4,
			(x+src_w)*4, (y+src_h)*4, 0, (src_x+src_w-1)*32, (src_y+src_h-1)*32, -1024, -1024);
		gDPPipeSync(gfx_dlistp++);
	} else {
		//Multi-slice path
		int num_slices = src_h/slice_h;
		int remainder_h = src_h%slice_h;
		int slice_y = 0;
		//Render slices
		for(int i=0; i<num_slices; i++) {
			if(y+slice_y < -slice_h) {
				//Skip offscreen slices
				slice_y += slice_h;
				continue;
			}
			LoadTexture(image, src_x, src_y+slice_y, (src_x+src_w)-1, (src_y+slice_y+slice_h)-1);
			gSPScisTextureRectangle(gfx_dlistp++, x*4, (y+(src_h-(slice_y+slice_h)))*4,
				(x+src_w)*4, (y+(src_h-slice_y))*4, 0, 
				(src_x+src_w-1)*32, (src_y+slice_y+slice_h-1)*32, -1024, -1024);
			gDPPipeSync(gfx_dlistp++);
			slice_y += slice_h;
		}
		if(remainder_h == 0) {
			return;
		}
		//Render remainder
		LoadTexture(image, src_x, src_y+slice_y, (src_x+src_w)-1, (src_y+slice_y+remainder_h)-1);
		gSPScisTextureRectangle(gfx_dlistp++, x*4, y*4,
			(x+src_w)*4, (y+remainder_h)*4, 0,
			(src_x+src_w-1)*32, (src_y+slice_y+remainder_h-1)*32, -1024, -1024);
		gDPPipeSync(gfx_dlistp++);
	}
}

void ImagePutTintFlip(N64Image *image, int x, int y, int flip, u32 tint)
{
	//Check for hidden image
	if(x < -image->w || y < -image->h) {
		return;
	}
	if(tint == GFX_COLOR_WHITE) {
		//White equals no tint
		GfxSetRenderMode(GFX_RENDER_IMAGE);
	} else {
		//Get tint channels
		u8 tint_r = (tint >> 24) & 0xFF;
		u8 tint_g = (tint >> 16) & 0xFF;
		u8 tint_b = (tint >> 8) & 0xFF;
		u8 tint_a = tint & 0xFF;
		GfxSetRenderMode(GFX_RENDER_IMAGETINT);
		gDPSetPrimColor(gfx_dlistp++, 0, 0, tint_r, tint_g, tint_b, tint_a);
	}
	LoadPalette(image);
	switch(flip) {
		case IMAGE_FLIP_NONE:
			PutImageNoFlip(image, x, y, 0, 0, image->w, image->h);
			break;
			
		case IMAGE_FLIP_X:
			PutImageFlipX(image, x, y, 0, 0, image->w, image->h);
			break;
			
		case IMAGE_FLIP_Y:
			PutImageFlipY(image, x, y, 0, 0, image->w, image->h);
			break;
			
		case IMAGE_FLIP_XY:
			PutImageFlipXY(image, x, y, 0, 0, image->w, image->h);
			break;
			
		default:
			PutImageNoFlip(image, x, y, 0, 0, image->w, image->h);
			break;
	}
}

void ImagePutPartialTintFlip(N64Image *image, int x, int y, int src_x, int src_y, int src_w, int src_h, int flip, u32 tint)
{
	//Check for hidden image
	if(x < -src_w || y < -src_h) {
		return;
	}
	if(tint == GFX_COLOR_WHITE) {
		//White equals no tint
		GfxSetRenderMode(GFX_RENDER_IMAGE);
	} else {
		//Get tint channels
		u8 tint_r = (tint >> 24) & 0xFF;
		u8 tint_g = (tint >> 16) & 0xFF;
		u8 tint_b = (tint >> 8) & 0xFF;
		u8 tint_a = tint & 0xFF;
		GfxSetRenderMode(GFX_RENDER_IMAGETINT);
		gDPSetPrimColor(gfx_dlistp++, 0, 0, tint_r, tint_g, tint_b, tint_a);
	}
	LoadPalette(image);
	switch(flip) {
		case IMAGE_FLIP_NONE:
			PutImageNoFlip(image, x, y, src_x, src_y, src_w, src_h);
			break;
			
		case IMAGE_FLIP_X:
			PutImageFlipX(image, x, y, src_x, src_y, src_w, src_h);
			break;
			
		case IMAGE_FLIP_Y:
			PutImageFlipY(image, x, y, src_x, src_y, src_w, src_h);
			break;
			
		case IMAGE_FLIP_XY:
			PutImageFlipXY(image, x, y, src_x, src_y, src_w, src_h);
			break;
			
		default:
			PutImageNoFlip(image, x, y, src_x, src_y, src_w, src_h);
			break;
	}
}