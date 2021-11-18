#include <ultra64.h>
#include "state.h"
#include "gfx.h"
#include "image.h"
#include "bool.h"
#include "libcext.h"

static N64Image *ship_image;
static float ship_angle;
static float ship_cos, ship_sin;

#define SHIP_IMAGE_W 32
#define SHIP_IMAGE_H 32
#define SHIP_HALF_W 8
#define SHIP_HALF_H 12

static void StateInit()
{
	ship_image = ImageCreate(SHIP_IMAGE_W, SHIP_IMAGE_H, IMG_FMT_I8);
	ship_angle = 0;
}

static void ClearShipImage()
{
	u8 *data = ship_image->data;
	for(int i=0; i<SHIP_IMAGE_W*SHIP_IMAGE_H; i++) {
		data[i] = 0; //Write black
	}
}

static void DrawShipLine(int x0, int y0, int x1, int y1)
{
	//Implementation of Bresenham line drawing algorithm
	u8 *data = ship_image->data;
	int dx =  abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1; 
	int err = dx + dy;
	int e2;
	while(1) {
		data[(y0*SHIP_IMAGE_W)+x0] = 0xFF;
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = 2 * err;
		if (e2 >= dy) {
			err += dy; x0 += sx;
		}
		if (e2 <= dx) {
			err += dx; y0 += sy;
		}
	}
}

static void UpdateShipImage()
{
	float c, s;
	float x1, y1, x2, y2, x3, y3;
	ClearShipImage();
	c = cosf(ship_angle*M_DTOR);
	s = sinf(ship_angle*M_DTOR);
	//Rotate (0, -12) to ship image
	x1 = (0*c)-(-SHIP_HALF_H*s);
	y1 = (0*s)+(-SHIP_HALF_H*c);
	x1 += SHIP_IMAGE_W/2;
	y1 += SHIP_IMAGE_H/2;
	//Rotate (-8, 12) to ship image
	x2 = (-SHIP_HALF_W*c)-(SHIP_HALF_H*s);
	y2 = (-SHIP_HALF_W*s)+(SHIP_HALF_H*c);
	x2 += SHIP_IMAGE_W/2;
	y2 += SHIP_IMAGE_H/2;
	//Rotate (8, 12) to ship image
	x3 = (SHIP_HALF_W*c)-(SHIP_HALF_H*s);
	y3 = (SHIP_HALF_W*s)+(SHIP_HALF_H*c);
	x3 += SHIP_IMAGE_W/2;
	y3 += SHIP_IMAGE_H/2;
	DrawShipLine(x1, y1, x2, y2);
	DrawShipLine(x1, y1, x3, y3);
	DrawShipLine(x2, y2, x3, y3);
	ImageFlushData(ship_image);
}

static void StateMain()
{
	UpdateShipImage();
	ship_angle += 1;
	if(ship_angle >= 360) {
		ship_angle -= 360;
	}
}

static void StateDraw()
{
	int screen_w = GfxGetWidth();
	int screen_h = GfxGetHeight();
	ImagePut(ship_image, (screen_w-SHIP_IMAGE_W)/2, (screen_h-SHIP_IMAGE_H)/2);
}

static void StateDestroy()
{
	ImageDelete(ship_image);
}

StateEntry Game_StateData = {
	StateInit, StateMain, StateDraw, StateDestroy
};