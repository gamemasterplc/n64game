#pragma once

#define GLIST_SIZE 8192
#define NUM_CFBS 3

#include <ultra64.h>

//Render mode defines
typedef enum gfx_render_mode {
	GFX_RENDER_RECT,
	GFX_RENDER_RECT_ALPHA,
	GFX_RENDER_IMAGE,
	GFX_RENDER_IMAGETINT
} GfxRenderMode;

//Forces alpha to 255
#define GFX_MAKE_COLOR(r, g, b) ((((u8)(r)) << 24)|(((u8)(g)) << 16)|(((u8)(b)) << 8)|255)
//Used for when alpha can be not 255
#define GFX_MAKE_COLOR_ALPHA(r, g, b, a) ((((u8)(r)) << 24)|(((u8)(g)) << 16)|(((u8)(b)) << 8)|((u8)(a)))

//Color defines
#define GFX_COLOR_BLACK GFX_MAKE_COLOR(0, 0, 0)
#define GFX_COLOR_WHITE GFX_MAKE_COLOR(255, 255, 255)
#define GFX_COLOR_RED GFX_MAKE_COLOR(255, 0, 0)
#define GFX_COLOR_GREEN GFX_MAKE_COLOR(0, 255, 0)
#define GFX_COLOR_BLUE GFX_MAKE_COLOR(0, 0, 255)
#define GFX_COLOR_YELLOW GFX_MAKE_COLOR(255, 255, 0)
#define GFX_COLOR_MAGENTA GFX_MAKE_COLOR(255, 0, 255)
#define GFX_COLOR_CYAN GFX_MAKE_COLOR(0, 255, 255)

//Externs
extern Gfx *gfx_dlistp;

//Function definitions
void GfxInit();
void GfxSetScreenSize(int width, int height);
int GfxGetWidth();
int GfxGetHeight();
void GfxSetClearColor(u8 r, u8 g, u8 b);
void GfxSetScissor(int x, int y, int w, int h);
void GfxResetScissor();
void GfxStartFrame();
void GfxSetRenderMode(GfxRenderMode mode);
void GfxPutRect(int x, int y, int w, int h, u32 color);
void GfxEndFrame();